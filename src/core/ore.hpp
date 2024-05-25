#ifndef SHAPEZX_CORE_ORE
#define SHAPEZX_CORE_ORE

#include <cstdint>
#include <string>

namespace shapezx {

using std::int64_t;
using std::string;

struct Ore {
  string name;
  int64_t value;
};
} // namespace shapezx

#endif