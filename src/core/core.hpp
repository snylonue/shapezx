#ifndef SHAPEZX_CORE_HPP
#define SHAPEZX_CORE_HPP

#include "../vec/vec.hpp"
#include "machine.hpp"
#include "ore.hpp"
#include "task.hpp"

#include <nlohmann/detail/exceptions.hpp>
#include <nlohmann/detail/macro_scope.hpp>
#include <nlohmann/json.hpp>

#include <cstddef>
#include <cstdint>
#include <exception>
#include <format>
#include <functional>
#include <iostream>
#include <memory>
#include <nlohmann/json_fwd.hpp>
#include <optional>
#include <random>
#include <ranges>
#include <span>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace shapezx {

using nlohmann::json;

using std::nullopt;
using std::size_t;
using std::span;
using std::unique_ptr;
using std::vector;

using ssize_t = std::make_signed_t<size_t>;

struct Chunk {
  optional<Item> ore;
  optional<unique_ptr<Building>> building;

  Chunk() = default;
  Chunk(const Chunk &chunk)
      : ore(chunk.ore), building(chunk.building.transform(
                            [](const auto &b) { return b->clone(); })) {}

  Chunk &operator=(Chunk &&other) = default;

  void update(MapAccessor);
};

void to_json(json &j, const Chunk &p);

void from_json(const json &j, Chunk &p);

struct Efficiency {
  std::int32_t miner = 1;
  std::int32_t belt = 1;
  std::int32_t cutter = 1;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Efficiency, miner, belt, cutter);

struct State;

struct Map {
  static constexpr double HAS_ORE_PROBALITY = 0.3;
  static constexpr array<double, 2> DISTRIBUTION{0.9, 0.1};

  vector<Chunk> chunks;
  size_t height;
  size_t width;

  Map() = default;
  Map(size_t h, size_t w, size_t seed) : chunks(h * w), height(h), width(w) {
    std::mt19937_64 gen{seed};
    std::bernoulli_distribution has_ore(HAS_ORE_PROBALITY);
    std::discrete_distribution<size_t> ore_distribution(DISTRIBUTION.begin(),
                                                        DISTRIBUTION.end());

    for (auto &chk : this->chunks) {
      if (has_ore(gen)) {
        chk.ore = ORES[ore_distribution(gen)];
      }
    }
  }

  // caller must guarantee that chks.size() == h * w
  Map(const vector<Chunk> &chks, size_t h, size_t w)
      : chunks(chks), height(h), width(w) {}
  Map(size_t h, size_t w) : chunks(h * w), height(h), width(w) {}

  auto &operator[](this auto &&self, size_t x, size_t y) {
    return self.chunks.at(x * self.width + y);
  }

  auto &operator[](this auto &&self, vec::Vec2<std::size_t> pos) {
    return self[pos[0], pos[1]];
  }

  void update(State &ctx);
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Map, chunks, height, width);

struct MapAccessor {
  vec::Vec2<size_t> pos;
  std::reference_wrapper<Map> map;
  std::reference_wrapper<State> ctx;

  MapAccessor(vec::Vec2<size_t> p, Map &m, State &ctx_)
      : pos(p), map(m), ctx(ctx_) {}

  auto &current_chunk(this auto &&self) {
    return self.map.get()[self.pos[0], self.pos[1]];
  }

  auto &get_chunk(this auto &&self, vec::Vec2<ssize_t> r) {
    return self.map.get()[self.relative_pos_by(r)];
  }

  MapAccessor relocate(vec::Vec2<> p) const {
    return {p, this->map, this->ctx};
  }

  std::pair<Chunk &, MapAccessor> get_chunk_and_accessor(this auto &&self,
                                                         vec::Vec2<ssize_t> r) {
    return {self.map.get()[self.relative_pos_by(r)],
            MapAccessor(self.pos + r, self.map, self.ctx)};
  }

  // Returns r + current position .
  vec::Vec2<size_t> relative_pos_by(vec::Vec2<ssize_t> r) const {
    return this->pos + r;
  }

  // returns modified chunks
  vector<vec::Vec2<>>
  add_machine(std::unique_ptr<shapezx::Building> &&machine) {
    auto &m = this->map.get();
    auto rect = machine->relative_rect();
    vector<vec::Vec2<>> res{this->pos};
    if (!m[this->pos].building) {
      for (auto [r, c] : rect_iter(rect) | std::views::drop(1)) {
        auto [chk, acc] = this->get_chunk_and_accessor({r, c});
        res.push_back(acc.pos);
        if (chk.building) {
          throw std::exception();
        }
        chk.building = std::make_unique<PlaceHolder>(
            machine->info().id, machine->info().direction, this->pos);
      }

      m[pos].building = std::make_optional(std::move(machine));
    }

    return res;
  }

  vector<vec::Vec2<>> remove_machine() {
    auto &machine = this->current_chunk().building.value();
    auto rect = machine->relative_rect();
    vector<vec::Vec2<>> res;
    for (auto [r, c] : rect_iter(rect)) {
      auto [chk, acc] = this->get_chunk_and_accessor({r, c});
      res.push_back(acc.pos);
      chk.building.reset();
    }

    return res;
  }
};

struct Global {
  static Global load(const std::string &p) noexcept;

  std::uint32_t value = 0;

  optional<size_t> last_played;

  size_t max_height = 20;
  size_t max_width = 30;

  std::vector<std::string> saves;

  std::string &create_save() {
    auto n = this->saves.size();
    return saves.emplace_back(std::format("./saves/{}.json", n));
  }

  void save_to(const std::string &p) const;
};

inline void to_json(nlohmann::json &nlohmann_json_j,
                    const Global &nlohmann_json_t) {
  nlohmann_json_j["value"] = nlohmann_json_t.value;
  if (nlohmann_json_t.last_played) {
    nlohmann_json_j["last_played"] = *nlohmann_json_t.last_played;
  } else {
    nlohmann_json_j["last_played"] = nullptr;
  }
  nlohmann_json_j["max_height"] = nlohmann_json_t.max_height;
  nlohmann_json_j["max_width"] = nlohmann_json_t.max_width;
  nlohmann_json_j["saves"] = nlohmann_json_t.saves;
}
inline void from_json(const nlohmann ::json &nlohmann_json_j,
                      Global &nlohmann_json_t) {
  nlohmann_json_j.at("value").get_to(nlohmann_json_t.value);
  try {
    size_t last;
    nlohmann_json_j.at("last_played").get_to(last);
    nlohmann_json_t.last_played = last;
  } catch (...) {
    nlohmann_json_t.last_played = nullopt;
  }
  nlohmann_json_j.at("max_height").get_to(nlohmann_json_t.max_height);
  nlohmann_json_j.at("max_width").get_to(nlohmann_json_t.max_width);
  nlohmann_json_j.at("saves").get_to(nlohmann_json_t.saves);
};

struct IdGenerator {
  std::uint32_t cur = 0;

  std::uint32_t gen() {
    this->cur += 1;
    return this->cur;
  }
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(IdGenerator, cur);

struct State {
  Map map;
  Efficiency eff;
  Buffer store;
  std::uint32_t value = 0;
  vector<Task> tasks;
  IdGenerator id_;

  State() = default;
  State(size_t height, size_t width, size_t seed) : map(height, width, seed) {}

  State(size_t height, size_t width)
      : State(height, width, std::random_device()()) {}

  State(Map &&map_) : map(std::move(map_)) {}

  MapAccessor create_accessor_at(shapezx::vec::Vec2<std::size_t> pos) {
    return {pos, this->map, *this};
  }

  void update(std::function<void()> on_task_complete, Global &global_state) {
    // std::cout << "updating\n";
    this->map.update(*this);
    global_state.value += this->value;
    this->value = 0;

    for (auto &task : this->tasks) {
      if (task.update(*this)) {
        on_task_complete();
      }
    }
  }

  void take_item(const Item &item, size_t num) {
    std::cout << std::format("take {} {}\n", num, item.name);
    this->store.increase(item, num);
    this->value += item.value * num;
  }

  void add_task(Task &&task) { this->tasks.push_back(std::move(task)); }

  void save_to(const std::string &) const;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(State, map, eff, store, value, tasks, id_);

} // namespace shapezx

#endif
