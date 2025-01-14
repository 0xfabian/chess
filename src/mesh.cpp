#include <mesh.h>
#include <cstring>

using namespace glm;

Mesh::Mesh(const std::vector<Vertex>& _vertices, const std::vector<int>& _indices)
{
    vertices = _vertices;
    indices = _indices;

    build();
}

void Mesh::load(const char* path)
{
    using namespace std;

    fstream file(path);

    if (!file.is_open())
    {
        cout << "Failed to load model " << path << endl;
        return;
    }

    vector<vec3> temp_vertices;
    vector<vec2> temp_uvs;
    vector<vec3> temp_normals;

    unordered_map<string, int> vertex_map;

    string line;
    while (getline(file, line))
    {
        istringstream iss(line);
        string prefix;
        iss >> prefix;

        if (prefix == "v")
        {
            vec3 v;
            iss >> v.x >> v.y >> v.z;
            temp_vertices.push_back(v);
        }
        else if (prefix == "vt")
        {
            vec2 uv;
            iss >> uv.x >> uv.y;
            uv.y = -uv.y;
            temp_uvs.push_back(uv);
        }
        else if (prefix == "vn")
        {
            vec3 n;
            iss >> n.x >> n.y >> n.z;
            temp_normals.push_back(n);
        }
        else if (prefix == "f")
        {
            int v[3], t[3], n[3];
            char slash;

            for (int i = 0; i < 3; i++)
            {
                iss >> v[i] >> slash >> t[i] >> slash >> n[i];

                v[i]--;
                t[i]--;
                n[i]--;
            }

            for (int i = 0; i < 3; i++)
            {
                string key = to_string(v[i]) + "/" + to_string(t[i]) + "/" + to_string(n[i]);

                if (vertex_map.find(key) == vertex_map.end())
                {
                    vertex_map[key] = vertex_map.size();
                    vertices.push_back(Vertex(temp_vertices[v[i]], temp_normals[n[i]], temp_uvs[t[i]]));
                }

                indices.push_back(vertex_map[key]);
            }
        }
    }

    compute_tangets();

    build();

    std::cout << "Loaded model " << path << std::endl;
}

void Mesh::compute_tangets()
{
    for (int i = 0; i < indices.size(); i += 3)
    {
        Vertex& v0 = vertices[indices[i]];
        Vertex& v1 = vertices[indices[i + 1]];
        Vertex& v2 = vertices[indices[i + 2]];

        vec3 edge1 = v1.pos - v0.pos;
        vec3 edge2 = v2.pos - v0.pos;

        vec2 deltaUV1 = v1.uv - v0.uv;
        vec2 deltaUV2 = v2.uv - v0.uv;

        float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

        vec3 tangent;
        tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

        tangent = normalize(tangent);

        v0.tangent += tangent;
        v1.tangent += tangent;
        v2.tangent += tangent;
    }

    for (auto& v : vertices)
        v.tangent = normalize(v.tangent);
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

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
    glEnableVertexAttribArray(2);

    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tangent));
    glEnableVertexAttribArray(3);
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