#include <lines.h>

using namespace glm;

std::vector<LineVertex> lines;

void draw_lines()
{
    GLuint vao;
    GLuint vbo;

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, lines.size() * sizeof(LineVertex), lines.data(), GL_STREAM_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(LineVertex), (void*)offsetof(LineVertex, pos));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(LineVertex), (void*)offsetof(LineVertex, color));
    glEnableVertexAttribArray(1);

    glDrawArrays(GL_LINES, 0, lines.size());

    lines.clear();
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
}

void line(vec3 start, vec3 end, vec3 color)
{
    lines.push_back(LineVertex(start, color));
    lines.push_back(LineVertex(end, color));
}