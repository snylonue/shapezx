#include <gtkmm/button.h>
#include <gtkmm/image.h>
#include <gtkmm/listbox.h>

#include <string>
#include <vector>

namespace shapezx::ui {
struct MachineIcon : Gtk::Button {
  std::string name;

  explicit MachineIcon(const std::string &&name_, Gtk::Image &&icon_)
      : name(std::move(name_)), icon(std::move(icon_)) {
    this->set_child(icon);
  }

protected:
  Gtk::Image icon;
};

struct MachineSelector : Gtk::ListBox {
  explicit MachineSelector() {
    this->icons.emplace_back("miner",
                             Gtk::Image("./assets/miner.png"));

    for (auto & icon : this->icons) {
      this->append(icon);
    }
  }

protected:
  std::vector<MachineIcon> icons;
};
} // namespace shapezx::ui