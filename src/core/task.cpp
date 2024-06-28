#include "task.hpp"
#include "core.hpp"

namespace shapezx {
bool Task::update(State &ctx) {
  if (!this->completed) {
    for (auto [item, num] : this->target_.items) {
      if (ctx.store.get(item) < num) {
        return false;
      }
    }

    this->completed = true;
    return true;
  }
  return false;
}
} // namespace shapezx