#pragma once

#include <glm.hpp>
#include <vector>
#include <glad/glad.h>

#include "shader.h"

struct LineVertex
{
    glm::vec3 pos;
    glm::vec3 color;

    LineVertex(glm::vec3 _pos, glm::vec3 _color)
    {
        pos = _pos;
        color = _color;
    }
};

extern std::vector<LineVertex> lines;

void draw_lines();

void line(glm::vec3 start, glm::vec3 end, glm::vec3 color);