#include "app.h"
 
#include "core/core.hpp"

int main(int argc, char **argv)
{
    auto core = shapezx::Map{{shapezx::Chunk{}}, 1, 1};

    auto hello_world = HelloWorld::create();
    hello_world->set_my_label("Hello from C++");
    // Show the window and spin the event loop until the window is closed.
    hello_world->run();
    return 0;
}