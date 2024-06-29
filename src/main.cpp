#include "core/core.hpp"
#include "core/machine.hpp"
#include "core/ore.hpp"
#include "ui/machine.hpp"
#include "vec/vec.hpp"

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <gdk/gdkkeysyms.h>
#include <gdkmm/enums.h>
#include <glib.h>
#include <glibmm/refptr.h>
#include <glibmm/signalproxy.h>
#include <gtkmm.h>
#include <gtkmm/application.h>
#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/enums.h>
#include <gtkmm/eventcontroller.h>
#include <gtkmm/eventcontrollerkey.h>
#include <gtkmm/gridview.h>
#include <gtkmm/image.h>
#include <gtkmm/label.h>
#include <gtkmm/listbox.h>
#include <gtkmm/listboxrow.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/widget.h>
#include <gtkmm/window.h>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <sigc++/connection.h>
#include <sigc++/signal.h>

#include <cstddef>
#include <format>
#include <functional>
#include <iostream>
#include <memory>
#include <optional>
#include <ranges>
#include <unordered_map>
#include <utility>
#include <vector>

using nlohmann::json;
using shapezx::ui::Connections;
using shapezx::ui::UIState;

class Chunk : public Gtk::Button {
  static Gtk::Image load_icon(const shapezx::Item &item) {
    if (item == shapezx::IRON_ORE) {
      return Gtk::Image{"./assets/iron.png"};
    }
    if (item == shapezx::GOLD) {
      return Gtk::Image{"./assets/gold.png"};
    }
    if (item == shapezx::STONE) {
      return Gtk::Image{"./assets/stone.png"};
    }
    return {};
  }

public:
  using sig_machine_placed = sigc::signal<void(
      std::vector<shapezx::vec::Vec2<>>, shapezx::Building *)>;

  shapezx::MapAccessor map_accessor;
  std::reference_wrapper<UIState> ui_state;
  sig_machine_placed machine_placed;

  Gtk::Image ore_icon;

  explicit Chunk(shapezx::MapAccessor current_chunk_, UIState &ui_state)
      : map_accessor(current_chunk_), ui_state(ui_state) {
    this->reset_label();
    this->set_expand(true);
    this->set_halign(Gtk::Align::FILL);
    this->set_valign(Gtk::Align::FILL);
    if (auto ore = current_chunk_.current_chunk().ore; ore) {
      this->ore_icon = load_icon(*ore);
      this->set_child(this->ore_icon);
    }
  }

  void on_clicked() override {
    if (auto &state = this->ui_state.get();
        state.machine_selected &&
        !this->map_accessor.current_chunk().building) {
      auto &placing = state.machine_selected;
      auto &dir = state.direction;
      std::unique_ptr<shapezx::Building> machine;
      switch (*placing) {
      case shapezx::BuildingType::Miner:
        machine = std::make_unique<shapezx::Miner>(dir.value());
        break;
      case shapezx::BuildingType::Belt:
        machine = std::make_unique<shapezx::Belt>(dir.value());
        break;
      case shapezx::BuildingType::Cutter:
        machine = std::make_unique<shapezx::Cutter>(dir.value());
        break;
      case shapezx::BuildingType::TrashCan:
        machine = std::make_unique<shapezx::TrashCan>(dir.value());
        break;
      case shapezx::BuildingType::TaskCenter:
        machine = std::make_unique<shapezx::TaskCenter>();
        break;
      default:
        return;
      }

      auto ref = machine.get();
      auto modified = this->map_accessor.add_machine(std::move(machine));
      this->machine_placed.emit(modified, ref);

      placing.reset();
      dir.reset();
    }
  }

  void reset_label() {
    this->set_label(
        std::format("{}\nat ({} {})\nwith {}",
                    this->map_accessor.current_chunk()
                        .building
                        .transform([](auto const &b) {
                          return std::format("{}", b->info().type);
                        })
                        .value_or("none"),
                    this->map_accessor.pos[0], this->map_accessor.pos[1],
                    this->map_accessor.current_chunk()
                        .ore.transform([](auto const &ore) { return ore.name; })
                        .value_or("")));
  }

  sig_machine_placed signal_machine_placed() const {
    return this->machine_placed;
  }
};

struct IdGenerator {
  std::uint32_t cur = 0;

  std::uint32_t gen() {
    this->cur += 1;
    return this->cur;
  }
};

class Map : public Gtk::Grid {
public:
  std::vector<Chunk> chunks;
  std::unordered_map<std::uint32_t, std::unique_ptr<shapezx::ui::Machine>>
      machines;
  Connections conns;
  IdGenerator id_;

  explicit Map(UIState &ui_state, shapezx::State &game_state) {
    this->set_expand(true);
    this->set_valign(Gtk::Align::FILL);
    this->set_halign(Gtk::Align::FILL);

    auto place_machine = [&, width = game_state.map.width](
                             std::vector<shapezx::vec::Vec2<>> v,
                             shapezx::Building *ref) {
      auto id = this->id_.gen();
      auto it = this->machines
                    .insert(std::make_pair(
                        id, shapezx::ui::Machine::create(ref->info().type, v[0],
                                                         ref->info().direction,
                                                         id, ui_state)))
                    .first;

      auto &machine = it->second;

      conns.add(machine->signal_machine_removed().connect(
          [&game_state, width = game_state.map.width, this](std::uint32_t id) {
            auto &m = this->machines.at(id);
            auto pos = m->pos_;
            auto acc = game_state.create_accessor_at(pos);
            this->remove(*m);
            auto res = acc.remove_machine();

            for (auto chk : res) {
              this->attach(this->chunks[chk[0] * width + chk[1]], chk[1],
                           chk[0]);
            }
          }));

      for (auto chk : v) {
        this->remove(this->chunks[chk[0] * width + chk[1]]);
      }
      auto [w, h] = ref->size();
      auto get = [](size_t i) {
        return [=](const shapezx::vec::Vec2<> &v) { return v[i]; };
      };

      auto r = std::ranges::min(v | std::ranges::views::transform(get(0)));
      auto c = std::ranges::min(v | std::ranges::views::transform(get(1)));

      this->attach(*machine, c, r, w, h);
    };

    for (auto const r :
         std::views::iota(std::size_t(0), game_state.map.height)) {
      for (auto const c :
           std::views::iota(std::size_t(0), game_state.map.width)) {
        auto &chunk = this->chunks.emplace_back(
            game_state.create_accessor_at({r, c}), ui_state);
        conns.add(chunk.signal_machine_placed().connect(place_machine));
        this->attach(chunk, c, r);
      }
    }

    for (auto const r :
         std::views::iota(std::size_t(0), game_state.map.height)) {
      for (auto const c :
           std::views::iota(std::size_t(0), game_state.map.width)) {
        auto acc = game_state.create_accessor_at({r, c});
        if (auto &building = acc.current_chunk().building;
            building && building.value()->info().type !=
                            shapezx::BuildingType::PlaceHolder) {
          auto ref = building->get();
          auto v =
              shapezx::rect_iter(ref->relative_rect()) |
              std::ranges::views::transform(
                  [acc](std::pair<shapezx::ssize_t, shapezx::ssize_t> pair) {
                    return acc.relative_pos_by({pair.first, pair.second});
                  }) |
              std::ranges::to<std::vector<shapezx::vec::Vec2<>>>();
          place_machine(v, ref);
        }
      }
    }
  }
};

class UpgradeMachine final : public Gtk::Window {
public:
  std::reference_wrapper<shapezx::State> game_state_;

  Gtk::Box selector;
  std::vector<Gtk::Button> options;

  Connections conns;

  UpgradeMachine(shapezx::State &game_state)
      : game_state_(game_state), selector(Gtk::Orientation::VERTICAL) {
    auto connect = [this](auto &but, auto sig) {
      this->conns.add(but.signal_clicked().connect(sig));
    };

    auto &miner = this->options.emplace_back("Upgrade miner");
    connect(miner, [this]() {
      this->game_state_.get().eff.miner += 1;
      this->destroy();
    });

    auto &belt = this->options.emplace_back("Upgrade belt");
    connect(belt, [this]() {
      this->game_state_.get().eff.belt += 1;
      this->destroy();
    });

    auto &cutter = this->options.emplace_back("Upgrade cutter");
    connect(cutter, [this]() {
      this->game_state_.get().eff.cutter += 1;
      this->destroy();
    });

    for (auto &but : this->options) {
      this->selector.append(but);
    }

    this->set_child(this->selector);
  }
};

class MainGame final : public Gtk::Window {
protected:
  shapezx::State state;
  std::reference_wrapper<shapezx::Global> global_state_;
  UIState ui_state;
  sigc::signal<void(shapezx::BuildingType)> on_placing_machine_begin;
  Glib::RefPtr<Gtk::EventControllerKey> ev_key;
  Map map;
  Gtk::ScrolledWindow map_window;
  Connections conns;
  Gtk::Box box;
  shapezx::ui::MachineSelector machines;
  UpgradeMachine upgrade_machine;
  std::filesystem::path save_path;

public:
  explicit MainGame(shapezx::State &&state, shapezx::Global &global_state,
                    const std::filesystem::path &path)
      : state(std::move(state)), global_state_(global_state),
        ev_key(Gtk::EventControllerKey::create()),
        map(this->ui_state, this->state), box(Gtk::Orientation::VERTICAL),
        upgrade_machine(this->state), save_path(path) {

    std::cout << "6\n";

    this->conns.add(Glib::signal_timeout().connect(
        [this]() {
          this->state.update([this]() { this->upgrade_machine.set_visible(); },
                             this->global_state_);
          return true;
        },
        50));

    this->conns.add(this->machines.signal_machine_selected().connect(
        [this](shapezx::BuildingType type) {
          this->ui_state.machine_selected = type;
          this->ui_state.direction = shapezx::Direction::Up;
        }));

    this->conns.add(this->machines.signal_remove_selected().connect([this]() {
      if (!this->ui_state.map_locked) {
        this->ui_state.machine_removing = true;
        this->ui_state.machine_selected.reset();
        this->ui_state.direction.reset();
      }
    }));

    this->conns.add(this->machines.signal_save().connect(
        [this]() { this->state.save_to(this->save_path); }));

    this->conns.add(this->signal_destroy().connect(
        [this]() { this->state.save_to(this->save_path); }));

    this->ev_key->set_propagation_phase(Gtk::PropagationPhase::CAPTURE);
    this->conns.add(this->ev_key->signal_key_pressed().connect(
        [this](guint keyval, guint, Gdk::ModifierType) {
          std::cout << "received event\n";
          if ((keyval == GDK_KEY_R || keyval == GDK_KEY_r) &&
              this->ui_state.machine_selected) {
            std::cout << "updating direction\n";
            this->ui_state.direction =
                this->ui_state.direction
                    .or_else(
                        []() { return std::optional(shapezx::Direction::Up); })
                    .transform(
                        [](auto const d) { return shapezx::right_of(d); });
            return true;
          }

          return false;
        },
        false));
    this->add_controller(this->ev_key);

    this->set_title("shapezx");
    this->set_default_size(1920, 1080);

    this->box.set_expand(false);
    // this->box.set_vexpand(true);
    this->box.set_valign(Gtk::Align::FILL);
    this->box.set_halign(Gtk::Align::FILL);

    this->map_window.set_child(this->map);
    this->box.append(this->map_window);
    this->box.append(this->machines);

    this->set_expand(false);
    // this->set_valign(Gtk::Align::FILL);
    // this->set_halign(Gtk::Align::FILL);

    this->set_child(this->box);
  }

  MainGame(const MainGame &) = delete;
  MainGame(MainGame &&) = delete;
};

class StartScreen final : public Gtk::Box {
public:
  Gtk::Button continue_game_;
  Gtk::Button new_game_;
  Gtk::Button exit_;

  explicit StartScreen()
      : Gtk::Box(Gtk::Orientation::VERTICAL), continue_game_("Continue"),
        new_game_("New Game"), exit_("Exit") {
    this->continue_game_.set_sensitive(false);

    this->append(this->continue_game_);
    this->append(this->new_game_);
    this->append(this->exit_);
  }

  Glib::SignalProxy<void()> signal_new_game() {
    return this->new_game_.signal_clicked();
  }

  Glib::SignalProxy<void()> signal_exit() {
    return this->exit_.signal_clicked();
  }
};

class App final : public Gtk::Window {
public:
  StartScreen start_screen_;
  std::optional<MainGame> main_game_;
  Connections conns;

  shapezx::Global global_state;

  explicit App() {
    this->conns.add(this->start_screen_.signal_new_game().connect([this]() {
      auto const &p = this->global_state.create_save();
      auto state = shapezx::State(this->global_state.max_height,
                                  this->global_state.max_width);
      auto &g =
          this->main_game_.emplace(std::move(state), this->global_state, p);
      this->conns.add(g.signal_destroy().connect([this]() {
        this->start_screen_.set_visible();
        this->main_game_.reset();
      }));

      g.present();
      this->start_screen_.set_visible(false);
    }));

    this->conns.add(this->start_screen_.signal_exit().connect(
        [this]() { this->destroy(); }));

    this->set_child(this->start_screen_);
  }
};

int main(int argc, char **argv) {
  auto app = Gtk::Application::create();

  return app->make_window_and_run<App>(argc, argv);
}