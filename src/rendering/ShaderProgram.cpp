#include "rendering/ShaderProgram.h"

#include <glad/gl.h>

#include <array>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <string>
#include <utility>

namespace microsw::rendering
{
namespace
{

void requireContext()
{
    if (!GLAD_GL_VERSION_3_3 || !glGetString || glGetString(GL_VERSION) == nullptr)
    {
        throw std::runtime_error{"ShaderProgram requires a current OpenGL 3.3 context and loaded GLAD"};
    }
}

std::string shaderLog(GLuint shader)
{
    GLint length{};
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
    std::string log(static_cast<std::size_t>(length > 0 ? length : 1), '\0');
    GLsizei written{};
    glGetShaderInfoLog(shader, static_cast<GLsizei>(log.size()), &written, log.data());
    log.resize(static_cast<std::size_t>(written));
    return log;
}

std::string programLog(GLuint program)
{
    GLint length{};
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
    std::string log(static_cast<std::size_t>(length > 0 ? length : 1), '\0');
    GLsizei written{};
    glGetProgramInfoLog(program, static_cast<GLsizei>(log.size()), &written, log.data());
    log.resize(static_cast<std::size_t>(written));
    return log;
}

class CompiledShader
{
public:
    CompiledShader(GLenum type, std::string_view source, const char* stage)
    {
        if (source.empty() || source.size() > static_cast<std::size_t>(std::numeric_limits<GLint>::max()))
        {
            throw std::invalid_argument{std::string{stage} + " shader source is empty or too large"};
        }
        id = glCreateShader(type);
        if (id == 0)
        {
            throw std::runtime_error{std::string{"Failed to create "} + stage + " shader"};
        }
        try
        {
            const char* data = source.data();
            const GLint length = static_cast<GLint>(source.size());
            glShaderSource(id, 1, &data, &length);
            glCompileShader(id);
            GLint compiled{};
            glGetShaderiv(id, GL_COMPILE_STATUS, &compiled);
            if (compiled != GL_TRUE)
            {
                throw std::runtime_error{std::string{stage} + " shader compilation failed: " + shaderLog(id)};
            }
        }
        catch (...)
        {
            glDeleteShader(id);
            throw;
        }
    }

    ~CompiledShader() { glDeleteShader(id); }
    CompiledShader(const CompiledShader&) = delete;
    CompiledShader& operator=(const CompiledShader&) = delete;
    GLuint id{};
};

}

class ShaderProgram::Impl
{
public:
    Impl(std::string_view vertexSource, std::string_view fragmentSource)
    {
        requireContext();
        const CompiledShader vertex{GL_VERTEX_SHADER, vertexSource, "vertex"};
        const CompiledShader fragment{GL_FRAGMENT_SHADER, fragmentSource, "fragment"};
        program = glCreateProgram();
        if (program == 0)
        {
            throw std::runtime_error{"Failed to create shader program"};
        }
        try
        {
            glAttachShader(program, vertex.id);
            glAttachShader(program, fragment.id);
            glLinkProgram(program);
            GLint linked{};
            glGetProgramiv(program, GL_LINK_STATUS, &linked);
            if (linked != GL_TRUE)
            {
                throw std::runtime_error{"Shader program link failed: " + programLog(program)};
            }
            // Linked executable survives; detached stage objects are deleted by RAII.
            glDetachShader(program, vertex.id);
            glDetachShader(program, fragment.id);
        }
        catch (...)
        {
            glDeleteProgram(program);
            throw;
        }
    }

    ~Impl()
    {
        GLint current{};
        glGetIntegerv(GL_CURRENT_PROGRAM, &current);
        // Ensure deletion is immediate if this program is currently bound.
        if (static_cast<GLuint>(current) == program)
        {
            glUseProgram(0);
        }
        glDeleteProgram(program);
    }

    GLuint program{};
};

ShaderProgram::ShaderProgram(std::string_view vertexSource, std::string_view fragmentSource)
    : impl_{std::make_unique<Impl>(vertexSource, fragmentSource)}
{
}

ShaderProgram::~ShaderProgram() = default;
ShaderProgram::ShaderProgram(ShaderProgram&& other) noexcept = default;
ShaderProgram& ShaderProgram::operator=(ShaderProgram&& other) noexcept = default;

void ShaderProgram::bind() const
{
    if (!impl_)
    {
        throw std::logic_error{"Cannot bind a moved-from ShaderProgram"};
    }
    requireContext();
    glUseProgram(impl_->program);
}

void ShaderProgram::setMatrix4(std::string_view name, const math::Matrix4& value) const
{
    if (!impl_)
    {
        throw std::logic_error{"Cannot upload to a moved-from ShaderProgram"};
    }
    requireContext();
    GLint current{};
    glGetIntegerv(GL_CURRENT_PROGRAM, &current);
    if (static_cast<GLuint>(current) != impl_->program)
    {
        throw std::logic_error{"ShaderProgram must be bound before uniform upload"};
    }
    if (name.empty() || name.find('\0') != std::string_view::npos)
    {
        throw std::invalid_argument{"Uniform name must be nonempty and contain no null characters"};
    }
    const std::string terminatedName{name};
    const GLint location = glGetUniformLocation(impl_->program, terminatedName.c_str());
    if (location == -1)
    {
        throw std::invalid_argument{"Uniform not found or inactive: " + terminatedName};
    }
    std::array<GLfloat, 16> data{};
    for (std::size_t column = 0; column < 4; ++column)
    {
        for (std::size_t row = 0; row < 4; ++row)
        {
            const auto coefficient = value(row, column);
            if (!std::isfinite(coefficient)
                || std::abs(coefficient) > std::numeric_limits<GLfloat>::max())
            {
                throw std::invalid_argument{"Matrix coefficient must be finite and float-representable"};
            }
            // Explicit rendering boundary: never rely on Matrix4 physical storage.
            data[column * 4 + row] = static_cast<GLfloat>(coefficient);
        }
    }
    glUniformMatrix4fv(location, 1, GL_FALSE, data.data());
}

void ShaderProgram::setVector3(std::string_view name, const math::Vector3& value) const
{
    if (!impl_)
    {
        throw std::logic_error{"Cannot upload to a moved-from ShaderProgram"};
    }
    requireContext();
    GLint current{};
    glGetIntegerv(GL_CURRENT_PROGRAM, &current);
    if (static_cast<GLuint>(current) != impl_->program)
    {
        throw std::logic_error{"ShaderProgram must be bound before uniform upload"};
    }
    if (name.empty() || name.find('\0') != std::string_view::npos)
    {
        throw std::invalid_argument{"Uniform name must be nonempty and contain no null characters"};
    }
    const std::string terminatedName{name};
    const GLint location = glGetUniformLocation(impl_->program, terminatedName.c_str());
    if (location == -1)
    {
        throw std::invalid_argument{"Uniform not found or inactive: " + terminatedName};
    }
    const std::array<math::Scalar, 3> components{value.x(), value.y(), value.z()};
    std::array<GLfloat, 3> data{};
    for (std::size_t index = 0; index < components.size(); ++index)
    {
        if (!std::isfinite(components[index])
            || std::abs(components[index]) > std::numeric_limits<GLfloat>::max())
        {
            throw std::invalid_argument{"Vector component must be finite and float-representable"};
        }
        data[index] = static_cast<GLfloat>(components[index]);
    }
    glUniform3fv(location, 1, data.data());
}

}
