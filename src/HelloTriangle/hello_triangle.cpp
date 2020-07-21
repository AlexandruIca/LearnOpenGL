#include <SDL.h>
#include <glad/glad.h>
#include <spdlog/spdlog.h>

#include <cstdlib>
#include <memory>
#include <string>
#include <vector>

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

    std::vector<GLfloat> const vertices = {
        0.5F, 0.5F, 0.0F, 0.5F, -0.5F, 0.0F, -0.5F, -0.5F, 0.0F, -0.5F, 0.5F, 0.0F
    };

    std::vector<unsigned int> indices = { 0, 1, 3, 1, 2, 3 };

    char const* const vertex_shader_source = R"(
    #version 460 core

    layout(location = 0) in vec3 pos;

    void main() {
        gl_Position = vec4(pos.xyz, 1.0);
    }
    )";

    char const* const fragment_shader_source = R"(
    #version 460 core

    out vec4 fragColor;

    void main() {
        fragColor = vec4(1.0f, 0.5f, 0.25f, 1.0f);
    }
    )";

    unsigned int vao = 0;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    unsigned int vbo = 0;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);

    unsigned int vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_source, nullptr);
    glCompileShader(vertex_shader);

    int success = 0;
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);

    if(success == 0) {
        int log_length = 0;
        glGetShaderiv(vertex_shader, GL_INFO_LOG_LENGTH, &log_length);

        std::unique_ptr<char> shader_log{ new char[log_length] };

        glGetShaderInfoLog(vertex_shader, log_length, nullptr, shader_log.get());
        spdlog::error("[Vertex Shader] Error compiling vertex shader: {}!", shader_log.get());
    }

    unsigned int fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_source, nullptr);
    glCompileShader(fragment_shader);

    int success2 = 0;
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success2);

    if(success2 == 0) {
        int log_length = 0;
        glGetShaderiv(fragment_shader, GL_INFO_LOG_LENGTH, &log_length);
        std::unique_ptr<char> shader_log{ new char[log_length] };
        glGetShaderInfoLog(fragment_shader, log_length, nullptr, shader_log.get());
        spdlog::error("[Fragment Shader] Error compiling fragment shader: {}!", shader_log.get());
    }

    unsigned int shader_program = glCreateProgram();
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);
    glLinkProgram(shader_program);

    int success3 = 0;
    glGetProgramiv(shader_program, GL_LINK_STATUS, &success3);

    if(success3 == 0) {
        int program_log_length = 0;
        glGetProgramiv(shader_program, GL_INFO_LOG_LENGTH, &program_log_length);
        std::unique_ptr<char> program_log{ new char[program_log_length] };
        glGetProgramInfoLog(shader_program, program_log_length, nullptr, program_log.get());
        spdlog::error("[Shader Linking] Error linking shaders: {}!", program_log.get());
    }

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), nullptr);
    glEnableVertexAttribArray(0);

    unsigned int ibo = 0;
    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

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

        glUseProgram(shader_program);
        glBindVertexArray(vao);

        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, nullptr);

        glBindVertexArray(0);
        glUseProgram(0);

        SDL_GL_SwapWindow(window.get());
    }

    SDL_GL_DeleteContext(gl_context);
}
