#ifndef SHAPEZX_CORE_ORE
#define SHAPEZX_CORE_ORE

#include <nlohmann/json.hpp>

#include <array>
#include <cstdint>
#include <functional>
#include <string>

namespace shapezx {

using nlohmann::json;

using std::array;
using std::int64_t;
using std::string;

struct Item {
  string name;
  int64_t value;

  bool operator==(const Item &) const = default;
  bool operator<(const Item &other) const {
    return (this->name == other.name) ? this->name < other.name
                                      : this->value < other.value;
  };
};

void to_json(json &j, const Item &item);
void from_json(const json &j, Item &item);


static const Item IRON_ORE{"iron ore", 30};
static const Item GOLD{"gold", 60};
static const array<Item, 2> ORES{IRON_ORE, GOLD};

static const Item IRON{"iron", 40};
static const Item STONE{"stone", 1};

} // namespace shapezx

namespace std {
template <> struct std::hash<shapezx::Item> {
  std::size_t operator()(const shapezx::Item &item) const noexcept {
    return std::hash<std::string>{}(item.name);
  }
};
} // namespace std

#endif