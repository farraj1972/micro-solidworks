#pragma once

#include "core/math/Vector3.h"

#include <cstddef>
#include <memory>
#include <span>

namespace microsw::rendering
{
class PointRenderer
{
public:
    PointRenderer();
    ~PointRenderer();
    PointRenderer(const PointRenderer&) = delete;
    PointRenderer& operator=(const PointRenderer&) = delete;
    PointRenderer(PointRenderer&&) noexcept;
    PointRenderer& operator=(PointRenderer&&) noexcept;

    // Empty input clears the renderer. Coordinates must be finite and
    // representable as float. Existing data is preserved on validation failure.
    void setVertices(std::span<const math::Vector3> vertices);

    // Caller supplies a bound shader with position at location 0. Drawing uses
    // GL_POINTS and restores the previous GL point size.
    void draw() const;
    [[nodiscard]] std::size_t vertexCount() const noexcept;

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};
}
