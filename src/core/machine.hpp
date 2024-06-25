#ifndef SHAPEZX_CORE_MACHINE
#define SHAPEZX_CORE_MACHINE

#include "../vec/vec.hpp"
#include "ore.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <format>
#include <iterator>
#include <memory>
#include <optional>
#include <ranges>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <variant>
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
  struct None {
    std::monostate inner;
  };

  struct Any {
    std::monostate inner;
  };

  struct Specific;

  struct Custom {
    Buffer inner;

    Capability merge(const Custom &oc) const {
      auto items = this->inner;
      for (const auto &[item, val] : oc.inner.items) {
        auto num = std::min(val, items.get(item));
        items[item] = num;
      }
      return Capability{.restriction = Custom{.inner = items}};
    }

    Capability merge(const Specific &oc) const {
      auto items = this->inner.items |
                   std::views::filter([&](const pair<Item, size_t> entry) {
                     auto const &inner = oc.inner;
                     return std::find(inner.cbegin(), inner.cend(),
                                      entry.first) != inner.cend();
                   }) |
                   std::ranges::to<std::unordered_map<Item, size_t>>();
      return Capability{.restriction = Custom{.inner = Buffer(items)}};
    }
  };

  struct Specific {
    vector<Item> inner;

    Capability merge(const Custom &oc) const { return oc.merge(*this); }

    Capability merge(const Specific &oc) const {
      auto items1 = this->inner;
      auto items2 = oc.inner;

      std::sort(items1.begin(), items1.end());
      std::sort(items1.begin(), items1.end());

      vector<Item> res;
      std::set_intersection(items1.begin(), items1.end(), items2.begin(),
                            items2.end(), std::back_inserter(res));

      return Capability{.restriction = Specific{.inner = res}};
    }
  };

  template <class... Ts> struct overloaded : Ts... {
    using Ts::operator()...;
  };

  std::variant<None, Any, Custom, Specific> restriction;

  static Capability none() { return {.restriction = None()}; }
  static Capability any() { return {.restriction = Any()}; }
  static Capability custom(const Buffer &buf) {
    return {.restriction = Custom(buf)};
  }
  static Capability specific(const vector<Item> &items) {
    return {.restriction = Specific(items)};
  }

  template <typename S> auto &get() {
    return std::get<S>(this->restriction).inner;
  }

  template <typename S> const auto &get() const {
    return std::get<S>(this->restriction).inner;
  }

  bool accepts(Item item, size_t q) const {
    return this->num_accepts(item)
        .transform([=](auto const val) { return val >= q; })
        .value_or(true);
  }

  optional<size_t> num_accepts(Item item) const {
    return std::visit(
        overloaded{[](None) { return optional<size_t>{0}; },
                   [](Any) { return optional<size_t>{}; },
                   [&](const Custom &c) { return optional(c.inner.get(item)); },
                   [&](const Specific &s) {
                     if (std::find(s.inner.cbegin(), s.inner.cend(), item) !=
                         s.inner.cend()) {
                       return optional<size_t>{};
                     } else {
                       return optional<size_t>{0};
                     }
                   }},
        this->restriction);
  }

  Capability merge(Capability other) const {
    return std::visit(
        overloaded{
            [](None) { return Capability::none(); }, [=](Any) { return other; },
            [&](const Custom &c) {
              return std::visit(
                  overloaded{[](None) { return Capability::none(); },
                             [this](Any) { return *this; },
                             [&](auto const &oc) { return c.merge(oc); }},
                  other.restriction);
            },
            [&](const Specific &s) {
              return std::visit(
                  overloaded{[](None) { return Capability::none(); },
                             [this](Any) { return *this; },
                             [&](auto const &oc) { return s.merge(oc); }},
                  other.restriction);
            }},
        this->restriction);
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

    return Capability::custom(Buffer(
        this->buffer.items |
        std::views::transform([=](std::pair<Item, size_t> t) {
          return std::make_pair(
              t.first, std::min(size_t(4 * efficiency_factor), t.second));
        }) |
        std::ranges::to<std::unordered_map<Item, size_t>>()));
  }
};

struct Cutter final : public Building {
  BuildingInfo info_;
  Buffer in;
  Buffer out;

  explicit Cutter(Direction direction_)
      : info_(BuildingType::Cutter, {2, 1}, direction_) {}

  Cutter(const Cutter &) = default;

  BuildingInfo info() const override { return this->info_; }

  vector<vec::Vec2<size_t>> input_positons(MapAccessor &) const override;

  Capability transport_capability() const {
    return Capability::specific({IRON_ORE});
  }

  void input(MapAccessor &, Buffer &, Capability) override;

  void update(MapAccessor) override;

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

  void input(MapAccessor &, Buffer &, Capability) override;

  vector<vec::Vec2<size_t>> input_positons(MapAccessor &) const override;

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

template <> struct std::formatter<shapezx::Buffer> {
  constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

  auto format(const shapezx::Buffer &buf, std::format_context &ctx) const {
    std::format_to(ctx.out(), "{{\n");
    for (auto [item, num] : buf.items) {
      std::format_to(ctx.out(), "{}: {}\n", item.name, num);
    }

    return std::format_to(ctx.out(), "}}");
  }
};
} // namespace std

#endif
