#include <camera.h>

using namespace glm;

void Camera::set_aspect(int width, int height)
{
    aspect_ratio = width / (float)height;
}

vec3 Camera::get_position()
{
    return distance * vec3(sin(theta) * cos(phi), sin(phi), -cos(theta) * cos(phi));
}

mat4 Camera::get_view_matrix()
{
    vec3 pos = get_position();

    mat4 perspective_mat = perspective(radians(fov), aspect_ratio, near, far);
    perspective_mat[0][0] *= -1;

    mat4 view_mat = lookAt(pos, vec3(0), vec3(0, 1, 0));

    return perspective_mat * view_mat;
}

mat4 Camera::get_view_matrix_no_translation()
{
    vec3 pos = get_position();

    mat4 perspective_mat = perspective(radians(fov), aspect_ratio, near, far);
    perspective_mat[0][0] *= -1;

    mat4 view_mat = mat3(lookAt(pos, vec3(0), vec3(0, 1, 0)));

    return perspective_mat * view_mat;
}

void Camera::orbit(float dx, float dy)
{
    theta -= dx;
    phi += dy;

    if (phi < 0.1)
        phi = 0.1;

    if (phi > M_PI_2 - 0.1)
        phi = M_PI_2 - 0.1;
}

void Camera::zoom(int delta)
{
    distance -= delta / 2.0f;

    distance = clamp(distance, 2.0f, 12.0f);
}