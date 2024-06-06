#include "core.hpp"
#include "equipment.hpp"

namespace shapezx {

void Chunk::update(MapAccessor m) {
  if (this->building) {
    auto &b = *this->building;
    b->update(m);
  }
}

void Map::update() {
  for (auto [r, row] : this->chunks | std::ranges::views::chunk(this->width) |
                           std::ranges::views::enumerate) {
    for (auto [c, chunk] : row | std::ranges::views::enumerate) {
      auto acc = MapAccessor(vec::Vec2<std::size_t>(r, c), *this);
      chunk.update(acc);
    }
  }
}

} // namespace shapezx
