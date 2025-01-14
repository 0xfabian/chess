#include <app.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace std;
using namespace glm;

int last_mx, last_my;
bool on_board = false;
int board_x, board_y;

vec3 light_pos = vec3(0, 10, 0);
mat4 light_mat;

Shader solid_shader;
Shader shadow_shader;
Shader skybox_shader;
Shader to_cubemap_shader;
Shader conv_shader;
Shader prefilter_shader;
Shader brdf_lut_shader;

Mesh models[6];
Mesh board_model;
Mesh skybox_model;
Board board;

GLuint white_albedo;
GLuint black_albedo;
GLuint board_albedo;

GLuint white_normal;
GLuint black_normal;
GLuint board_normal;

GLuint white_arm;
GLuint black_arm;
GLuint board_arm;

GLuint shadow_fbo;
GLuint shadow_map;
GLuint offset_texture;
int shadow_map_size = 2048;
int window_size = 16;
int filter_size = 8;

GLuint capture_fbo;
GLuint env_map;
GLuint irradiance_map;
GLuint prefilter_map;
GLuint brdf_lut;

GLuint reflection_texture;
GLuint reflection_rbo;

GLuint load_texture(const char* path, bool srgb = false)
{
    int width, height, channels;
    unsigned char* data = stbi_load(path, &width, &height, &channels, 0);

    GLuint format = srgb ? GL_SRGB : GL_RGB;

    if (channels == 4)
        format = srgb ? GL_SRGB_ALPHA : GL_RGBA;

    if (data)
    {
        GLuint id;
        glGenTextures(1, &id);
        glBindTexture(GL_TEXTURE_2D, id);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, channels == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, data);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(data);

        cout << "Loaded texture " << path << endl;
        return id;
    }
    else
    {
        cout << "Failed to load texture " << path << endl;
        return 0;
    }
}

void load_textures()
{
    white_albedo = load_texture("assets/textures/white_albedo.png", true);
    black_albedo = load_texture("assets/textures/black_albedo.png", true);
    board_albedo = load_texture("assets/textures/board_albedo.png", true);

    white_normal = load_texture("assets/textures/white_normal.png");
    black_normal = load_texture("assets/textures/black_normal.png");
    board_normal = load_texture("assets/textures/board_normal.png");

    white_arm = load_texture("assets/textures/white_arm.png");
    black_arm = load_texture("assets/textures/black_arm.png");
    board_arm = load_texture("assets/textures/board_arm.png");
}

void load_environment()
{
    cout << "Loading environment ..." << endl;

    stbi_set_flip_vertically_on_load(true);

    GLuint hdr_texture;
    int widht, height, channels;
    float* data = stbi_loadf("assets/env/env.hdr", &widht, &height, &channels, 0);

    if (data)
    {
        glGenTextures(1, &hdr_texture);
        glBindTexture(GL_TEXTURE_2D, hdr_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, widht, height, 0, GL_RGB, GL_FLOAT, data);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        stbi_image_free(data);
    }

    cout << "Done" << endl;
    cout << "Generating environment cubemap ..." << endl;

    glGenFramebuffers(1, &capture_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, capture_fbo);

    glGenTextures(1, &env_map);
    glBindTexture(GL_TEXTURE_CUBE_MAP, env_map);

    for (int i = 0; i < 6; i++)
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 512, 512, 0, GL_RGB, GL_FLOAT, nullptr);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    mat4 capture_projection = perspective(radians(90.0f), 1.0f, 0.1f, 10.0f);
    mat4 capture_views[] =
    {
        lookAt(vec3(0), vec3(1, 0, 0), vec3(0, -1, 0)),
        lookAt(vec3(0), vec3(-1, 0, 0), vec3(0, -1, 0)),
        lookAt(vec3(0), vec3(0, 1, 0), vec3(0, 0, 1)),
        lookAt(vec3(0), vec3(0, -1, 0), vec3(0, 0, -1)),
        lookAt(vec3(0), vec3(0, 0, 1), vec3(0, -1, 0)),
        lookAt(vec3(0), vec3(0, 0, -1), vec3(0, -1, 0))
    };

    to_cubemap_shader.bind();
    to_cubemap_shader.upload_mat4("projection", capture_projection);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, hdr_texture);
    to_cubemap_shader.upload_int("env", 0);

    glViewport(0, 0, 512, 512);
    glBindFramebuffer(GL_FRAMEBUFFER, capture_fbo);

    for (int i = 0; i < 6; i++)
    {
        to_cubemap_shader.upload_mat4("view", capture_views[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, env_map, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        skybox_model.draw();
    }

    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    cout << "Done" << endl;
    cout << "Generating irradiance map ..." << endl;

    glGenTextures(1, &irradiance_map);
    glBindTexture(GL_TEXTURE_CUBE_MAP, irradiance_map);

    for (int i = 0; i < 6; i++)
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 32, 32, 0, GL_RGB, GL_FLOAT, nullptr);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    conv_shader.bind();
    conv_shader.upload_mat4("projection", capture_projection);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, env_map);
    conv_shader.upload_int("env", 0);

    glViewport(0, 0, 32, 32);
    glBindFramebuffer(GL_FRAMEBUFFER, capture_fbo);

    for (int i = 0; i < 6; i++)
    {
        conv_shader.upload_mat4("view", capture_views[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradiance_map, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        skybox_model.draw();
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    cout << "Done" << endl;
    cout << "Generating prefilter map ..." << endl;

    glGenTextures(1, &prefilter_map);
    glBindTexture(GL_TEXTURE_CUBE_MAP, prefilter_map);

    for (int i = 0; i < 6; i++)
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 128, 128, 0, GL_RGB, GL_FLOAT, nullptr);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    prefilter_shader.bind();
    prefilter_shader.upload_mat4("projection", capture_projection);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, env_map);
    prefilter_shader.upload_int("env", 0);

    glBindFramebuffer(GL_FRAMEBUFFER, capture_fbo);

    for (int mip = 0; mip < 5; mip++)
    {
        unsigned int mip_size = 128 * pow(0.5, mip);

        glViewport(0, 0, mip_size, mip_size);

        float roughness = (float)mip / 4.0;
        prefilter_shader.upload_float("roughness", roughness);

        for (int i = 0; i < 6; i++)
        {
            prefilter_shader.upload_mat4("view", capture_views[i]);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, prefilter_map, mip);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            skybox_model.draw();
        }
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    cout << "Done" << endl;
    cout << "Generating BRDF LUT ..." << endl;

    glGenTextures(1, &brdf_lut);
    glBindTexture(GL_TEXTURE_2D, brdf_lut);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, 512, 512, 0, GL_RG, GL_FLOAT, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindFramebuffer(GL_FRAMEBUFFER, capture_fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brdf_lut, 0);

    glViewport(0, 0, 512, 512);

    brdf_lut_shader.bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    vector<Vertex> vertices =
    {
        Vertex(vec3(-1.f, -1.f, 0.f)),
        Vertex(vec3(1.f, -1.f, 0.f)),
        Vertex(vec3(1.f, 1.f, 0.f)),
        Vertex(vec3(-1.f, 1.f, 0.f))
    };

    vector<int> indices = { 0, 1, 2, 0, 2, 3 };

    Mesh quad(vertices, indices);
    quad.draw();
    quad.destroy();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    cout << "Done" << endl;
}

void create_reflection_texture()
{
    glGenTextures(1, &reflection_texture);
    glBindTexture(GL_TEXTURE_2D, reflection_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, window.width / 2, window.height / 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glGenRenderbuffers(1, &reflection_rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, reflection_rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, window.width / 2, window.height / 2);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

vector<float> generate_offset_data()
{
    size_t buffer_size = window_size * window_size * filter_size * filter_size * 2;
    vector<float> data(buffer_size);

    int index = 0;

    for (int i = 0; i < window_size * window_size; i++)
    {
        for (int fy = filter_size - 1; fy >= 0; fy--)
        {
            for (int fx = 0; fx < filter_size; fx++)
            {
                float x = ((float)fx + 0.5 + randf(-0.5, 0.5)) / (float)filter_size;
                float y = ((float)fy + 0.5 + randf(-0.5, 0.5)) / (float)filter_size;

                data[index] = sqrtf(y) * cosf(2 * M_PI * x);
                data[index + 1] = sqrtf(y) * sinf(2 * M_PI * x);

                index += 2;
            }
        }
    }

    return data;
}

void create_offset_texture()
{
    vector<float> data = generate_offset_data();

    int samples = filter_size * filter_size;

    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &offset_texture);
    glBindTexture(GL_TEXTURE_3D, offset_texture);
    glTexStorage3D(GL_TEXTURE_3D, 1, GL_RGBA32F, samples / 2, window_size, window_size);
    glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, samples / 2, window_size, window_size, GL_RGBA, GL_FLOAT, data.data());
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_3D, 0);
}

void shadow_mapping_init()
{
    glGenFramebuffers(1, &shadow_fbo);

    glGenTextures(1, &shadow_map);
    glBindTexture(GL_TEXTURE_2D, shadow_map);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadow_map_size, shadow_map_size, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    GLfloat border_color[] = { 1.0, 1.0, 1.0, 1.0 };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border_color);

    glBindFramebuffer(GL_FRAMEBUFFER, shadow_fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadow_map, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    create_offset_texture();
}

void render_skybox()
{
    glDepthMask(GL_FALSE);

    skybox_shader.bind();
    skybox_shader.upload_mat4("cam_mat", cam.get_view_matrix_no_translation());

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, env_map);
    skybox_shader.upload_int("skybox", 0);

    skybox_model.draw();

    glDepthMask(GL_TRUE);
}

void render_board()
{
    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_2D, board_albedo);
    solid_shader.upload_int("albedo_texture", 6);

    glActiveTexture(GL_TEXTURE7);
    glBindTexture(GL_TEXTURE_2D, board_arm);
    solid_shader.upload_int("arm_texture", 7);

    glActiveTexture(GL_TEXTURE8);
    glBindTexture(GL_TEXTURE_2D, board_normal);
    solid_shader.upload_int("normal_map", 8);

    solid_shader.upload_mat4("model_mat", mat4(1));
    solid_shader.upload_int("white", 2);
    board_model.draw();
}

enum RenderContext
{
    Normal,
    ShadowPass,
};

void render_pieces(RenderContext context = RenderContext::Normal)
{
    for (int y = 0; y < 8; y++)
    {
        for (int x = 0; x < 8; x++)
        {
            Square* square = board.getSquare(x, y);

            if (square->isEmpty())
                continue;

            Piece piece = square->getPiece();
            Color color = square->getColor();
            Mesh* model = &models[piece];

            float up = 0;

            if (board.getSelected() == square)
                up = 0.3;

            mat4 model_mat = translate(mat4(1), vec3(x - 4 + 0.5, up, y - 4 + 0.5));

            if (context == RenderContext::ShadowPass)
                shadow_shader.upload_mat4("model_mat", model_mat);
            else
            {
                glActiveTexture(GL_TEXTURE6);
                glBindTexture(GL_TEXTURE_2D, color == WHITE ? white_albedo : black_albedo);
                solid_shader.upload_int("albedo_texture", 6);

                glActiveTexture(GL_TEXTURE7);
                glBindTexture(GL_TEXTURE_2D, color == WHITE ? white_arm : black_arm);
                solid_shader.upload_int("arm_texture", 7);

                glActiveTexture(GL_TEXTURE8);
                glBindTexture(GL_TEXTURE_2D, color == WHITE ? white_normal : black_normal);
                solid_shader.upload_int("normal_map", 8);

                solid_shader.upload_mat4("model_mat", model_mat);
                solid_shader.upload_int("white", color);
            }

            model->draw();
        }
    }
}

void shadow_pass()
{
    glBindFramebuffer(GL_FRAMEBUFFER, shadow_fbo);
    glViewport(0, 0, shadow_map_size, shadow_map_size);
    glClear(GL_DEPTH_BUFFER_BIT);

    shadow_shader.bind();
    shadow_shader.upload_mat4("light_mat", light_mat);

    render_pieces(RenderContext::ShadowPass);
}

void lighting_pass()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, window.width, window.height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    render_skybox();

    solid_shader.bind();
    solid_shader.upload_vec3("eye", cam.get_position());
    solid_shader.upload_mat4("cam_mat", cam.get_view_matrix());
    solid_shader.upload_vec3("light", light_pos);
    solid_shader.upload_mat4("light_mat", light_mat);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, shadow_map);
    solid_shader.upload_int("shadow_map", 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_3D, offset_texture);
    solid_shader.upload_int("offset_texture", 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_CUBE_MAP, irradiance_map);
    solid_shader.upload_int("irradiance_map", 2);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_CUBE_MAP, prefilter_map);
    solid_shader.upload_int("prefilter_map", 3);

    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, brdf_lut);
    solid_shader.upload_int("brdf_lut", 4);

    solid_shader.upload_int("reflection", 1);

    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, reflection_texture);

    glBindFramebuffer(GL_FRAMEBUFFER, capture_fbo);
    glBindRenderbuffer(GL_RENDERBUFFER, reflection_rbo);

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, reflection_rbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, reflection_texture, 0);

    glViewport(0, 0, window.width / 2, window.height / 2);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0, 0, 0, 0);
    render_pieces();

    glViewport(0, 0, window.width, window.height);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    solid_shader.upload_int("reflection_texture", 5);
    solid_shader.upload_int("reflection", 0);

    render_board();

    render_pieces();
}

void App::init()
{
    glClearColor(0, 0, 0, 1);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    glLineWidth(3);

    SDL_GetMouseState(&last_mx, &last_my);

    solid_shader = Shader("solid");
    shadow_shader = Shader("shadow");
    skybox_shader = Shader("skybox");
    to_cubemap_shader = Shader("to_cubemap");
    conv_shader = Shader("conv");
    prefilter_shader = Shader("prefilter");
    brdf_lut_shader = Shader("brdf_lut");

    skybox_model.load("assets/env/skybox.obj");
    board_model.load("assets/models/board.obj");

    const char* piece_names[] = { "king", "queen", "bishop", "knight", "rook", "pawn" };

    for (int i = 0; i < 6; i++)
    {
        string path = "assets/models/" + string(piece_names[i]) + ".obj";
        models[i].load(path.c_str());
    }

    load_textures();

    load_environment();
    create_reflection_texture();

    shadow_mapping_init();

    mat4 perspective_mat = perspective(radians(90.0f), 1.0f, 1.0f, 100.0f);
    perspective_mat[0][0] *= -1;
    mat4 view_mat = lookAt(light_pos, vec3(0), vec3(0, 0, 1));
    light_mat = perspective_mat * view_mat;
}

bool rayPlaneIntersection(const vec3& rayOrigin, const vec3& rayDirection, const vec3& planeNormal, float planeD, vec3& hit)
{
    float denom = dot(planeNormal, rayDirection);

    if (abs(denom) < 1e-6)
        return false;

    float t = -(dot(planeNormal, rayOrigin) + planeD) / denom;

    if (t < 0)
        return false;

    vec3 intersectionPoint = rayOrigin + t * rayDirection;

    hit = intersectionPoint;

    return true;
}

void App::update(float dt)
{
    int mx, my;

    SDL_GetMouseState(&mx, &my);

    float sens = 0.003;

    float dx = sens * (mx - last_mx);
    float dy = sens * (my - last_my);

    if (is_button_pressed(SDL_BUTTON_RIGHT))
        cam.orbit(dx, dy);

    float x = (2.0f * mx / window.width) - 1.0f;
    float y = 1.0f - (2.0f * my / window.height);

    vec4 near_hom = inverse(cam.get_view_matrix()) * vec4(x, y, -1.0f, 1.0f);
    vec3 near = vec3(near_hom) / near_hom.w;

    vec4 far_hom = inverse(cam.get_view_matrix()) * vec4(x, y, 1.0f, 1.0f);
    vec3 far = vec3(far_hom) / far_hom.w;;

    vec3 origin = near;
    vec3 dir = normalize(far - near);
    vec3 hit;

    on_board = false;

    if (rayPlaneIntersection(origin, dir, vec3(0, 1, 0), 0, hit))
    {
        if (hit.x > -4 && hit.x < 4 && hit.z > -4 && hit.z < 4)
        {
            on_board = true;
            board_x = hit.x + 4;
            board_y = hit.z + 4;
        }
    }

    last_mx = mx;
    last_my = my;

    if (is_button_down(SDL_BUTTON_LEFT))
    {
        if (on_board)
            board.click(board_x, board_y);
        else
            board.clearSelected();
    }
}

void draw_square(int x, int y, vec3 color, float scale = 0.8)
{
    vec3 off = vec3(x - 4 + (1 - scale) / 2.0, 0.1, y - 4 + (1 - scale) / 2.0);

    if (x == board_x && y == board_y)
        color = vec3(1, 0, 0);

    line(vec3(0, 0, 0) + off, vec3(scale, 0, 0) + off, color);
    line(vec3(scale, 0, 0) + off, vec3(scale, 0, scale) + off, color);
    line(vec3(scale, 0, scale) + off, vec3(0, 0, scale) + off, color);
    line(vec3(0, 0, scale) + off, vec3(0, 0, 0) + off, color);
}

void App::draw()
{
    Square* selected = board.getSelected();

    if (selected != nullptr)
    {
        draw_square(selected->getX(), selected->getY(), vec3(1, 1, 0));

        vector<Square> moves = board.getValidMoves();

        for (auto& move : moves)
            draw_square(move.getX(), move.getY(), vec3(0, 1, 1));
    }

    if (on_board)
        draw_square(board_x, board_y, vec3(1, 0, 0));

    shadow_pass();
    lighting_pass();
}

void App::clean()
{
    for (int i = 0; i < 6; i++)
        models[i].destroy();

    board_model.destroy();

    solid_shader.destroy();
    shadow_shader.destroy();
    skybox_shader.destroy();
    to_cubemap_shader.destroy();
    conv_shader.destroy();
    prefilter_shader.destroy();
    brdf_lut_shader.destroy();

    glDeleteTextures(1, &shadow_map);
    glDeleteTextures(1, &offset_texture);
    glDeleteFramebuffers(1, &shadow_fbo);

    glDeleteTextures(1, &env_map);
    glDeleteTextures(1, &irradiance_map);
    glDeleteTextures(1, &prefilter_map);
    glDeleteTextures(1, &brdf_lut);
    glDeleteFramebuffers(1, &capture_fbo);

    glDeleteTextures(1, &reflection_texture);
    glDeleteRenderbuffers(1, &reflection_rbo);

    glDeleteTextures(1, &white_albedo);
    glDeleteTextures(1, &black_albedo);
    glDeleteTextures(1, &board_albedo);

    glDeleteTextures(1, &white_normal);
    glDeleteTextures(1, &black_normal);
    glDeleteTextures(1, &board_normal);

    glDeleteTextures(1, &white_arm);
    glDeleteTextures(1, &black_arm);
    glDeleteTextures(1, &board_arm);
}