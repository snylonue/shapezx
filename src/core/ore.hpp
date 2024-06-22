#ifndef SHAPEZX_CORE_ORE
#define SHAPEZX_CORE_ORE

#include <array>
#include <cstdint>
#include <functional>
#include <string>

namespace shapezx {

using std::array;
using std::int64_t;
using std::string;

struct Item {
  string name;
  int64_t value;

  bool operator==(const Item &) const = default;
};

static const Item IRON_ORE{"iron ore", 30};
static const Item GOLD{"gold", 60};
static const array<Item, 2> ORES{IRON_ORE, GOLD};

static const Item IRON{"iron", 40};
static const Item STONE{"stone", 1};
} // namespace shapezx

template <> struct std::hash<shapezx::Item> {
  std::size_t operator()(const shapezx::Item &item) const noexcept {
    return std::hash<std::string>{}(item.name);
  }
};

#endif