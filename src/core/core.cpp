#include "core.hpp"
#include "machine.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <ranges>

namespace shapezx {

void Chunk::update(MapAccessor m) {
  if (this->building) {
    auto &b = *this->building;
    b->update(m);
  }
}

void Map::update(State &ctx) {
  for (auto const [r, row] : this->chunks |
                                 std::ranges::views::chunk(this->width) |
                                 std::ranges::views::enumerate) {
    for (auto const [c, chunk] : row | std::ranges::views::enumerate) {
      auto acc = MapAccessor(vec::Vec2<>(r, c), *this, ctx);
      chunk.update(acc);
    }
  }
}

void to_json(json &j, const Chunk &p) {
  if (p.ore) {
    j["ore"] = *p.ore;
  } else {
    j["ore"] = nullptr;
  }
  if (p.building) {
    json b;
    auto &building = *p.building;
    building->to_json(b);
    std::cout << b << '\n';
    j["building"] = b;
  } else {
    j["building"] = nullptr;
  }

  std::cout << j << '\n';
}

void from_json(const json &j, Chunk &p) {
  std::cout << "t1\n";
  try {
    Item it;
    j.at("ore").get_to(it);
    p.ore = it;
  } catch (const json::exception &) {
    p.ore = nullopt;
  }
  std::cout << "t2\n";
  try {
    std::cout << j << '\n';
    BuildingInfo info;
    j.at("/building/info"_json_pointer).get_to(info);
    std::cout << "info\n";
    auto serialize_building = [&](std::unique_ptr<Building> &&b) {
      b->from_json(j["building"]);
      std::cout << "building set\n";
      p.building = std::move(b);
      std::cout << "building set\n";
    };
    switch (info.type) {
    case BuildingType::Miner:
      serialize_building(std::make_unique<Miner>());
      break;
    case BuildingType::Belt:
      serialize_building(std::make_unique<Belt>());
      break;
    case BuildingType::Cutter:
      serialize_building(std::make_unique<Cutter>());
      break;
    case BuildingType::TrashCan:
      serialize_building(std::make_unique<TrashCan>());
      break;
    case BuildingType::TaskCenter:
      serialize_building(std::make_unique<TaskCenter>());
      break;
    case BuildingType::PlaceHolder:
      serialize_building(std::make_unique<PlaceHolder>());
      break;
    }
  } catch (const json::exception &e) {
    // std::cout << e.what() << '\n';
    p.building = nullopt;
  }
  std::cout << "t3\n";
}

Global Global::load(const std::string &p) noexcept try {
  if (!std::filesystem::exists(p)) {
    return {};
  }
  std::ifstream f(p);
  auto j = json::parse(f);
  f.close();
  return j.get<Global>();
} catch (...) {
  return {};
}

template <typename T>
void save_json(const T &obj, const std::string &p) {
  std::ofstream f(p);
  auto j = json(obj);
  f << j;
  f.close();
}

void Global::save_to(const std::string &p) const {
  save_json(*this, p);
}

void State::save_to(const std::string &p) const {
  save_json(*this, p);
}
} // namespace shapezx
