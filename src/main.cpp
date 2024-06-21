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
#include <gtkmm/listbox.h>
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
    this->set_expand(true);
    this->set_halign(Gtk::Align::FILL);
    this->set_valign(Gtk::Align::FILL);
    // this->set();
  }

  void on_clicked() override {
    if (auto placing = this->ui_state.get().machine_selected;
        placing && !this->map_accessor.current_chunk().building) {
      std::unique_ptr<shapezx::Building> machine;
      switch (*placing) {
      case shapezx::BuildingType::Miner:
        machine =
            shapezx::Building::create<shapezx::Miner>(shapezx::Direction::Down);
        break;
      case shapezx::BuildingType::Belt:
        machine =
            shapezx::Building::create<shapezx::Belt>(shapezx::Direction::Down);
        break;
      case shapezx::BuildingType::Cutter:
        machine = shapezx::Building::create<shapezx::Cutter>(
            shapezx::Direction::Down);
        break;
      case shapezx::BuildingType::TrashCan:
        machine = shapezx::Building::create<shapezx::TrashCan>(
            shapezx::Direction::Down);
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

  explicit Map(const UIState &ui_state, shapezx::State &game_state) {
    this->set_expand(true);
    this->set_valign(Gtk::Align::FILL);
    this->set_halign(Gtk::Align::FILL);

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

struct Connections {
  std::vector<sigc::connection> conns;

  void add(sigc::connection &&conn) { conns.push_back(std::move(conn)); }

  ~Connections() {
    for (auto &conn : this->conns) {
      conn.disconnect();
    }
  }
};

class MainGame : public Gtk::Window {
protected:
  shapezx::State state;
  UIState ui_state;
  sigc::signal<void(shapezx::BuildingType)> on_placing_machine_begin;
  Map map;
  Connections conns;
  Gtk::Box box;
  shapezx::ui::MachineSelector machines;

public:
  explicit MainGame(shapezx::State &&state)
      : state(std::move(state)), map(this->ui_state, this->state),
        box(Gtk::Orientation::VERTICAL) {

    this->conns.add(Glib::signal_timeout().connect(
        [this]() {
          this->state.update();
          return true;
        },
        20));

    this->conns.add(this->machines.signal_machine_selected().connect(
        [this](shapezx::BuildingType type) {
          this->ui_state.machine_selected = type;
        }));

    this->set_title("shapezx");
    this->set_default_size(1920, 1080);

    this->box.set_expand(false);
    // this->box.set_vexpand(true);
    this->box.set_valign(Gtk::Align::FILL);
    this->box.set_halign(Gtk::Align::FILL);

    this->box.append(this->map);
    this->box.append(this->machines);

    this->set_expand(false);
    // this->set_valign(Gtk::Align::FILL);
    // this->set_halign(Gtk::Align::FILL);

    this->set_child(this->box);
  }

  MainGame(const MainGame &) = delete;
  MainGame(MainGame &&) = delete;
};

int main(int argc, char **argv) {
  auto state = shapezx::State(20, 30);

  auto app = Gtk::Application::create();

  return app->make_window_and_run<MainGame>(argc, argv, std::move(state));
}