#include "../core/machine.hpp"

#include <gtkmm/button.h>
#include <gtkmm/image.h>
#include <gtkmm/listbox.h>
#include <sigc++/signal.h>

#include <string>
#include <vector>

namespace shapezx::ui {
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

class MachineSelector : public Gtk::ListBox {
protected:
  std::vector<MachineIcon> icons;
  sigc::signal<void(BuildingType)> sig_machine_selected;

public:
  explicit MachineSelector() {
    this->icons.emplace_back(BuildingType::Miner,
                             Gtk::Image("./assets/miner.png"),
                             this->sig_machine_selected);

    for (auto &icon : this->icons) {
      this->append(icon);
    }
  }

  sigc::signal<void(BuildingType)> signal_machine_selected() {
    return this->sig_machine_selected;
  }
};
} // namespace shapezx::ui