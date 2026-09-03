#pragma once

#include "core/math/Matrix4.h"

#include <memory>
#include <string_view>

namespace microsw::rendering
{

// Requires GLAD loaded and a compatible OpenGL 3.3 context current throughout
// construction, use and destruction (including move-assignment cleanup).
class ShaderProgram
{
public:
    ShaderProgram(std::string_view vertexSource, std::string_view fragmentSource);
    // Unbinds this program if current, then deletes it; other bindings are preserved.
    ~ShaderProgram();

    ShaderProgram(const ShaderProgram&) = delete;
    ShaderProgram& operator=(const ShaderProgram&) = delete;
    ShaderProgram(ShaderProgram&& other) noexcept;
    ShaderProgram& operator=(ShaderProgram&& other) noexcept;

    // Moved-from objects reject use with std::logic_error.
    void bind() const;

    // Requires this program bound; otherwise throws std::logic_error.
    // Missing/invalid names throw std::invalid_argument (including optimized-out uniforms).
    // Converts finite, float-representable coefficients to column-major floats.
    void setMatrix4(std::string_view name, const math::Matrix4& value) const;

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

}
