#include "machine.hpp"
#include "core.hpp"
#include <algorithm>

namespace shapezx {

void Building::update(MapAccessor) {}

void Miner::update(MapAccessor m) {
  const auto &chk = m.current_chunk();
  if (chk.ore) {
    const auto &ore = chk.ore.value();
    this->ores.increase(ore, m.ctx.get().efficiency_factor);
  }

  if (!this->ores.empty()) {
    auto [out_chk, acc] =
        m.get_chunk_and_accessor(to_vec2(this->info_.direction));
    if (out_chk.building) {
      auto &out = out_chk.building.value();
      if (std::ranges::any_of(out->input_positons(acc),
                              [&](const auto d) { return d == m.pos; })) {
        out->input(m, this->ores, Capability::custom(this->ores));
      }
    }
  }
}

vector<vec::Vec2<size_t>> Belt::input_positons(MapAccessor &m) const {
  return {m.relative_pos_by(to_vec2(opposite_of(this->info_.direction)))};
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
  if (!this->buffer.empty()) {
  this->progress += 10;
  if (this->progress == 100) {
    this->progress = 0;
    auto [out_chk, acc] =
        m.get_chunk_and_accessor(to_vec2(this->info_.direction));
    if (out_chk.building) {
      auto &out = out_chk.building.value();
      if (std::ranges::any_of(out->input_positons(acc),
                              [&](const auto d) { return d == m.pos; })) {
        auto capability =
            this->transport_capability(m.ctx.get().efficiency_factor);

        out->input(m, this->buffer, capability);
        }
      }
    }
  }
}

vector<vec::Vec2<size_t>> Cutter::input_positons(MapAccessor &m) const {
  return {m.relative_pos_by(to_vec2(this->info_.direction))};
}

void Cutter::input(MapAccessor &m, Buffer &buf, Capability cap) {}

vector<vec::Vec2<size_t>> TrashCan::input_positons(MapAccessor &m) const {
  return std::views::transform(ALL_DIRECTIONS, to_vec2) |
         std::views::transform(
             [&](const auto d) { return m.relative_pos_by(d); }) |
         std::ranges::to<vector>();
}

void TrashCan::input(MapAccessor &, Buffer &buf, Capability cap) {
  for (auto &[item, val] : buf.items) {
    auto accepts = cap.num_accepts(item).value_or(val);

    if (accepts) {
      val -= accepts;
    }
  }
}
} // namespace shapezx
