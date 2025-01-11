#pragma once

#include <mesh.h>

#include <vector>
#include <glm.hpp>
#include <glad/glad.h>
#include <string>

#include "misc.h"

struct Vertex
{
    glm::vec3 pos;
    glm::vec3 normal;

    Vertex(const glm::vec3& _pos, const glm::vec3& _norm)
    {
        pos = _pos;
        normal = _norm;
    }
};

struct Mesh
{
    GLuint vao;
    GLuint vbo;
    GLuint ebo;

    std::vector<Vertex> vertices;
    std::vector<int> indices;

    Mesh() {}
    Mesh(const std::vector<Vertex>& _vertices, const std::vector<int>& _indices);

    void load(const char* path);
    void draw();
    void build();
    void rebuffer();
    void destroy();
};