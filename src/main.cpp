#include "core/core.hpp"
#include "vec/vec.hpp"

#include <sigc++/connection.h>
#include <gtkmm.h>
#include <gtkmm/application.h>
#include <gtkmm/button.h>
#include <gtkmm/enums.h>
#include <gtkmm/gridview.h>
#include <gtkmm/label.h>
#include <gtkmm/widget.h>

#include <cstddef>
#include <format>
#include <iostream>
#include <ostream>
#include <ranges>
#include <utility>
#include <vector>

class Chunk : public Gtk::Button {
public:
  explicit Chunk(const shapezx::Chunk& ck, shapezx::vec::Vec2<std::size_t> pos) {
    this->set_label(std::format("{} at ({} {})",
                                ck.building
                                    .transform([](auto const &b) {
                                      return std::format("{}", b->info().type);
                                    })
                                    .value_or("none"),
                                pos[0], pos[1]));
    this->set_expand();
    this->signal_clicked().connect(
        [=]() { std::println(std::cout, "clicked ({} {})", pos[0], pos[1]); });
  }
};

class Map : public Gtk::Grid {
public:
  explicit Map(const shapezx::Map &map) {
    std::cout << map.width << '\n';
    for (auto const [r, row] : map.chunks |
                                   std::views::chunk(map.width) |
                                   std::views::enumerate) {
      for (auto const [c, ck] : row | std::views::enumerate) {
        auto &chunk =
            this->chunks.emplace_back(map[r, c], shapezx::vec::Vec2<>(r, c));
        this->attach(chunk, c, r);
      }
    }
  }

protected:
  std::vector<Chunk> chunks;
};

class MainGame : public Gtk::Window {
public:
  shapezx::State state;
  Map map;

  explicit MainGame(const shapezx::State &&state)
      : state(std::move(state)), map(this->state.map) {
    
    // should be disconnected before destructing
    this->timer = Glib::signal_timeout().connect([this]() {
      this->state.update();
      return true;
    }, 20);

    this->set_title("shapezx");
    this->set_default_size(1920, 1080);

    this->set_child(this->map);
  }

  ~MainGame() {
    timer.disconnect();
  }

  bool on_close_request() override {
    return false;
  }

protected:
  sigc::connection timer;
};

int main(int argc, char **argv) {
  auto core = shapezx::Map{20, 30};
  auto state = shapezx::State(std::move(core), shapezx::Context());

  auto app = Gtk::Application::create();

  return app->make_window_and_run<MainGame>(argc, argv, std::move(state));
}