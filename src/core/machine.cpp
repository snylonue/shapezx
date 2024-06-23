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
        out->input(m, this->ores, {Capability::Custom, this->ores});
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
