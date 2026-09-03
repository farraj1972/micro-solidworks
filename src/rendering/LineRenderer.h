#pragma once

#include "core/math/Vector3.h"

#include <cstddef>
#include <memory>
#include <span>

namespace microsw::rendering
{

// Requires loaded GLAD and a compatible current OpenGL 3.3 context for
// construction, upload, drawing and destruction (including move assignment).
// Does not own a shader or configure render-pass state.
// Construction/upload require no pending GL errors and report failures as runtime_error.
class LineRenderer
{
public:
    LineRenderer();
    ~LineRenderer();
    LineRenderer(const LineRenderer&) = delete;
    LineRenderer& operator=(const LineRenderer&) = delete;
    LineRenderer(LineRenderer&&) noexcept;
    LineRenderer& operator=(LineRenderer&&) noexcept;

    // Pairs are independent segments. Empty input clears the renderer.
    // Odd counts, non-finite/out-of-float-range coordinates: invalid_argument.
    // Counts exceeding GPU API limits: length_error. Validation precedes upload.
    // Construction/upload restore previous VAO/array-buffer bindings.
    void setVertices(std::span<const math::Vector3> vertices);

    // Caller supplies a bound shader with position at location 0.
    // Nonempty draw leaves this VAO bound; all other render state is unchanged.
    // Empty and moved-from draw are no-ops. Upload on moved-from throws logic_error.
    void draw() const;
    [[nodiscard]] std::size_t vertexCount() const noexcept;

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

}
