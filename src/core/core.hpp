#ifndef SHAPEZX_CORE_HPP
#define SHAPEZX_CORE_HPP

#include "../vec/vec.hpp"
#include "machine.hpp"
#include "ore.hpp"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <random>
#include <span>
#include <type_traits>
#include <vector>

namespace shapezx {

using std::nullopt;
using std::size_t;
using std::span;
using std::unique_ptr;
using std::vector;

using ssize_t = std::make_signed_t<size_t>;

struct Context {
  std::int32_t efficiency_factor = 1;
};

struct Chunk {
  optional<Item> ore;
  optional<unique_ptr<Building>> building;

  Chunk() = default;
  Chunk(const Chunk &chunk)
      : ore(chunk.ore), building(chunk.building.transform(
                            [](const auto &b) { return b->clone(); })) {}

  void update(MapAccessor);
};

struct Map {
  static constexpr double HAS_ORE_PROBALITY = 0.3;
  static constexpr array<double, 2> DISTRIBUTION{0.9, 0.1};

  vector<Chunk> chunks;
  size_t height;
  size_t width;

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

  void add_machine(vec::Vec2<std::size_t> pos, unique_ptr<Building> &&machine) {
    (*this)[pos].building = std::make_optional(std::move(machine));
  }

  void update(Context &ctx);
};

struct MapAccessor {
  vec::Vec2<size_t> pos;
  std::reference_wrapper<Map> map;
  std::reference_wrapper<Context> ctx;

  MapAccessor(vec::Vec2<size_t> p, Map &m, Context &ctx_)
      : pos(p), map(m), ctx(ctx_) {}

  auto &current_chunk(this auto &&self) {
    return self.map.get()[self.pos[0], self.pos[1]];
  }

  auto &get_chunk(this auto &&self, vec::Vec2<ssize_t> r) {
    return self.map.get()[self.relative_pos_by(r)];
  }

  // Returns r + current position .
  vec::Vec2<size_t> relative_pos_by(vec::Vec2<ssize_t> r) {
    return this->pos + r;
  }

  void add_machine(std::unique_ptr<shapezx::Building> &&machine) {
    this->map.get().add_machine(this->pos, std::move(machine));
  }
};

struct State {
  Map map;
  Context ctx;

  State(size_t height, size_t width, size_t seed) : map(height, width, seed) {}

  State(size_t height, size_t width)
      : State(height, width, std::random_device()()) {}

  State(const Map &&map_, const Context &&ctx_)
      : map(std::move(map_)), ctx(std::move(ctx_)) {}

  MapAccessor create_accessor_at(shapezx::vec::Vec2<std::size_t> pos) {
    return {pos, this->map, this->ctx};
  }

  void update() {
    // std::cout << "updating\n";
    this->map.update(this->ctx);
  }
};
} // namespace shapezx

#endif
