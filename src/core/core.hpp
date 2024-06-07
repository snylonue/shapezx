#ifndef SHAPEZX_CORE_HPP
#define SHAPEZX_CORE_HPP

#include "../vec/vec.hpp"
#include "equipment.hpp"
#include "ore.hpp"


#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
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

  Chunk &operator[](size_t x, size_t y) { return chunks[x * this->width + y]; }

  void update(Context &ctx);
};

struct MapAccessor {
  vec::Vec2<size_t> pos;
  std::reference_wrapper<Map> map;
  std::reference_wrapper<Context> ctx;

  MapAccessor(vec::Vec2<size_t> p, Map &m, Context &ctx_) : pos(p), map(m), ctx(ctx_) {}

  Chunk &current_chunk() { return map[pos[0], pos[1]]; }

  // Returns r + current position .
  vec::Vec2<size_t> relative_pos_by(vec::Vec2<ssize_t> r) {
    return this->pos + r;
  }
};

struct State {
  Map map;
};
} // namespace shapezx

#endif
