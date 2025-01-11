#include <mesh.h>

using namespace glm;

Mesh::Mesh(const std::vector<Vertex>& _vertices, const std::vector<int>& _indices)
{
    vertices = _vertices;
    indices = _indices;

    build();
}

void Mesh::load(const char* path)
{
    char* data = read_file(path);

    if (!data)
    {
        printf("Failed to load mesh data\n");
        return;
    }

    int i = 0;
    std::string line;

    std::vector<vec3> normals;

    while (data[i])
    {
        if (data[i] == '\n')
        {
            if (line[0] == 'v' && line[1] == ' ')
            {
                float x, y, z;

                sscanf(line.c_str(), "v %f %f %f", &x, &y, &z);

                vertices.push_back(Vertex(vec3(x, y, z), vec3(0, 0, 0)));
            }
            else if (line[0] == 'v' && line[1] == 'n')
            {
                float x, y, z;

                sscanf(line.c_str(), "vn %f %f %f", &x, &y, &z);

                normals.push_back(vec3(x, y, z));
            }
            else if (line[0] == 'f')
            {
                int v1, v2, v3;
                int n1, n2, n3;

                sscanf(line.c_str(), "f %d//%d %d//%d %d//%d", &v1, &n1, &v2, &n2, &v3, &n3);

                vertices[v1 - 1].normal = normals[n1 - 1];
                vertices[v2 - 1].normal = normals[n2 - 1];
                vertices[v3 - 1].normal = normals[n3 - 1];

                indices.push_back(v1 - 1);
                indices.push_back(v3 - 1);
                indices.push_back(v2 - 1);
            }

            line.clear();
        }
        else
            line.push_back(data[i]);

        i++;
    }

    free(data);

    build();
}

void Mesh::draw()
{
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
}

void Mesh::build()
{
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STREAM_DRAW);

    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(int), indices.data(), GL_STREAM_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, pos));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(1);
}

void Mesh::rebuffer()
{
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(Vertex), vertices.data());
}

void Mesh::destroy()
{
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);
    glDeleteVertexArrays(1, &vao);
}