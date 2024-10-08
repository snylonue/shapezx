#ifndef SHAPEZX_UI_MACHINE
#define SHAPEZX_UI_MACHINE

#include "../core/core.hpp"
#include "../core/machine.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <format>
#include <functional>
#include <gdkmm/pixbuf.h>
#include <glibmm/refptr.h>
#include <glibmm/signalproxy.h>
#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/enums.h>
#include <gtkmm/image.h>
#include <gtkmm/label.h>
#include <gtkmm/listbox.h>
#include <gtkmm/listboxrow.h>
#include <gtkmm/overlay.h>
#include <gtkmm/widget.h>
#include <gtkmm/window.h>
#include <memory>
#include <optional>
#include <ranges>
#include <sigc++/signal.h>

#include <string>
#include <utility>
#include <vector>

namespace shapezx::ui {

struct UIState {
  std::optional<BuildingType> machine_selected = std::nullopt;
  std::optional<Direction> direction = std::nullopt;
  bool machine_removing = false;
  size_t map_locked = 0;

  void lock_map() { this->map_locked += 1; }

  void unlock_map() {
    assert(this->map_locked);
    this->map_locked -= 1;
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

class MachineIcon : public Gtk::Button {
protected:
  Gtk::Image icon;
  BuildingType type;
  sigc::signal<void(BuildingType)> sig_machine_selected;

public:
  explicit MachineIcon(BuildingType type_, Gtk::Image &&icon_,
                       sigc::signal<void(BuildingType)> sig_machine_selected_)
      : icon(std::move(icon_)), type(type_),
        sig_machine_selected(sig_machine_selected_) {
    this->set_child(icon);
  }

  void on_clicked() override { this->sig_machine_selected.emit(this->type); }
};

class MachineSelector : public Gtk::Box {
protected:
  std::vector<MachineIcon> icons;
  Gtk::Button remove;
  Gtk::Image remove_icon;
  sigc::signal<void(BuildingType)> sig_machine_selected;

  Gtk::Button save;

public:
  explicit MachineSelector() : remove_icon(Gtk::Image{"./assets/remove.png"}) {
    auto add_icon = [this](BuildingType type, Gtk::Image &&img) {
      this->icons.emplace_back(type, std::move(img),
                               this->sig_machine_selected);
    };
    add_icon(BuildingType::Miner, Gtk::Image{"./assets/miner.png"});
    add_icon(BuildingType::Cutter, Gtk::Image{"./assets/cutter.png"});
    add_icon(BuildingType::TrashCan, Gtk::Image{"./assets/trashcan.png"});
    add_icon(BuildingType::Belt, Gtk::Image{"./assets/belt.png"});
    add_icon(BuildingType::TaskCenter, Gtk::Image{"./assets/task_center.png"});

    this->remove.set_child(this->remove_icon);

    for (auto &icon : this->icons) {
      this->append(icon);
    }

    this->append(this->remove);
    this->append(this->save);

    this->set_hexpand();
    this->set_halign(Gtk::Align::CENTER);
  }

  sigc::signal<void(BuildingType)> signal_machine_selected() {
    return this->sig_machine_selected;
  }

  Glib::SignalProxy<void()> signal_remove_selected() {
    return this->remove.signal_clicked();
  }

  Glib::SignalProxy<void()> signal_save() {
    return this->save.signal_clicked();
  }
};

class TaskCenter;

class Machine : public Gtk::Button {
public:
  static std::unique_ptr<Machine>
  create(BuildingType type, vec::Vec2<> pos, Direction d, std::uint32_t id,
         UIState &ui_state, const shapezx::State &game_state,
         sigc::signal<void(std::uint32_t)> machine_removed);

  BuildingType type_;
  vec::Vec2<> pos_;
  std::uint32_t id_;
  Gtk::Image icon_;
  std::reference_wrapper<UIState> ui_state_;
  sigc::signal<void(std::uint32_t)> machine_removed_;

  explicit Machine(BuildingType type, Glib::RefPtr<Gdk::Pixbuf> icon,
                   vec::Vec2<> pos, Direction d, std::uint32_t id,
                   UIState &ui_state,
                   sigc::signal<void(std::uint32_t)> machine_removed)
      : type_(type), pos_(pos), id_(id), ui_state_(ui_state),
        machine_removed_(machine_removed) {
    switch (d) {
    case Direction::Right:
      this->icon_.set(icon->rotate_simple(Gdk::Pixbuf::Rotation::CLOCKWISE));
      break;
    case Direction::Down:
      this->icon_.set(icon->rotate_simple(Gdk::Pixbuf::Rotation::UPSIDEDOWN));
      break;
    case Direction::Left:
      this->icon_.set(
          icon->rotate_simple(Gdk::Pixbuf::Rotation::COUNTERCLOCKWISE));
      break;
    default:
      this->icon_.set(icon);
      break;
    }
    this->icon_.set_expand();
    this->set_child(this->icon_);
  }

  void on_clicked() override {
    if (this->ui_state_.get().machine_removing) {
      this->machine_removed_.emit(this->id_);
      this->ui_state_.get().machine_removing = false;
    }
  }

  sigc::signal<void(std::uint32_t)> signal_machine_removed() {
    return this->machine_removed_;
  }

  virtual void update() {}
};

class TaskSelector final : public Gtk::Window {
public:
  Gtk::Box box_;
  std::vector<Gtk::Button> butns_;
  std::reference_wrapper<const shapezx::State> game_state_;
  Connections conns;

  std::optional<std::size_t> selected;

  explicit TaskSelector(const shapezx::State &game_state)
      : box_(Gtk::Orientation::VERTICAL), game_state_(game_state) {
    this->butns_.reserve(this->game_state_.get().tasks.size());
    for (auto [i, task] :
         this->game_state_.get().tasks | std::ranges::views::enumerate) {
      std::string label = std::format("{} - ", i);
      for (auto [item, num] : task.target_.items) {
        label += std::format("{}: {}", item.name, num);
      }
      auto &butn = this->butns_.emplace_back(label);
      conns.add(butn.signal_clicked().connect([this, i]() {
        if (this->selected != i) {
          this->selected = i;
        } else {
          this->selected = std::nullopt;
        }
        this->set_visible(false);
      }));
    }

    for (auto &butn : this->butns_) {
      this->box_.append(butn);
    }

    this->set_child(this->box_);
  }
};

class TaskCenter final : public Machine {
public:
  TaskSelector setting_;
  Connections conns_;
  Gtk::Label info_;

  std::reference_wrapper<const shapezx::State> game_state_;

  explicit TaskCenter(Glib::RefPtr<Gdk::Pixbuf> icon, vec::Vec2<> pos,
                      Direction d, std::uint32_t id, UIState &ui_state,
                      const shapezx::State &game_state,
                      sigc::signal<void(std::uint32_t)> machine_removed)
      : Machine(BuildingType::TaskCenter, icon, pos, d, id, ui_state,
                machine_removed),
        setting_(game_state), game_state_(game_state) {
    this->conns_.add(this->setting_.signal_show().connect(
        [this]() { this->ui_state_.get().lock_map(); }));
    this->conns_.add(this->setting_.signal_destroy().connect(
        [this]() { this->ui_state_.get().unlock_map(); }));
  }

  void on_clicked() override {
    if (this->ui_state_.get().machine_removing) {
      this->machine_removed_.emit(this->id_);
      this->ui_state_.get().machine_removing = false;
    } else {
      this->setting_.set_visible();
    }
  }

  void update() override {
    if (this->setting_.selected) {
      auto selected = *this->setting_.selected;
      std::string s;
      auto &task = this->game_state_.get().tasks[selected];
      if (task.completed_) {
        s = "Completed";
      } else {
        auto &store = this->game_state_.get().store;
        for (auto &[item, num] : task.target_.items) {
          s += std::format("{}: {} / {}\n", item.name, store.get(item), num);
        }
      }
      this->info_.set_text(s);
      this->unset_child();
      this->set_child(this->info_);
    } else {
      this->unset_child();
      this->set_child(this->icon_);
    }
  }
};

} // namespace shapezx::ui

#endif