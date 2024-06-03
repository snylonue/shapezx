#include "equipment.hpp"
#include "core.hpp"

#include <array>
#include <ranges>

namespace shapezx {

constexpr Direction left_of(Direction d) {
  switch (d) {
  case Direction::Left:
    return Direction::Down;
  case Direction::Down:
    return Direction::Right;
  case Direction::Right:
    return Direction::Up;
  case Direction::Up:
    return Direction::Left;
  }
}

constexpr Direction opposite_of(Direction d) {
  switch (d) {
  case Direction::Left:
    return Direction::Right;
  case Direction::Down:
    return Direction::Up;
  case Direction::Right:
    return Direction::Left;
  case Direction::Up:
    return Direction::Down;
  }
}

constexpr Direction right_of(Direction d) { return opposite_of(left_of(d)); }

constexpr std::array<Direction, 4> ALL_DIRECTIONS = {
    Direction::Down, Direction::Up, Direction::Left, Direction::Right};

constexpr vec::Vec2<ssize_t> to_vec2(Direction d) {
  switch (d) {
  case Direction::Left:
    return {-1, 0};
  case Direction::Down:
    return {0, -1};
  case Direction::Right:
    return {1, 0};
  case Direction::Up:
    return {0, 1};
  }
}

void Building::update(MapAccessor) {}

void Miner::update(MapAccessor m) {
  const auto &chk = m.current_chunk();
  if (chk.ore) {
    const auto &ore = chk.ore.value();
    if (this->ores.num == 0) {
      this->ores.item = ore;
    }
    this->ores.num += this->info_.efficiency;
  }
}

Capability Miner::output_capabilities(MapAccessor &m) const {
  return {
    .positions = std::views::transform(ALL_DIRECTIONS, to_vec2) |
         std::views::transform(
             [&](const auto d) { return m.relative_pos_by(d); }) |
         std::ranges::to<vector>()
  };
}
} // namespace shapezx