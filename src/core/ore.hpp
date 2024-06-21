#ifndef SHAPEZX_CORE_ORE
#define SHAPEZX_CORE_ORE

#include <array>
#include <cstdint>
#include <string>

namespace shapezx {

using std::array;
using std::int64_t;
using std::string;

struct Item {
  string name;
  int64_t value;
};

static const Item IRON{"iron", 30};
static const Item GOLD{"gold", 60};
static const array<Item, 2> ORES{IRON, GOLD};
} // namespace shapezx

#endif