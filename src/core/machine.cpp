#include "machine.hpp"
#include "core.hpp"
#include "ore.hpp"

#include <algorithm>
#include <format>
#include <iostream>

namespace shapezx {

void output_to(MapAccessor m, vec::Vec2<ssize_t> at, vec::Vec2<> from,
               Buffer &buf, Capability cap) {
  auto [out_chk, acc] = m.get_chunk_and_accessor(at);
  std::cout << std::format("{}\n", acc.pos);
  if (out_chk.building) {
    auto &out = out_chk.building.value();
    std::cout << std::format("{}\n", out->info().type);
    if (std::ranges::any_of(out->input_positons(acc), [=](const auto d) {
          std::cout << std::format("{}\n", d);
          return d == from;
        })) {
      std::cout << "ok?\n";
      out->input(acc, buf, cap);
    }
  }
}

void output_to(MapAccessor m, vec::Vec2<ssize_t> at, Buffer &buf,
               Capability cap) {
  output_to(m, at, m.pos, buf, cap);
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

void to_json(json &j, const Building &p) { p.to_json(j); }

void from_json(const json &j, Building &p) { p.from_json(j); }

void Miner::update(MapAccessor m) {
  const auto &chk = m.current_chunk();
  if (chk.ore) {
    const auto &ore = chk.ore.value();
    this->ores.increase(ore, m.ctx.get().eff.miner);
  }

  if (!this->ores.empty()) {
    std::cout << std::format("miner: {}\n", this->ores);
    output_to(m, to_vec2(this->info_.direction), this->ores,
              Capability::custom(this->ores));
  }
}

vector<vec::Vec2<size_t>> Belt::input_positons(MapAccessor &m) const {
  return {m.relative_pos_by(to_vec2(opposite_of(this->info_.direction)))};
}

void Belt::input(MapAccessor &m, Buffer &buf, Capability cap) {
  cap = cap.merge(this->transport_capability(m.ctx.get().eff.belt));

  consume(buf, this->buffer, cap);
}

void Belt::update(MapAccessor m) {
  if (!this->buffer.empty()) {
    std::cout << std::format("belt: {}\n", this->buffer);

    this->progress += 10;
    if (this->progress == 100) {
      this->progress = 0;
      auto capability = this->transport_capability(m.ctx.get().eff.belt);
      output_to(m, to_vec2(this->info_.direction), this->buffer, capability);
    }
  }
}

vector<vec::Vec2<size_t>> Cutter::input_positons(MapAccessor &m) const {
  return {m.relative_pos_by(to_vec2(this->info_.direction))};
}

void Cutter::input(MapAccessor &, Buffer &buf, Capability cap) {
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
    auto d = this->info_.direction;
    auto select = [this](Item item) {
      return Capability::custom({.items = {{item, this->out.get(item)}}});
    };

    output_to(m, to_vec2(opposite_of(d)), this->out, select(IRON));
    output_to(m, to_vec2(opposite_of(d)) + to_vec2(right_of(d)),
              m.pos + to_vec2(right_of(d)), this->out, select(STONE));
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

Building *PlaceHolder::base(MapAccessor &m) {
  return m.map.get()[this->pos_].building->get();
}

const Building *PlaceHolder::base(MapAccessor &m) const {
  return m.map.get()[this->pos_].building->get();
}

void PlaceHolder::input(MapAccessor &m, Buffer &buf, Capability cap) {
  auto acc = m.relocate(this->pos_);
  return this->base(m)->input(acc, buf, cap);
}

vector<vec::Vec2<size_t>> PlaceHolder::input_positons(MapAccessor &m) const {
  auto acc = m.relocate(this->pos_);
  return this->base(m)->input_positons(acc);
}

void TaskCenter::input(MapAccessor &, Buffer &buf, Capability cap) {
  consume(buf, this->buffer, cap);
}

void TaskCenter::update(MapAccessor m) {
  for (auto &[item, num] : this->buffer.items) {
    m.ctx.get().take_item(item, num);
    num = 0;
  }
}

vector<vec::Vec2<size_t>> TaskCenter::input_positons(MapAccessor &m) const {
  std::cout << std::format("tc pos: {}\n", m.pos);
  return vector<vec::Vec2<ssize_t>>{{-1, 0}, {-1, 1}, {0, 2},  {1, 2},
                                    {2, 0},  {2, 1},  {0, -1}, {1, -1}} |
         std::views::transform(
             [&](const auto d) { return m.relative_pos_by(d); }) |
         std::ranges::to<vector>();
}
} // namespace shapezx
