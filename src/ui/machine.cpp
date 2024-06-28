#include "machine.hpp"
#include <memory>

namespace shapezx::ui {
std::unique_ptr<Machine> Machine::create(BuildingType type, vec::Vec2<> pos,
                                         Direction d, std::uint32_t id,
                                         UIState &ui_state) {
  switch (type) {
  case BuildingType::Miner:
    return std::make_unique<Machine>(
        type, Gdk::Pixbuf::create_from_file("./assets/miner.png"), pos, d, id,
        ui_state);
  case BuildingType::TrashCan:
    return std::make_unique<Machine>(
        type, Gdk::Pixbuf::create_from_file("./assets/trash.png"), pos, d,
        id, ui_state);
  case BuildingType::Belt:
    return std::make_unique<Machine>(
        type, Gdk::Pixbuf::create_from_file("./assets/belt.png"), pos, d, id,
        ui_state);
  case BuildingType::Cutter:
    return std::make_unique<Machine>(
        type, Gdk::Pixbuf::create_from_file("./assets/cutter_large.png"), pos,
        d, id, ui_state);
  case BuildingType::TaskCenter:
    return std::make_unique<TaskCenter>(
        Gdk::Pixbuf::create_from_file("./assets/task_center.png"), pos, d, id,
        ui_state);
  default:
    std::unreachable();
  }
}
} // namespace shapezx::ui