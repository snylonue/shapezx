#include "../core/machine.hpp"

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

// namespace images {
// static Gtk::Image MINER{"./assets/miner.png"};
// static Gtk::Image CUTTER{"./assets/cutter.png"};
// static Gtk::Image BELT{"./assets/belt.png"};
// static Gtk::Image TRASH{"./assets/trash.png"};
// } // namespace images

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
  sigc::signal<void(BuildingType)> sig_machine_selected;

public:
  explicit MachineSelector() {
    auto add_icon = [this](BuildingType type, Gtk::Image &&img) {
      this->icons.emplace_back(type, std::move(img),
                               this->sig_machine_selected);
    };
    add_icon(BuildingType::Miner, Gtk::Image{"./assets/miner.png"});
    add_icon(BuildingType::Cutter, Gtk::Image{"./assets/cutter.png"});
    add_icon(BuildingType::TrashCan, Gtk::Image{"./assets/trashcan.png"});
    add_icon(BuildingType::Belt, Gtk::Image{"./assets/belt.png"});

    for (auto &icon : this->icons) {
      this->append(icon);
    }

    this->set_hexpand();
    this->set_halign(Gtk::Align::CENTER);
  }

  sigc::signal<void(BuildingType)> signal_machine_selected() {
    return this->sig_machine_selected;
  }
};

class Machine final : public Gtk::Button {
public:
  shapezx::BuildingType type_;

  explicit Machine(shapezx::BuildingType type) : type_(type) {}
};
} // namespace shapezx::ui