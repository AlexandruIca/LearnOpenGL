#include <SDL.h>
#include <glad/glad.h>
#include <spdlog/spdlog.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <cmath>
#include <cstdlib>
#include <memory>
#include <string>
#include <vector>

#include "util/shader.hpp"

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

    int num_attributes = 0;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &num_attributes);
    spdlog::info("[OpenGL] Max number of vertex attributes: {}", num_attributes);

    std::vector<GLfloat> const vertices = {
        // positions        // colors         // texture coords
        0.5F,  0.5F,  0.0F, 1.0F, 0.0F, 0.0F, 1.0F, 1.0F, // top right
        0.5F,  -0.5F, 0.0F, 0.0F, 1.0F, 0.0F, 1.0F, 0.0F, // bottom right
        -0.5F, -0.5F, 0.0F, 0.0F, 0.0F, 1.0F, 0.0F, 0.0F, // bottom left
        -0.5F, 0.5F,  0.0F, 1.0F, 1.0F, 0.0F, 0.0F, 1.0F  // top left
    };

    std::vector<unsigned int> indices = { 0, 1, 3, 1, 2, 3 };

    unsigned int vao = 0;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    unsigned int vbo = 0;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), nullptr); // NOLINT
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat))); // NOLINT
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat))); // NOLINT
    glEnableVertexAttribArray(2);

    shader shader_program{ "shader.vs", "shader.fs" };

    unsigned int ibo = 0;
    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    int tex_width = 0;
    int tex_height = 0;
    int tex_num_channels = 0;

    unsigned char* data = stbi_load("container.jpg", &tex_width, &tex_height, &tex_num_channels, 0);

    if(data == nullptr) {
        spdlog::error("[STB_Image] Couldn't load file: container.jpg!");
    }

    unsigned int texture = 0;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tex_width, tex_height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);

    glBindTexture(GL_TEXTURE_2D, 0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    bool window_should_close = false;
    constexpr color clear_color{ 0.0F, 0.0F, 0.0F, 1.0F };

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

        glBindTexture(GL_TEXTURE_2D, texture);
        shader_program.use();
        glBindVertexArray(vao);

        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, nullptr);

        glBindVertexArray(0);
        shader::unbind();
        glBindTexture(GL_TEXTURE_2D, 0);

        SDL_GL_SwapWindow(window.get());
    }

    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ibo);

    SDL_GL_DeleteContext(gl_context);
}
