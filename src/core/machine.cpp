#include "machine.hpp"
#include "core.hpp"
#include "ore.hpp"
#include <algorithm>

namespace shapezx {

void output_to(MapAccessor m, vec::Vec2<ssize_t> at, Buffer &buf,
               Capability cap) {
  auto [out_chk, acc] = m.get_chunk_and_accessor(at);
  if (out_chk.building) {
    auto &out = out_chk.building.value();
    if (std::ranges::any_of(out->input_positons(acc),
                            [&](const auto d) { return d == m.pos; })) {
      out->input(acc, buf, cap);
    }
  }
}

void consume(Buffer &from, Buffer &to, Capability cap) {
  for (auto &[item, val] : from.items) {
    auto accepts = cap.num_accepts(item).value_or(val);

    if (accepts) {
      val -= accepts;
      to.increase(item, accepts);
    }
  }
}

void Building::update(MapAccessor) {}

void Miner::update(MapAccessor m) {
  const auto &chk = m.current_chunk();
  if (chk.ore) {
    const auto &ore = chk.ore.value();
    this->ores.increase(ore, m.ctx.get().efficiency_factor);
  }

  if (!this->ores.empty()) {
    output_to(m, to_vec2(this->info_.direction), this->ores,
              Capability::custom(this->ores));
  }
}

vector<vec::Vec2<size_t>> Belt::input_positons(MapAccessor &m) const {
  return {m.relative_pos_by(to_vec2(opposite_of(this->info_.direction)))};
}

void Belt::input(MapAccessor &m, Buffer &buf, Capability cap) {
  cap = cap.merge(this->transport_capability(m.ctx.get().efficiency_factor));

  consume(buf, this->buffer, cap);
}

void Belt::update(MapAccessor m) {
  if (!this->buffer.empty()) {
    this->progress += 10;
    if (this->progress == 100) {
      this->progress = 0;
      auto capability =
          this->transport_capability(m.ctx.get().efficiency_factor);
      output_to(m, to_vec2(this->info_.direction), this->buffer, capability);
    }
  }
}

vector<vec::Vec2<size_t>> Cutter::input_positons(MapAccessor &m) const {
  return {m.relative_pos_by(to_vec2(this->info_.direction))};
}

void Cutter::input(MapAccessor &m, Buffer &buf, Capability cap) {
  cap = cap.merge(this->transport_capability());

  consume(buf, this->in, cap);
}

void Cutter::update(MapAccessor m) {
  if (this->in.get(IRON_ORE) && this->out.empty()) {
    this->in.increase(IRON_ORE, -1);
    this->out.increase(IRON, 1);
    this->out.increase(STONE, 1);
  }

  if (!this->out.empty()) {
    output_to(m, to_vec2(opposite_of(this->info_.direction)), this->out,
              Capability::specific({IRON}));

    output_to(m,
              to_vec2(opposite_of(this->info_.direction)) +
                  to_vec2(right_of(this->info_.direction)),
              this->out, Capability::specific({STONE}));
  }
}

vector<vec::Vec2<size_t>> TrashCan::input_positons(MapAccessor &m) const {
  return std::views::transform(ALL_DIRECTIONS, to_vec2) |
         std::views::transform(
             [&](const auto d) { return m.relative_pos_by(d); }) |
         std::ranges::to<vector>();
}

void TrashCan::input(MapAccessor &, Buffer &buf, Capability cap) {
  Buffer tmp;
  consume(buf, tmp, cap);
}
} // namespace shapezx
