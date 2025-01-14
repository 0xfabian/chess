#pragma once

#include <mesh.h>

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <glm.hpp>
#include <glad/glad.h>

struct Vertex
{
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec2 uv;
    glm::vec3 tangent;

    Vertex(const glm::vec3& _pos)
    {
        pos = _pos;
        normal = glm::vec3(0);
        uv = glm::vec2(0);
        tangent = glm::vec3(0);
    }

    Vertex(const glm::vec3& _pos, const glm::vec3& _norm, const glm::vec2& _uv)
    {
        pos = _pos;
        normal = _norm;
        uv = _uv;
        tangent = glm::vec3(0);
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
    void compute_tangets();
    void draw();
    void build();
    void rebuffer();
    void destroy();
};