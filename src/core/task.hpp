#ifndef SHAPEZX_CORE_TASK
#define SHAPEZX_CORE_TASK

#include "machine.hpp"

namespace shapezx {

struct Task {
  Buffer target_;
  bool completed = false;

  bool update(State &ctx);
};

} // namespace shapezx

#endif