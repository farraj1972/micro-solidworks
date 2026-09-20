#pragma once

#include "core/geometry/Plane.h"
#include "core/geometry/Segment3.h"

#include <vector>

namespace microsw::modeling
{
class Profile
{
public:
    Profile(std::vector<geometry::Point3> boundary, const geometry::Plane& supportPlane);
    [[nodiscard]] const std::vector<geometry::Point3>& boundary() const noexcept { return boundary_; }
    [[nodiscard]] const geometry::Plane& supportPlane() const noexcept { return supportPlane_; }
    [[nodiscard]] std::vector<geometry::Segment3> segments() const;
private:
    std::vector<geometry::Point3> boundary_;
    geometry::Plane supportPlane_;
};
}
