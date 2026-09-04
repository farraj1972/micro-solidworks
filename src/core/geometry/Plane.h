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

    // Finite non-negative tolerance; zero uses the exact computed residual.
    [[nodiscard]] bool contains(const Point3& point,
        math::Scalar tolerance = defaultGeometricTolerance) const;

private:
    Point3 origin_;
    math::Vector3 normal_;
};

// Metric operations do not snap results using geometric tolerance.
// Non-representable results throw std::overflow_error.
[[nodiscard]] Point3 closestPoint(const Plane& primitive, const Point3& point);
[[nodiscard]] math::Scalar distance(const Plane& primitive, const Point3& point);
[[nodiscard]] math::Scalar signedDistance(const Plane& primitive, const Point3& point);

// Relations use dimensionless unit-vector residuals and the geometric default.
[[nodiscard]] bool isParallel(const Plane& a, const Plane& b,
    math::Scalar tolerance = defaultGeometricTolerance);
[[nodiscard]] bool isPerpendicular(const Plane& a, const Plane& b,
    math::Scalar tolerance = defaultGeometricTolerance);

}
