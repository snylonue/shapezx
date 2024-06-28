#ifndef SHAPEZX_CORE_TASK
#define SHAPEZX_CORE_TASK

#include "machine.hpp"

#include <nlohmann/json.hpp>

namespace shapezx {

struct Task {
  Buffer target_;
  bool completed_ = false;

  bool update(State &ctx);
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Task, target_, completed_);

} // namespace shapezx

#endif