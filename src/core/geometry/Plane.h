#pragma once

#include "core/geometry/Point3.h"

namespace microsw::geometry
{

// Infinite oriented 3D plane: { P | dot(P - origin, normal) = 0 }.
// Opposite normals preserve opposite orientations of the same point set.
// Only the supplied origin and a unit normal are stored; Plane is not a Face.
class Plane
{
public:
    // Non-finite normals or Euclidean magnitude <= the default geometric
    // tolerance throw std::invalid_argument. Normalization preserves orientation.
    Plane(const Point3& origin, const math::Vector3& normal);

    [[nodiscard]] const Point3& origin() const noexcept { return origin_; }
    [[nodiscard]] const math::Vector3& normal() const noexcept { return normal_; }

private:
    Point3 origin_;
    math::Vector3 normal_;
};

}
