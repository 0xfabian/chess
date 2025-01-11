#pragma once

#include <core.h>
#include <mesh.h>
#include <chess.h>

struct App
{
    void init();
    void update(float dt);
    void draw();
    void clean();
};