#include "core/core.hpp"
#include "core/machine.hpp"
#include "ui/machine.hpp"
#include "vec/vec.hpp"

#include <gtkmm.h>
#include <gtkmm/application.h>
#include <gtkmm/button.h>
#include <gtkmm/enums.h>
#include <gtkmm/gridview.h>
#include <gtkmm/label.h>
#include <gtkmm/listboxrow.h>
#include <gtkmm/widget.h>
#include <sigc++/connection.h>
#include <sigc++/signal.h>

#include <cstddef>
#include <format>
#include <functional>
#include <memory>
#include <optional>
#include <ranges>
#include <utility>
#include <vector>

struct UIState {
  std::optional<shapezx::BuildingType> machine_selected = std::nullopt;
};

class Chunk : public Gtk::Button {
public:
  shapezx::MapAccessor map_accessor;
  std::reference_wrapper<const UIState> ui_state;

  explicit Chunk(shapezx::MapAccessor current_chunk_, const UIState &ui_state)
      : map_accessor(current_chunk_), ui_state(ui_state) {
    this->reset_label();
    this->set_expand();
  }

  void on_clicked() override {
    if (auto placing = this->ui_state.get().machine_selected;
        placing && !this->map_accessor.current_chunk().building) {
      std::unique_ptr<shapezx::Building> machine;
      switch (*placing) {
      case shapezx::BuildingType::Miner:
        machine = shapezx::Building::create<shapezx::Miner>(
            shapezx::Direction::Down, 1);
        break;
      case shapezx::BuildingType::Belt:
        machine = shapezx::Building::create<shapezx::Belt>(
            shapezx::Direction::Down, 1);
        break;
      case shapezx::BuildingType::Cutter:
        machine = shapezx::Building::create<shapezx::Cutter>(
            shapezx::Direction::Down, 1);
        break;
      case shapezx::BuildingType::TrashCan:
        machine = shapezx::Building::create<shapezx::TrashCan>(
            shapezx::Direction::Down, 1);
        break;
      default:
        return;
      }

      this->map_accessor.add_machine(std::move(machine));
      this->reset_label();
    }
  }

  void reset_label() {
    this->set_label(std::format("{} at ({} {})",
                                this->map_accessor.current_chunk()
                                    .building
                                    .transform([](auto const &b) {
                                      return std::format("{}", b->info().type);
                                    })
                                    .value_or("none"),
                                this->map_accessor.pos[0],
                                this->map_accessor.pos[1]));
  }
};

class Map : public Gtk::Grid {
public:
  std::vector<Chunk> chunks;

  explicit Map(UIState const &ui_state, shapezx::State &game_state) {
    for (auto const r :
         std::views::iota(std::size_t(0), game_state.map.height)) {
      for (auto const c :
           std::views::iota(std::size_t(0), game_state.map.width)) {
        auto &chunk = this->chunks.emplace_back(
            game_state.create_accessor_at({r, c}), ui_state);
        this->attach(chunk, c, r);
      }
    }
  }
};

class MainGame : public Gtk::Window {
protected:
  shapezx::State state;
  UIState ui_state;
  sigc::signal<void(shapezx::BuildingType)> on_placing_machine_begin;
  Map map;
  sigc::connection timer_conn;
  sigc::connection machine_selected_conn;
  Gtk::ListBox box;
  shapezx::ui::MachineSelector machines;

public:
  explicit MainGame(shapezx::State &&state)
      : state(std::move(state)), map(this->ui_state, this->state) {

    // should be disconnected before destructing
    this->timer_conn = Glib::signal_timeout().connect(
        [this]() {
          this->state.update();
          return true;
        },
        20);

    this->machine_selected_conn =
        this->machines.signal_machine_selected().connect(
            [this](shapezx::BuildingType type) {
              this->ui_state.machine_selected = type;
            });

    this->set_title("shapezx");
    this->set_default_size(1920, 1080);

    this->box.append(this->map);
    this->box.append(this->machines);

    this->set_child(this->box);
  }

  MainGame(const MainGame &) = delete;
  MainGame(MainGame &&) = delete;

  ~MainGame() {
    this->timer_conn.disconnect();
    this->machine_selected_conn.disconnect();
  }
};

int main(int argc, char **argv) {
  auto core = shapezx::Map{2, 3};
  auto state = shapezx::State(std::move(core), shapezx::Context());

  auto app = Gtk::Application::create();

  return app->make_window_and_run<MainGame>(argc, argv, std::move(state));
}