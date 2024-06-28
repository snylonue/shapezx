#include "ore.hpp"

namespace shapezx {
using nlohmann::json;

void to_json(json &j, const Item &item) {
  j = json{{"name", item.name}, {"value", item.value}};
}

void from_json(const json &j, Item &item) {
  j.at("name").get_to(item.name);
  j.at("value").get_to(item.value);
}
} // namespace shapezx