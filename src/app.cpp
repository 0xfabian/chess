#include <app.h>

using namespace std;
using namespace glm;

int last_mx, last_my;
bool on_board = false;
int board_x, board_y;

vec3 light_pos = vec3(0, 10, 0);
mat4 light_mat;

Shader solid_shader;
Shader shadow_shader;

Mesh models[6];
Mesh board_model;
Board board;

GLuint shadow_fbo;
GLuint shadow_map;
int shadow_map_size = 4096;

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
}

void shadow_pass()
{
    glBindFramebuffer(GL_FRAMEBUFFER, shadow_fbo);
    glViewport(0, 0, shadow_map_size, shadow_map_size);
    glClear(GL_DEPTH_BUFFER_BIT);
    // glEnable(GL_CULL_FACE);
    // glCullFace(GL_FRONT);

    shadow_shader.bind();
    shadow_shader.upload_mat4("light_mat", light_mat);

    for (int y = 0; y < 8; y++)
    {
        for (int x = 0; x < 8; x++)
        {
            Square* square = board.getSquare(x, y);

            if (square->isEmpty())
                continue;

            Piece piece = square->getPiece();
            Mesh* model = &models[piece];

            mat4 model_mat = translate(mat4(1), vec3(x - 4 + 0.5, 0, y - 4 + 0.5));

            shadow_shader.upload_mat4("model_mat", model_mat);

            model->draw();
        }
    }
}

void lighting_pass()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, window.width, window.height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    solid_shader.bind();
    solid_shader.upload_vec3("eye", cam.get_position());
    solid_shader.upload_mat4("cam_mat", cam.get_view_matrix());
    solid_shader.upload_vec3("light", light_pos);
    solid_shader.upload_mat4("light_mat", light_mat);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, shadow_map);
    solid_shader.upload_int("shadow_map", 1);

    glDisable(GL_CULL_FACE);

    solid_shader.upload_int("reflection", 1);

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

            mat4 model_mat = translate(mat4(1), vec3(x - 4 + 0.5, 0, y - 4 + 0.5));

            solid_shader.upload_mat4("model_mat", model_mat);
            solid_shader.upload_int("white", color);

            model->draw();
        }
    }

    glClear(GL_DEPTH_BUFFER_BIT);

    solid_shader.upload_mat4("model_mat", mat4(1));
    solid_shader.upload_int("white", 2);
    board_model.draw();

    solid_shader.upload_int("reflection", 0);

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

            mat4 model_mat = translate(mat4(1), vec3(x - 4 + 0.5, 0, y - 4 + 0.5));

            solid_shader.upload_mat4("model_mat", model_mat);
            solid_shader.upload_int("white", color);

            model->draw();
        }
    }
}

void App::init()
{
    glClearColor(0.7, 0.8, 0.9, 1);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glLineWidth(3);

    SDL_GetMouseState(&last_mx, &last_my);

    solid_shader = Shader("solid");
    shadow_shader = Shader("shadow");

    models[0].load("models/king.obj");
    models[1].load("models/queen.obj");
    models[2].load("models/bishop.obj");
    models[3].load("models/knight.obj");
    models[4].load("models/rook.obj");
    models[5].load("models/pawn.obj");
    board_model.load("models/board.obj");

    shadow_mapping_init();

    mat4 perspective_mat = perspective(radians(90.0f), 1.0f, 1.0f, 100.0f);
    perspective_mat[0][0] *= -1;
    mat4 view_mat = lookAt(light_pos, vec3(0), vec3(0, 0, 1));
    light_mat = perspective_mat * view_mat;
}

bool rayPlaneIntersection(const vec3& rayOrigin, const vec3& rayDirection, const vec3& planeNormal, float planeD, vec3& hit)
{
    // Dot product of plane normal and ray direction
    float denom = dot(planeNormal, rayDirection);

    // Check if ray is parallel to the plane
    if (abs(denom) < 1e-6)      // Use a small epsilon for floating-point comparison
        return false;         // No intersection or ray lies in the plane

    // Compute t (intersection parameter)
    float t = -(dot(planeNormal, rayOrigin) + planeD) / denom;

    // If t < 0, the intersection is behind the ray origin
    if (t < 0)
        return false;

    // Compute the intersection point
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
        color = vec3(1);

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
        draw_square(board_x, board_y, vec3(1));

    shadow_pass();
    lighting_pass();
}

void App::clean()
{
    for (int i = 0; i < 6; i++)
        models[i].destroy();

    solid_shader.destroy();
    shadow_shader.destroy();

    glDeleteTextures(1, &shadow_map);
    glDeleteFramebuffers(1, &shadow_fbo);
}