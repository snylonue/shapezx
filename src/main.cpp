#include "core/core.hpp"
#include "vec/vec.hpp"

#include <cstddef>
#include <gtkmm.h>
#include <gtkmm/application.h>
#include <gtkmm/button.h>
#include <gtkmm/enums.h>
#include <gtkmm/gridview.h>
#include <gtkmm/label.h>
#include <gtkmm/textview.h>
#include <gtkmm/widget.h>

#include <format>
#include <iostream>
#include <ostream>
#include <ranges>
#include <utility>
#include <vector>

class Chunk : public Gtk::Button {
public:
  explicit Chunk(shapezx::Chunk ck, shapezx::vec::Vec2<std::size_t> pos) {
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
  shapezx::Map map;

  explicit Map(const shapezx::Map &&m) : map(std::move(m)) {
    for (auto const [r, row] : this->map.chunks |
                                   std::views::chunk(this->map.width) |
                                   std::views::enumerate) {
      for (auto const [c, ck] : row | std::views::enumerate) {
        auto &chunk = this->chunks.emplace_back(
            map[r, c], shapezx::vec::Vec2<std::size_t>(r, c));
        this->attach(chunk, c, r);
      }
    }
  }

protected:
  std::vector<Chunk> chunks;
};

class MyWindow : public Gtk::Window {
public:
  Map map;

  explicit MyWindow(const shapezx::Map &&m) : map(std::move(m)) {
    this->set_title("shapezx");
    this->set_default_size(1920, 1080);

    this->set_child(this->map);
  }

  ~MyWindow() = default;
};

int main(int argc, char **argv) {
  auto core = shapezx::Map{2, 2};

  auto app = Gtk::Application::create();
  return app->make_window_and_run<MyWindow>(argc, argv, std::move(core));
}