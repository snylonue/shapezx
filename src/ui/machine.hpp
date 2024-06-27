#include "../core/machine.hpp"

#include <cstdint>
#include <glibmm/signalproxy.h>
#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/image.h>
#include <gtkmm/listbox.h>
#include <gtkmm/listboxrow.h>
#include <sigc++/signal.h>

#include <string>
#include <utility>
#include <vector>

namespace shapezx::ui {

struct UIState {
  std::optional<shapezx::BuildingType> machine_selected = std::nullopt;
  bool machine_removing = false;
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
  sigc::signal<void()> sig_remove_selected;

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

    this->remove.set_child(this->remove_icon);

    for (auto &icon : this->icons) {
      this->append(icon);
    }

    this->append(this->remove);

    this->set_hexpand();
    this->set_halign(Gtk::Align::CENTER);
  }

  sigc::signal<void(BuildingType)> signal_machine_selected() {
    return this->sig_machine_selected;
  }

  Glib::SignalProxy<void()> signal_remove_selected() {
    return this->remove.signal_clicked();
  }
};

class Machine final : public Gtk::Button {
  static Gtk::Image load_icon(BuildingType type) {
    switch (type) {
    case BuildingType::Miner:
      return Gtk::Image{"./assets/miner.png"};
    case BuildingType::TrashCan:
      return Gtk::Image{"./assets/trashcan.png"};
    case BuildingType::Belt:
      return Gtk::Image{"./assets/belt.png"};
    case BuildingType::Cutter:
      return Gtk::Image{"./assets/cutter_large.png"};
    default:
      return Gtk::Image{};
    }
  }

public:
  BuildingType type_;
  vec::Vec2<> pos_;
  std::uint32_t id_;
  Gtk::Image icon_;
  std::reference_wrapper<UIState> ui_state_;
  sigc::signal<void(std::uint32_t)> machine_removed;

  explicit Machine(BuildingType type, vec::Vec2<> pos, std::uint32_t id,
                   UIState &ui_state)
      : type_(type), pos_(pos), id_(id), icon_(load_icon(type)),
        ui_state_(ui_state) {
    this->icon_.set_expand();
    this->set_child(this->icon_);
  }

  void on_clicked() override {
    this->machine_removed.emit(this->id_);
  }

  sigc::signal<void(std::uint32_t)> signal_machine_removed() {
    return this->machine_removed;
  }
};
} // namespace shapezx::ui