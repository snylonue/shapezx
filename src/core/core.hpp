#include "equipment.hpp"
#include "ore.hpp"
#include "../vec/vec.hpp"

#include <cstddef>
#include <memory>
#include <optional>
#include <ranges>
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

struct Chunk {
  optional<Ore> ore;
  optional<unique_ptr<Building>> building;

  Chunk() = default;
  Chunk(const Chunk &chunk)
      : ore(chunk.ore), building(chunk.building.transform(
                            [](const auto &b) { return b->clone(); })) {}

      void update(MapAccessor);
};

struct Map {
  vector<Chunk> chunks;
  size_t height;
  size_t width;

  // caller must guarantee that chks.size() == h * w
  Map(const vector<Chunk> &chks, size_t h, size_t w)
      : chunks(chks), height(h), width(w) {}
  Map(size_t h, size_t w) : chunks(h * w), height(h), width(w) {}

  Chunk& operator[](size_t x, size_t y) {
    return chunks[x * this->width + y];
  }

  void update();
};

struct MapAccessor {
  vec::Vec2<size_t> pos;
  Map& map;

  MapAccessor(vec::Vec2<size_t> p, Map& m) : pos(p), map(m) {}

  Chunk& current_chunk() {
    return map[pos[0], pos[1]];
  }

  // Returns r + current position .
  vec::Vec2<size_t> relative_pos_by(vec::Vec2<ssize_t> r) {
    return this->pos + r;
  }
};

struct State {
  Map map;
};
} // namespace shapezx
