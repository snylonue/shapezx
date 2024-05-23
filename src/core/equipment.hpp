#include <cstddef>
#include <cstdint>
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

class Context;

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
  None,
};

template <typename Item> struct Buffer {
  Item item;
  size_t num;
};

struct BuildingInfo {
  BuildingType type;
  int64_t efficiency;
  pair<size_t, size_t> size;
  Direction direction;
};

struct Building {
  virtual BuildingInfo info() const = 0;
  virtual unique_ptr<Building> clone() const = 0;

  virtual ~Building() = 0;
};

struct Miner final : public Building {
  BuildingInfo info_;

  explicit Miner(Direction direction_, int64_t efficiency_)
      : info_(BuildingType::Miner, efficiency_, {1, 1}, direction_) {}

  Miner(const Miner &) = default;

  BuildingInfo info() const override { return this->info_; }

  unique_ptr<Building> clone() const override {
    return std::make_unique<Miner>(*this);
  }

  ~Miner() override = default;
};

struct Belt final : public Building {
  BuildingInfo info_;

  explicit Belt(Direction direction_, int64_t efficiency_)
      : info_(BuildingType::Belt, efficiency_, {1, 1}, direction_) {}

  Belt(const Belt &) = default;

  BuildingInfo info() const override { return this->info_; }

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

  void input(Context &ctx) {}

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