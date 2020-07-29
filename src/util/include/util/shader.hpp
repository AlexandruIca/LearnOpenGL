#ifndef UTIL_SHADER_HPP
#define UTIL_SHADER_HPP
#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <string>

class shader
{
private:
    unsigned int m_id;

    enum shader_type
    {
        vertex = GL_VERTEX_SHADER,
        fragment = GL_FRAGMENT_SHADER
    };

    [[nodiscard]] auto create_shader(shader_type type, char const* source) -> unsigned int;
    [[nodiscard]] auto create_program(unsigned int vs, unsigned int fs) -> unsigned int;

public:
    shader() = delete;
    shader(shader const&) noexcept = default;
    shader(shader&&) noexcept = default;
    ~shader() noexcept = default;

    shader(std::string const& vs_path, std::string const& fs_path);

    auto operator=(shader const&) noexcept -> shader& = default;
    auto operator=(shader&&) noexcept -> shader& = default;

    auto use() const noexcept -> void;

    auto set_bool(std::string const& id, bool value) const noexcept -> void;
    auto set_int(std::string const& id, int value) const noexcept -> void;
    auto set_float(std::string const& id, float value) const noexcept -> void;
    auto set_mat4(std::string const& id, glm::mat4 const& value) const noexcept -> void;

    static auto unbind() noexcept -> void;
};

#endif // !UTIL_SHADER_HPP
