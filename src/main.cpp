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
  shapezx::vec::Vec2<std::size_t> pos;
  std::reference_wrapper<const UIState> ui_state;
  std::reference_wrapper<shapezx::State> game_state;

  explicit Chunk(shapezx::vec::Vec2<std::size_t> pos_, const UIState &ui_state,
                 shapezx::State &game_state)
      : pos(pos_), ui_state(ui_state), game_state(game_state) {
    this->reset_label();
    this->set_expand();
  }

  void on_clicked() override {
    if (auto placing = this->ui_state.get().machine_selected) {
      std::unique_ptr<shapezx::Building> machine;
      switch (*placing) {
      case shapezx::BuildingType::Miner:
        machine = std::make_unique<shapezx::Miner>(shapezx::Direction::Down, 1);
        break;
      case shapezx::BuildingType::Belt:
      case shapezx::BuildingType::Cutter:
      case shapezx::BuildingType::TrashCan:
      case shapezx::BuildingType::PlaceHolder:
        return;
        break;
      }

      this->game_state.get().map.add_machine(this->pos, std::move(machine));
      this->reset_label();
    }
  }

  void reset_label() {
    this->set_label(std::format("{} at ({} {})",
                                this->game_state.get()
                                    .map[this->pos]
                                    .building
                                    .transform([](auto const &b) {
                                      return std::format("{}", b->info().type);
                                    })
                                    .value_or("none"),
                                this->pos[0], this->pos[1]));
  }
};

class Map : public Gtk::Grid {
public:
  std::vector<Chunk> chunks;

  explicit Map(const UIState &ui_state, shapezx::State &game_state) {
    for (auto const r :
         std::views::iota(std::size_t(0), game_state.map.height)) {
      for (auto const c :
           std::views::iota(std::size_t(0), game_state.map.width)) {
        auto &chunk = this->chunks.emplace_back(shapezx::vec::Vec2(r, c),
                                                ui_state, game_state);
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
            [&](shapezx::BuildingType type) {
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