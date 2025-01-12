#include <shader.h>

GLuint create_shader(const std::string& path, GLenum type)
{
    char* data = read_file(path.c_str());

    if (!data)
    {
        std::cerr << "Failed to read " << path << std::endl;
        exit(1);
    }

    GLuint id = glCreateShader(type);
    glShaderSource(id, 1, &data, 0);
    free(data);

    GLint compiled;
    glCompileShader(id);
    glGetShaderiv(id, GL_COMPILE_STATUS, &compiled);

    if (compiled != GL_TRUE)
    {
        std::cerr << "Failed to compile " << path << std::endl;

        GLint log_length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &log_length);

        char* log = (char*)malloc(log_length);
        glGetShaderInfoLog(id, log_length, 0, log);

        std::cerr << log << std::endl;

        free(log);

        exit(2);
    }

    return id;
}

Shader::Shader(const std::string& name)
{
    std::string vert_path = "shaders/" + name + ".vert";
    std::string frag_path = "shaders/" + name + ".frag";

    GLuint vert_id = create_shader(vert_path, GL_VERTEX_SHADER);
    GLuint frag_id = create_shader(frag_path, GL_FRAGMENT_SHADER);

    id = glCreateProgram();

    glAttachShader(id, vert_id);
    glAttachShader(id, frag_id);
    glLinkProgram(id);

    glDeleteShader(vert_id);
    glDeleteShader(frag_id);
}

void Shader::bind()
{
    glUseProgram(id);
}

void Shader::upload_float(const char* var_name, float value)
{
    GLuint loc = glGetUniformLocation(id, var_name);
    glUniform1f(loc, value);
}

void Shader::upload_int(const char* var_name, int value)
{
    GLuint loc = glGetUniformLocation(id, var_name);
    glUniform1i(loc, value);
}

void Shader::upload_vec3(const char* var_name, const glm::vec3& vec)
{
    GLuint loc = glGetUniformLocation(id, var_name);
    glUniform3f(loc, vec.x, vec.y, vec.z);
}

void Shader::upload_vec4(const char* var_name, const glm::vec4& vec)
{
    GLuint loc = glGetUniformLocation(id, var_name);
    glUniform4f(loc, vec.x, vec.y, vec.z, vec.w);
}

void Shader::upload_mat4(const char* var_name, const glm::mat4& matrix)
{
    GLuint loc = glGetUniformLocation(id, var_name);
    glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(matrix));
}

void Shader::destroy()
{
    glDeleteProgram(id);
}