#include "machine.hpp"
#include "core.hpp"

#include <array>

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

[[maybe_unused]] constexpr std::array<Direction, 4> ALL_DIRECTIONS = {
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
    this->ores[ore] += m.ctx.get().efficiency_factor * 1;
  }
}

void Belt::input(MapAccessor &m, Buffer &buf, Capability cap) {
  cap = cap.merge(this->transport_capability(m.ctx.get().efficiency_factor));

  for (auto &[item, val] : buf.items) {
    auto accepts = cap.num_accepts(item).value_or(val);

    if (accepts) {
      val -= accepts;
      this->buffer.increase(item, accepts);
    }
  }
}

void Belt::update(MapAccessor m) {
  this->progress += 10;
  if (this->progress == 100) {
    this->progress = 0;
    auto &nx = m.get_chunk(to_vec2(this->info_.direction));
    if (nx.building) {
      auto &building = nx.building.value();
      auto capability =
          this->transport_capability(m.ctx.get().efficiency_factor);

      building->input(m, this->buffer, capability);
    }
  }
}
} // namespace shapezx
