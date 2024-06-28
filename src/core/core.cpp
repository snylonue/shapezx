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
} // namespace shapezx
