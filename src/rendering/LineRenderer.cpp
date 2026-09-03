#include "rendering/LineRenderer.h"

#include <glad/gl.h>

#include <cmath>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

namespace microsw::rendering
{
namespace
{

void requireContext()
{
    if (!GLAD_GL_VERSION_3_3 || !glGetString || glGetString(GL_VERSION) == nullptr)
    {
        throw std::runtime_error{"LineRenderer requires a current OpenGL 3.3 context and loaded GLAD"};
    }
}

}

class LineRenderer::Impl
{
public:
    Impl()
    {
        requireContext();
        if (glGetError() != GL_NO_ERROR)
        {
            throw std::runtime_error{"Pending OpenGL error before LineRenderer construction"};
        }
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        if (vao == 0 || vbo == 0)
        {
            glDeleteVertexArrays(1, &vao);
            glDeleteBuffers(1, &vbo);
            throw std::runtime_error{"Failed to create line VAO/VBO"};
        }

        GLint previousVao{}, previousBuffer{};
        glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &previousVao);
        glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &previousBuffer);
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), nullptr);
        glBindVertexArray(static_cast<GLuint>(previousVao));
        glBindBuffer(GL_ARRAY_BUFFER, static_cast<GLuint>(previousBuffer));
        const GLenum error = glGetError();
        if (error != GL_NO_ERROR)
        {
            glDeleteVertexArrays(1, &vao);
            glDeleteBuffers(1, &vbo);
            throw std::runtime_error{"Line VAO configuration failed: " + std::to_string(error)};
        }
    }

    ~Impl()
    {
        // VAO first: release its reference to the owned VBO.
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &vbo);
    }

    GLuint vao{};
    GLuint vbo{};
    std::size_t count{};
};

LineRenderer::LineRenderer() : impl_{std::make_unique<Impl>()} {}
LineRenderer::~LineRenderer() = default;
LineRenderer::LineRenderer(LineRenderer&&) noexcept = default;
LineRenderer& LineRenderer::operator=(LineRenderer&&) noexcept = default;

std::size_t LineRenderer::vertexCount() const noexcept
{
    return impl_ ? impl_->count : 0;
}

void LineRenderer::setVertices(std::span<const math::Vector3> vertices)
{
    if (!impl_)
    {
        throw std::logic_error{"Cannot upload to a moved-from LineRenderer"};
    }
    requireContext();
    if (vertices.size() % 2 != 0)
    {
        throw std::invalid_argument{"GL_LINES requires an even vertex count"};
    }
    constexpr std::size_t bytesPerVertex = 3 * sizeof(GLfloat);
    if (vertices.size() > static_cast<std::size_t>(std::numeric_limits<GLsizei>::max())
        || vertices.size() > static_cast<std::size_t>(std::numeric_limits<GLsizeiptr>::max()) / bytesPerVertex)
    {
        throw std::length_error{"Line vertex data exceeds OpenGL count/size limits"};
    }

    std::vector<GLfloat> data;
    data.reserve(vertices.size() * 3);
    for (const auto& vertex : vertices)
    {
        for (const auto coordinate : {vertex.x(), vertex.y(), vertex.z()})
        {
            if (!std::isfinite(coordinate)
                || std::abs(coordinate) > std::numeric_limits<GLfloat>::max())
            {
                throw std::invalid_argument{"Line coordinates must be finite and float-representable"};
            }
            // Explicit rendering conversion; no assumption about Vector3 storage.
            data.push_back(static_cast<GLfloat>(coordinate));
        }
    }

    GLint previousBuffer{};
    if (glGetError() != GL_NO_ERROR)
    {
        throw std::runtime_error{"Pending OpenGL error before line upload"};
    }
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &previousBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, impl_->vbo);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(vertices.size() * bytesPerVertex),
                 data.empty() ? nullptr : data.data(), GL_DYNAMIC_DRAW);
    const GLenum error = glGetError();
    glBindBuffer(GL_ARRAY_BUFFER, static_cast<GLuint>(previousBuffer));
    if (error != GL_NO_ERROR)
    {
        throw std::runtime_error{"Line vertex upload failed: " + std::to_string(error)};
    }
    impl_->count = vertices.size();
}

void LineRenderer::draw() const
{
    if (vertexCount() == 0)
    {
        return;
    }
    requireContext();
    glBindVertexArray(impl_->vao);
    glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(impl_->count));
}

}
