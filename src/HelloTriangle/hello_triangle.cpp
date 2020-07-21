#include <SDL.h>
#include <glad/glad.h>
#include <spdlog/spdlog.h>

#include <cstdlib>
#include <memory>
#include <string>

auto sdl_error(std::string const& msg) -> void
{
    spdlog::error("[SDL2] <<{}>>: {}!", msg, SDL_GetError());
    std::exit(EXIT_FAILURE);
}

struct color
{
    GLfloat r = 0.0F;
    GLfloat g = 0.0F;
    GLfloat b = 0.0F;
    GLfloat a = 1.0F;
};

auto main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[]) noexcept -> int
{
    spdlog::info("Hello triangle!");

    auto sdl_window_deleter = [](SDL_Window* w) noexcept {
        SDL_DestroyWindow(w);
        SDL_Quit();
    };
    auto sdl_renderer_deleter = [](SDL_Renderer* r) noexcept { SDL_DestroyRenderer(r); };

    using window_t = std::unique_ptr<SDL_Window, decltype(sdl_window_deleter)>;
    using renderer_t = std::unique_ptr<SDL_Renderer, decltype(sdl_renderer_deleter)>;

    constexpr int window_width = 1280;
    constexpr int window_height = 720;

    if(SDL_Init(SDL_INIT_VIDEO) != 0) {
        sdl_error("Couldn't initialize SDL");
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    window_t window{ SDL_CreateWindow("HelloTriangle!",
                                      SDL_WINDOWPOS_CENTERED,
                                      SDL_WINDOWPOS_CENTERED,
                                      window_width,
                                      window_height,
                                      SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE),
                     sdl_window_deleter };

    if(window == nullptr) {
        sdl_error("Couldn't create a window");
    }

    renderer_t renderer{ SDL_CreateRenderer(window.get(), -1, SDL_RENDERER_ACCELERATED), sdl_renderer_deleter };

    if(renderer == nullptr) {
        sdl_error("Couldn't create a renderer");
    }

    SDL_GLContext gl_context = SDL_GL_CreateContext(window.get());

    if(gladLoadGLLoader(static_cast<GLADloadproc>(SDL_GL_GetProcAddress)) == 0) {
        spdlog::error("[glad] Failed to initialize OpenGL context");
        std::exit(EXIT_FAILURE);
    }

    spdlog::info("[OpenGL] Context created! Version {}.{}", GLVersion.major, GLVersion.minor);

    bool window_should_close = false;
    constexpr color clear_color{ 0.25F, 0.45F, 0.15F, 1.0F };

    while(!window_should_close) {
        SDL_Event e;
        while(SDL_PollEvent(&e) != 0) {
            switch(e.type) {
            case SDL_QUIT: {
                window_should_close = true;
                break;
            }
            case SDL_KEYDOWN: {
                if(e.key.keysym.sym == SDLK_ESCAPE) {
                    window_should_close = true;
                }
                break;
            }
            default: {
                break;
            }
            }
        }

        glClearColor(clear_color.r, clear_color.g, clear_color.b, clear_color.a);
        glClear(GL_COLOR_BUFFER_BIT);

        SDL_GL_SwapWindow(window.get());
    }

    SDL_GL_DeleteContext(gl_context);
}
