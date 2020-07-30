#include "util/shader.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <spdlog/spdlog.h>

#include <fstream>
#include <memory>
#include <sstream>

auto shader::create_shader(shader_type const type, char const* const source) -> unsigned int
{
    unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    int success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if(success == 0) {
        int log_length = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);

        std::unique_ptr<char> shader_log{ new char[log_length] };

        glGetShaderInfoLog(shader, log_length, nullptr, shader_log.get());
        if(type == shader_type::vertex) {
            spdlog::error("[Vertex Shader] Error compiling vertex shader: {}!", shader_log.get());
        }
        else {
            spdlog::error("[Fragment Shader] Error compiling fragment shader: {}!", shader_log.get());
        }
    }

    return shader;
}

auto shader::create_program(unsigned int const vs, unsigned int const fs) -> unsigned int
{
    unsigned int shader_program = glCreateProgram();
    glAttachShader(shader_program, vs);
    glAttachShader(shader_program, fs);
    glLinkProgram(shader_program);

    int success = 0;
    glGetProgramiv(shader_program, GL_LINK_STATUS, &success);

    if(success == 0) {
        int program_log_length = 0;
        glGetProgramiv(shader_program, GL_INFO_LOG_LENGTH, &program_log_length);
        std::unique_ptr<char> program_log{ new char[program_log_length] };
        glGetProgramInfoLog(shader_program, program_log_length, nullptr, program_log.get());
        spdlog::error("[Shader Linking] Error linking shaders: {}!", program_log.get());
    }

    return shader_program;
}

shader::shader(std::string const& vs_path, std::string const& fs_path)
    : m_id{ 0 }
{
    std::ifstream vs_file{ vs_path };
    std::ifstream fs_file{ fs_path };

    std::ostringstream vss{};
    std::ostringstream fss{};

    vss << vs_file.rdbuf();
    fss << fs_file.rdbuf();

    std::string const vs_source = vss.str();
    std::string const fs_source = fss.str();

    unsigned int vs = this->create_shader(shader_type::vertex, vs_source.c_str());
    unsigned int fs = this->create_shader(shader_type::fragment, fs_source.c_str());

    m_id = this->create_program(vs, fs);
}

auto shader::use() const noexcept -> void
{
    glUseProgram(m_id);
}

auto shader::set_bool(std::string const& id, bool const value) const noexcept -> void
{
    glUniform1i(glGetUniformLocation(m_id, id.c_str()), static_cast<int>(value));
}

auto shader::set_int(std::string const& id, int const value) const noexcept -> void
{
    glUniform1i(glGetUniformLocation(m_id, id.c_str()), value);
}

auto shader::set_float(std::string const& id, float const value) const noexcept -> void
{
    glUniform1f(glGetUniformLocation(m_id, id.c_str()), value);
}

auto shader::set_vec4(std::string const& id, glm::vec4 const& value) const noexcept -> void
{
    glUniform4f(glGetUniformLocation(m_id, id.c_str()), value.x, value.y, value.z, value.w);
}

auto shader::set_mat4(std::string const& id, glm::mat4 const& value) const noexcept -> void
{
    glUniformMatrix4fv(glGetUniformLocation(m_id, id.c_str()), 1, GL_FALSE, glm::value_ptr(value));
}

auto shader::unbind() noexcept -> void
{
    glUseProgram(0);
}
