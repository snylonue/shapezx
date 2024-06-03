#include <gtkmm.h>
#include <gtkmm/application.h>

#include "core/core.hpp"

class MyWindow : public Gtk::Window {
public:
  MyWindow() {
    set_title("Basic application");
    set_default_size(200, 200);
  }
};

int main(int argc, char **argv) {
  auto core = shapezx::Map{{shapezx::Chunk{}}, 1, 1};

  auto app = Gtk::Application::create();
  return app->make_window_and_run<MyWindow>(argc, argv);
}