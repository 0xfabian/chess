#pragma once

#include <SDL.h>
#include <glm.hpp>
#include <ext.hpp>

struct Camera
{
    float phi = M_PI_4;
    float theta = 0;
    float distance = 8.0;

    float fov = 80;
    float aspect_ratio;

    float near = 0.01;
    float far = 1000.0;

    void set_aspect(int width, int height);

    glm::vec3 get_position();
    glm::mat4 get_view_matrix();

    void orbit(float dx, float dy);
    void zoom(int delta);
};