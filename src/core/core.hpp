#include "equipment.hpp"
#include "ore.hpp"

#include <cstddef>
#include <memory>
#include <optional>
#include <vector>

namespace shapezx {

using std::unique_ptr;
using std::vector;
using std::size_t;
using std::nullopt;

struct Chunk {
  optional<Ore> ore;
  optional<unique_ptr<Building>> building;
  
  Chunk() = default;
  Chunk(const Chunk &chunk): ore(chunk.ore) {
    if (chunk.building.has_value()) {
      this->building = chunk.building.value()->clone();
    } else {
      this->building = nullopt;
    }
  }
};

struct Map {
  vector<Chunk> chunks;
  size_t height;
  size_t width;

  // caller must guarantee that chks.size() == h * w
  Map(const vector<Chunk>& chks, size_t h, size_t w): chunks(chks), height(h), width(w) {}
  Map(size_t h, size_t w): chunks(h * w), height(h), width(w) {} 
};
} // namespace shapezx