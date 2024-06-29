#include "machine.hpp"
#include <memory>

namespace shapezx::ui {
std::unique_ptr<Machine>
Machine::create(BuildingType type, vec::Vec2<> pos, Direction d,
                std::uint32_t id, UIState &ui_state,
                sigc::signal<void(std::uint32_t)> machine_removed) {
  switch (type) {
  case BuildingType::Miner:
    return std::make_unique<Machine>(
        type, Gdk::Pixbuf::create_from_file("./assets/miner.png"), pos, d, id,
        ui_state, machine_removed);
  case BuildingType::TrashCan:
    return std::make_unique<Machine>(
        type, Gdk::Pixbuf::create_from_file("./assets/trash.png"), pos, d, id,
        ui_state, machine_removed);
  case BuildingType::Belt:
    return std::make_unique<Machine>(
        type, Gdk::Pixbuf::create_from_file("./assets/belt.png"), pos, d, id,
        ui_state, machine_removed);
  case BuildingType::Cutter:
    return std::make_unique<Machine>(
        type, Gdk::Pixbuf::create_from_file("./assets/cutter_large.png"), pos,
        d, id, ui_state, machine_removed);
  case BuildingType::TaskCenter:
    return std::make_unique<TaskCenter>(
        Gdk::Pixbuf::create_from_file("./assets/task_center.png"), pos, d, id,
        ui_state, machine_removed);
  default:
    std::unreachable();
  }
}
} // namespace shapezx::ui