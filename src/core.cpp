#include <core.h>

using namespace std;
using namespace glm;

Window window;
Camera cam;
double delta_time;

Uint64 this_frame;
Uint64 last_frame;

Shader line_shader;

void core::init(int width, int height, const char* name)
{
    window.width = width;
    window.height = height;
    window.is_open = true;

    SDL_Init(SDL_INIT_EVERYTHING);

    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 8);

    window.sdl_win = SDL_CreateWindow
    (
        name,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        window.width,
        window.height,
        SDL_WINDOW_OPENGL
    );

    window.gl_context = SDL_GL_CreateContext(window.sdl_win);
    SDL_GL_MakeCurrent(window.sdl_win, window.gl_context);

    gladLoadGLLoader(SDL_GL_GetProcAddress);

    cam.set_aspect(window.width, window.height);

    line_shader = Shader("line");
}

void core::clean()
{
    line_shader.destroy();

    SDL_GL_DeleteContext(window.gl_context);
    SDL_DestroyWindow(window.sdl_win);

    Mix_CloseAudio();
    SDL_Quit();
}

void handle_events()
{
    SDL_Event event;

    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_QUIT:              window.is_open = false;                                 break;
        case SDL_KEYDOWN:           set_key_down(event.key.keysym.sym);                     break;
        case SDL_KEYUP:             set_key_up(event.key.keysym.sym);                       break;
        case SDL_MOUSEBUTTONDOWN:   buttons[event.button.button] = InputState::DOWN;        break;
        case SDL_MOUSEBUTTONUP:     buttons[event.button.button] = InputState::UP;          break;
        case SDL_MOUSEWHEEL:        cam.zoom(event.wheel.y);                                break;
        }
    }
}

void core::main_loop()
{
    last_frame = this_frame;
    this_frame = SDL_GetPerformanceCounter();

    delta_time = (this_frame - last_frame) / (double)SDL_GetPerformanceFrequency();

    update_inputs();
    handle_events();
}

void core::render()
{
    line_shader.bind();
    line_shader.upload_mat4("view", cam.get_view_matrix());

    draw_lines();

    SDL_GL_SwapWindow(window.sdl_win);
}