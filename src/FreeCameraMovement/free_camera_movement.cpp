#include <SDL.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include <spdlog/spdlog.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <memory>
#include <numeric>
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

class camera
{
private:
    glm::vec3 m_pos;
    glm::quat m_orient;

public:
    camera() noexcept = default;
    camera(camera const&) noexcept = default;
    camera(camera&&) noexcept = default;
    ~camera() noexcept = default;

    camera(glm::vec3 const& pos, glm::quat const& orient) noexcept
        : m_pos{ pos }
        , m_orient{ orient }
    {
    }
    explicit camera(glm::vec3 const& pos) noexcept
        : camera(pos, glm::quat{})
    {
    }

    auto operator=(camera const&) noexcept -> camera& = default;
    auto operator=(camera&&) noexcept -> camera& = default;

    auto position() const noexcept -> glm::vec3 const&
    {
        return m_pos;
    }

    auto orientation() const noexcept -> glm::quat const&
    {
        return m_orient;
    }

    auto view() const noexcept -> glm::mat4
    {
        return glm::translate(glm::mat4_cast(m_orient), m_pos);
    }

    auto translate(glm::vec3 const& v) noexcept -> void
    {
        m_pos += v * m_orient;
    }
    auto translate(float const x, float const y, float const z)
    {
        this->translate(glm::vec3{ x, y, z });
    }

    auto rotate(float const angle, glm::vec3 const& axis) noexcept -> void
    {
        m_orient *= glm::angleAxis(angle, axis * m_orient);
    }
    auto rotate(float const angle, float const x, float const y, float const z) noexcept -> void
    {
        this->rotate(angle, glm::vec3{ x, y, z });
    }

    auto yaw(float const angle) noexcept -> void
    {
        this->rotate(angle, 0.0F, 1.0F, 0.0F);
    }

    auto pitch(float const angle) noexcept -> void
    {
        this->rotate(angle, 1.0F, 0.0F, 0.0F);
    }

    auto roll(float const angle) noexcept -> void
    {
        this->rotate(angle, 0.0F, 0.0F, 1.0F);
    }
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

    int window_width = 1280; // NOLINT
    int window_height = 720; // NOLINT

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
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 0.5f,  -0.5f, -0.5f, 1.0f, 0.0f, 0.5f,  0.5f,  -0.5f, 1.0f, 1.0f, // NOLINT
        0.5f,  0.5f,  -0.5f, 1.0f, 1.0f, -0.5f, 0.5f,  -0.5f, 0.0f, 1.0f, -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, // NOLINT

        -0.5f, -0.5f, 0.5f,  0.0f, 0.0f, 0.5f,  -0.5f, 0.5f,  1.0f, 0.0f, 0.5f,  0.5f,  0.5f,  1.0f, 1.0f, // NOLINT
        0.5f,  0.5f,  0.5f,  1.0f, 1.0f, -0.5f, 0.5f,  0.5f,  0.0f, 1.0f, -0.5f, -0.5f, 0.5f,  0.0f, 0.0f, // NOLINT

        -0.5f, 0.5f,  0.5f,  1.0f, 0.0f, -0.5f, 0.5f,  -0.5f, 1.0f, 1.0f, -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, // NOLINT
        -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, -0.5f, -0.5f, 0.5f,  0.0f, 0.0f, -0.5f, 0.5f,  0.5f,  1.0f, 0.0f, // NOLINT

        0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.5f,  0.5f,  -0.5f, 1.0f, 1.0f, 0.5f,  -0.5f, -0.5f, 0.0f, 1.0f, // NOLINT
        0.5f,  -0.5f, -0.5f, 0.0f, 1.0f, 0.5f,  -0.5f, 0.5f,  0.0f, 0.0f, 0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // NOLINT

        -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.5f,  -0.5f, -0.5f, 1.0f, 1.0f, 0.5f,  -0.5f, 0.5f,  1.0f, 0.0f, // NOLINT
        0.5f,  -0.5f, 0.5f,  1.0f, 0.0f, -0.5f, -0.5f, 0.5f,  0.0f, 0.0f, -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, // NOLINT

        -0.5f, 0.5f,  -0.5f, 0.0f, 1.0f, 0.5f,  0.5f,  -0.5f, 1.0f, 1.0f, 0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // NOLINT
        0.5f,  0.5f,  0.5f,  1.0f, 0.0f, -0.5f, 0.5f,  0.5f,  0.0f, 0.0f, -0.5f, 0.5f,  -0.5f, 0.0f, 1.0f  // NOLINT
    };

    constexpr std::size_t num_verts = 36;
    std::vector<unsigned int> indices;
    indices.resize(num_verts);
    std::iota(indices.begin(), indices.end(), 0);

    unsigned int vao = 0;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    unsigned int vbo = 0;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), nullptr); // NOLINT
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat))); // NOLINT
    glEnableVertexAttribArray(1);

    shader shader_program{ "shader.vs.glsl", "shader.fs.glsl" };

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

    int tex2_width = 0;
    int tex2_height = 0;
    int tex2_num_channels = 0;
    stbi_set_flip_vertically_on_load(1);
    unsigned char* data2 = stbi_load("awesomeface.png", &tex2_width, &tex2_height, &tex2_num_channels, 0);

    if(data2 == nullptr) {
        spdlog::error("[STB_Image] Couldn't load file: awesomeface.png!");
    }

    unsigned int texture2 = 0;
    glGenTextures(1, &texture2);
    glBindTexture(GL_TEXTURE_2D, texture2);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex2_width, tex2_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data2);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data2);

    glBindTexture(GL_TEXTURE_2D, 0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glm::vec3 camera_pos{ 0.0F, 0.0F, 3.0F }; // NOLINT
    glm::vec3 camera_front{ 0.0F, 0.0F, -1.0F };
    camera cam{ camera_pos, camera_front };

    constexpr float translate_offset = 0.5F;
    constexpr float roll_offset = 0.5F;

    auto const fwidth = static_cast<float>(window_width);
    auto const fheight = static_cast<float>(window_height);
    float fov = 45.0F; // NOLINT
    constexpr float near = 0.1F;
    constexpr float far = 100.0F;
    glm::mat4 projection = glm::perspective(glm::radians(fov), fwidth / fheight, near, far);

    constexpr int num_cubes = 10;
    std::array<glm::vec3, num_cubes> positions = {
        glm::vec3{ 0.0f, 0.0f, 0.0f },    glm::vec3{ 2.0f, 5.0f, -15.0f },   // NOLINT
        glm::vec3{ -1.5f, -2.2f, -2.5f }, glm::vec3{ -3.8f, -2.0f, -12.3f }, // NOLINT
        glm::vec3{ 2.4f, -0.4f, -3.5f },  glm::vec3{ -1.7f, 3.0f, -7.5f },   // NOLINT
        glm::vec3{ 1.3f, -2.0f, -2.5f },  glm::vec3{ 1.5f, 2.0f, -2.5f },    // NOLINT
        glm::vec3{ 1.5f, 0.2f, -1.5f },   glm::vec3{ -1.3f, 1.0f, -1.5f }    // NOLINT
    };

    shader_program.use();
    shader_program.set_int("texture1", 0);
    shader_program.set_int("texture2", 1);
    shader_program.set_mat4("projection", projection);
    shader::unbind();

    bool window_should_close = false;
    constexpr color clear_color{ 0.0F, 0.0F, 0.0F, 1.0F };

    int last_mouse_x = window_width / 2;  // NOLINT
    int last_mouse_y = window_height / 2; // NOLINT

    float yaw = -90.0F; // NOLINT
    float pitch = 0.0F;

    bool dragging = false;

    glEnable(GL_DEPTH_TEST);

    auto start = std::chrono::steady_clock::now();

    while(!window_should_close) {
        using namespace std::chrono;
        auto end = steady_clock::now();
        float const elapsed = duration<float>{ end - start }.count();
        start = end;

        SDL_Event e;
        while(SDL_PollEvent(&e) != 0) {
            switch(e.type) {
            case SDL_QUIT: {
                window_should_close = true;
                break;
            }
            case SDL_WINDOWEVENT: {
                if(e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                    window_width = e.window.data1;
                    window_height = e.window.data2;
                    glViewport(0, 0, e.window.data1, e.window.data2);
                    shader_program.use();
                    shader_program.set_mat4(
                        "projection",
                        glm::perspective(
                            glm::radians(fov), static_cast<float>(e.window.data1) / e.window.data2, near, far));
                    shader::unbind();
                }
                break;
            }
            case SDL_KEYDOWN: {
                float const camera_speed = 50.0F * elapsed;

                switch(e.key.keysym.sym) {
                case SDLK_ESCAPE: {
                    window_should_close = true;
                    break;
                }
                case SDLK_UP: {
                    cam.translate(0.0F, 0.0F, translate_offset * camera_speed);
                    break;
                }
                case SDLK_DOWN: {
                    cam.translate(0.0F, 0.0F, -translate_offset * camera_speed);
                    break;
                }
                case SDLK_LEFT: {
                    cam.translate(translate_offset * camera_speed, 0.0F, 0.0F);
                    break;
                }
                case SDLK_RIGHT: {
                    cam.translate(-translate_offset * camera_speed, 0.0F, 0.0F);
                    break;
                }
                case SDLK_w: {
                    cam.translate(0.0F, -translate_offset * camera_speed, 0.0F);
                    break;
                }
                case SDLK_s: {
                    cam.translate(0.0F, translate_offset * camera_speed, 0.0F);
                    break;
                }
                case SDLK_q: {
                    cam.roll(roll_offset * camera_speed);
                    break;
                }
                case SDLK_e: {
                    cam.roll(-roll_offset * camera_speed);
                    break;
                }
                default: {
                    break;
                }
                }
                break;
            }
            case SDL_MOUSEBUTTONDOWN: {
                if(e.button.button == SDL_BUTTON_LEFT) {
                    dragging = true;
                    last_mouse_x = e.button.x;
                    last_mouse_y = e.button.y;
                }
                break;
            }
            case SDL_MOUSEBUTTONUP: {
                if(e.button.button == SDL_BUTTON_LEFT) {
                    dragging = false;
                }
                break;
            }
            case SDL_MOUSEWHEEL: {
                if(e.wheel.y != 0) {
                    fov -= e.wheel.y;

                    if(fov < 1.0F) {
                        fov = 1.0F;
                    }
                    if(fov > 45.0F) { // NOLINT
                        fov = 45.0F;  // NOLINT
                    }

                    float const a = static_cast<float>(window_width) / static_cast<float>(window_height);
                    glm::mat4 proj = glm::perspective(glm::radians(fov), a, near, far);

                    shader_program.use();
                    shader_program.set_mat4("projection", proj);
                    shader::unbind();
                }
                break;
            }
            default: {
                break;
            }
            }
        }

        if(dragging) {
            int mouse_x = 0;
            int mouse_y = 0;
            SDL_GetMouseState(&mouse_x, &mouse_y);

            auto x_offset = static_cast<float>(mouse_x - last_mouse_x);
            auto y_offset = static_cast<float>(last_mouse_y - mouse_y);

            last_mouse_x = mouse_x;
            last_mouse_y = mouse_y;

            constexpr float sensitivity = 0.001F;

            x_offset *= sensitivity;
            y_offset *= sensitivity;

            cam.yaw(-x_offset);
            cam.pitch(y_offset);
        }

        glClearColor(clear_color.r, clear_color.g, clear_color.b, clear_color.a);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture2);

        constexpr float to_seconds = 1'000.0F;
        glm::mat4 view = cam.view();

        shader_program.use();
        shader_program.set_mat4("view", view);
        glBindVertexArray(vao);
        for(std::size_t i = 0; i < positions.size(); ++i) {
            glm::mat4 model{ 1.0F };
            model = glm::translate(model, positions[i]);
            float angle = i * 20.0F * (SDL_GetTicks() / to_seconds);                                         // NOLINT
            model = model * glm::toMat4(glm::angleAxis(glm::radians(angle), glm::vec3{ 1.0F, 0.3F, 0.5F })); // NOLINT
            shader_program.set_mat4("model", model);

            glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, nullptr);
        }

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
