#ifndef SHAPEZX_CORE_EQUIPMENT
#define SHAPEZX_CORE_EQUIPMENT

#include "../vec/vec.hpp"
#include "ore.hpp"

#include <cstddef>
#include <cstdint>
#include <format>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace shapezx {

using std::int64_t;
using std::optional;
using std::pair;
using std::size_t;
using std::string;
using std::unique_ptr;
using std::vector;

constexpr int64_t BASIC_EFF = 10;

enum class BuildingType {
  Miner,
  Belt,
  Cutter,
  TrashCan,
  PlaceHolder,
};

enum class Direction {
  Up,
  Down,
  Left,
  Right,
};

template <typename Item> struct Buffer {
  Item item;
  size_t num;

  Buffer() : num(0) {}

  void clear() { this->num = 0; }

  Buffer take() {
    auto ret = Buffer(*this);
    this->clear();

    return ret;
  }
};

struct Capability {
  vector<vec::Vec2<size_t>> positions;
  Buffer<Ore> items;
};

struct BuildingInfo {
  BuildingType type;
  int64_t efficiency;
  pair<size_t, size_t> size;
  Direction direction;
};

struct MapAccessor;

struct Building {
  virtual BuildingInfo info() const = 0;
  virtual unique_ptr<Building> clone() const = 0;

  virtual Capability input_capabilities(MapAccessor &) const { return {}; }
  virtual Capability output_capabilities(MapAccessor &) const { return {}; }

  virtual void input(Capability){};
  virtual Buffer<Ore> output(Capability) { return Buffer<Ore>(); };

  // update internal state by 1 tick
  virtual void update(MapAccessor);

  virtual ~Building() = 0;
};

struct Miner final : public Building {
  BuildingInfo info_;
  Buffer<Ore> ores;

  explicit Miner(Direction direction_, int64_t efficiency_)
      : info_(BuildingType::Miner, efficiency_, {1, 1}, direction_) {}

  Miner(const Miner &) = default;

  BuildingInfo info() const override { return this->info_; }

  unique_ptr<Building> clone() const override {
    return std::make_unique<Miner>(*this);
  }

  Capability output_capabilities(MapAccessor &) const override;

  void input(Capability) override;
  Buffer<Ore> output(Capability) override { return this->ores.take(); }

  void update(MapAccessor m) override;

  ~Miner() override = default;
};

struct Belt final : public Building {
  BuildingInfo info_;
  std::uint32_t progress = 0;

  explicit Belt(Direction direction_, int64_t efficiency_)
      : info_(BuildingType::Belt, efficiency_, {1, 1}, direction_) {}

  Belt(const Belt &) = default;

  BuildingInfo info() const override { return this->info_; }

  void update(MapAccessor) override;

  unique_ptr<Building> clone() const override {
    return std::make_unique<Belt>(*this);
  }

  ~Belt() override = default;
};

struct Cutter final : public Building {
  BuildingInfo info_;

  explicit Cutter(Direction direction_, int64_t efficiency_)
      : info_(BuildingType::Cutter, efficiency_, {2, 1}, direction_) {}

  Cutter(const Cutter &) = default;

  BuildingInfo info() const override { return this->info_; }

  unique_ptr<Building> clone() const override {
    return std::make_unique<Cutter>(*this);
  }

  ~Cutter() override = default;
};

struct TrashCan final : public Building {
  BuildingInfo info_;

  explicit TrashCan(Direction direction_, int64_t efficiency_)
      : info_(BuildingType::TrashCan, efficiency_, {1, 1}, direction_) {}

  TrashCan(const TrashCan &) = default;

  BuildingInfo info() const override { return this->info_; }

  unique_ptr<Building> clone() const override {
    return std::make_unique<TrashCan>(*this);
  }

  ~TrashCan() override = default;
};

} // namespace shapezx

namespace std {
template <> struct std::formatter<shapezx::BuildingType> {
  constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

  auto format(const shapezx::BuildingType &build_type,
              std::format_context &ctx) const {
    switch (build_type) {

    case shapezx::BuildingType::Miner:
      return std::format_to(ctx.out(), "miner");
    case shapezx::BuildingType::Belt:
      return std::format_to(ctx.out(), "belt");
    case shapezx::BuildingType::Cutter:
      return std::format_to(ctx.out(), "cutter");
    case shapezx::BuildingType::TrashCan:
      return std::format_to(ctx.out(), "trashcan");
    case shapezx::BuildingType::PlaceHolder:
      return std::format_to(ctx.out(), "placeholder");
    }
  }
};
} // namespace std

#endif
