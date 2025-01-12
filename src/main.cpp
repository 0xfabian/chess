#include <iostream>
#include <core.h>
#include <app.h>

int main(int argc, char** argv)
{
    App app;

    core::init(1200, 800, "chess");
    app.init();

    while (window.is_open)
    {
        core::main_loop();
        app.update(delta_time);
        app.draw();
        core::render();
    }

    app.clean();
    core::clean();

    return 0;
}