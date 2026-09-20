#pragma once

#include "core/sketch/Sketch.h"
#include "core/topology/Solid.h"
#include "presentation/SolidPresentation.h"

#include <optional>

namespace microsw
{
// Application-level owner for the current extrusion result. Regeneration first
// constructs and validates a complete replacement, so rejected input leaves the
// previous Solid and its stable visual identity untouched.
class ActiveExtrusion
{
public:
    void regenerate(const sketch::Sketch& sketch, math::Scalar distance);
    [[nodiscard]] bool hasSolid() const noexcept { return solid_.has_value(); }
    [[nodiscard]] const topology::Solid* solid() const noexcept;
    [[nodiscard]] const presentation::SolidPresentation& presentation() const noexcept
        { return presentation_; }

private:
    std::optional<topology::Solid> solid_;
    presentation::SolidPresentation presentation_;
};
}
