#include "core.hpp"
#include "machine.hpp"

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
  }
}

void from_json(const json &j, Chunk &p) {
  try {
    Item it;
    j.at("ore").get_to(it);
    p.ore = it;
  } catch (const nlohmann::detail::out_of_range &) {
    p.ore = nullopt;
  }
  try {
    BuildingInfo info;
    j.at("info").get_to(info);
    auto serialize_building = [&](std::unique_ptr<Building> &&b) {
      b->from_json(j);
      p.building = std::move(b);
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
      break;
    }
  } catch (const nlohmann::detail::out_of_range &) {
    p.building = nullopt;
  }
}
} // namespace shapezx
