#ifndef SHAPEZX_CORE_MACHINE
#define SHAPEZX_CORE_MACHINE

#include "../vec/vec.hpp"
#include "ore.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <format>
#include <memory>
#include <optional>
#include <ranges>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <unordered_map>
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

using ssize_t = std::make_signed_t<size_t>;

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
    return {0, -1};
  case Direction::Down:
    return {1, 0};
  case Direction::Right:
    return {0, 1};
  case Direction::Up:
    return {-1, 0};
  }
}

struct Buffer {
  std::unordered_map<Item, size_t> items;

  void clear() { this->items.clear(); }

  Buffer take() {
    auto ret = Buffer(*this);
    this->clear();

    return ret;
  }

  size_t get(Item it) const try {
    return this->items.at(it);
  } catch (const std::out_of_range &) {
    return 0;
  }

  void set(Item it, size_t n) { this->items[it] = n; }

  size_t increase(Item it, ssize_t n) {
    auto &cur = this->items[it];
    if (n >= 0 || cur >= size_t(-n)) {
      cur += n;
    } else {
      cur = 0;
    }
    return cur;
  }

  size_t &operator[](Item it) { return this->items[it]; }

  void merge(Buffer &other) { this->items.merge(other.items); }
  void merge(Buffer &&other) { this->items.merge(std::move(other.items)); }

  bool empty() const {
    return this->items.empty() ||
           std::ranges::all_of(
               this->items, [](const auto &pair) { return pair.second == 0; });
  }
};

struct Capability {
  enum Type {
    None,
    Any,
    Custom,
  };

  Type type;
  // valid if type == Custom
  Buffer items;

  static Capability none() { return {.type = None, .items = {}}; }
  static Capability any() { return {.type = Any, .items = {}}; }

  bool accepts(Item item, size_t q) const {
    switch (this->type) {
    case None:
      return false;
    case Any:
      return true;
    case Custom:
      return this->items.get(item) >= q;
    }
  }

  optional<size_t> num_accepts(Item item) const {
    switch (this->type) {
    case None:
      return false;
    case Any:
      return std::nullopt;
    case Custom:
      return this->items.get(item);
    }
  }

  Capability merge(Capability other) const {
    switch (this->type) {
    case None:
      return Capability::none();
    case Any:
      return other;
    case Custom:
      auto items = this->items;
      for (const auto &[item, val] : other.items.items) {
        auto num = std::min(val, items.get(item));
        items[item] = num;
      }

      return {.type = Custom, .items = items};
    }
  }
};

struct BuildingInfo {
  BuildingType type;
  pair<size_t, size_t> size;
  Direction direction;
};

struct MapAccessor;

struct Building {
  template <typename T> static unique_ptr<Building> create(Direction d) {
    return std::make_unique<T>(d);
  }

  virtual BuildingInfo info() const = 0;
  virtual unique_ptr<Building> clone() const = 0;

  virtual void input(MapAccessor &, Buffer &, Capability) {};
  virtual vector<vec::Vec2<size_t>> input_positons(MapAccessor &) const {
    return {};
  };
  // virtual Buffer output(MapAccessor &, Capability) { return Buffer(); };

  // update internal state by 1 tick
  virtual void update(MapAccessor);

  virtual ~Building() = default;
};

struct Miner final : public Building {
  BuildingInfo info_;
  Buffer ores;

  explicit Miner(Direction direction_)
      : info_(BuildingType::Miner, {1, 1}, direction_) {}

  Miner(const Miner &) = default;

  BuildingInfo info() const override { return this->info_; }

  unique_ptr<Building> clone() const override {
    return std::make_unique<Miner>(*this);
  }

  void update(MapAccessor m) override;

  ~Miner() override = default;
};

struct Belt final : public Building {
  BuildingInfo info_;
  std::uint32_t progress = 0;
  Buffer buffer;

  explicit Belt(Direction direction_)
      : info_(BuildingType::Belt, {1, 1}, direction_) {}

  Belt(const Belt &) = default;

  BuildingInfo info() const override { return this->info_; }

  void input(MapAccessor &, Buffer &, Capability) override;

  vector<vec::Vec2<size_t>> input_positons(MapAccessor &) const override;

  void update(MapAccessor) override;

  unique_ptr<Building> clone() const override {
    return std::make_unique<Belt>(*this);
  }

  ~Belt() override = default;

  Capability transport_capability(int64_t efficiency_factor) const {
    if (this->buffer.empty()) {
      return Capability::any();
    }

    return Capability{
        .type = Capability::Custom,
        .items = {this->buffer.items |
                  std::views::transform([=](std::pair<Item, size_t> t) {
                    return std::make_pair(
                        t.first,
                        std::min(size_t(4 * efficiency_factor), t.second));
                  }) |
                  std::ranges::to<std::unordered_map<Item, size_t>>()}};
  }
};

struct Cutter final : public Building {
  BuildingInfo info_;

  explicit Cutter(Direction direction_)
      : info_(BuildingType::Cutter, {2, 1}, direction_) {}

  Cutter(const Cutter &) = default;

  BuildingInfo info() const override { return this->info_; }

  unique_ptr<Building> clone() const override {
    return std::make_unique<Cutter>(*this);
  }

  ~Cutter() override = default;
};

struct TrashCan final : public Building {
  BuildingInfo info_;

  explicit TrashCan(Direction direction_)
      : info_(BuildingType::TrashCan, {1, 1}, direction_) {}

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
