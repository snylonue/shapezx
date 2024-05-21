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

} // namespace shapezx