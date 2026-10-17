#pragma once

#include "core/geometry/Arc2.h"
#include "core/geometry/Circle2.h"
#include "core/geometry/Segment2.h"

#include <vector>

namespace microsw::geometry
{
enum class IntersectionKind { None, Proper, Endpoint, Tangent, Coincident, Overlap, Ambiguous };

struct Intersection2
{
    IntersectionKind kind{IntersectionKind::None};
    std::vector<Point2> points;
    [[nodiscard]] bool intersects() const noexcept { return kind != IntersectionKind::None; }
};

[[nodiscard]] Intersection2 intersect(const Segment2&, const Segment2&,
    math::Scalar tolerance = defaultGeometricTolerance);
[[nodiscard]] Intersection2 intersect(const Segment2&, const Circle2&,
    math::Scalar tolerance = defaultGeometricTolerance);
[[nodiscard]] Intersection2 intersect(const Segment2&, const Arc2&,
    math::Scalar tolerance = defaultGeometricTolerance);
[[nodiscard]] Intersection2 intersect(const Circle2&, const Circle2&,
    math::Scalar tolerance = defaultGeometricTolerance);
[[nodiscard]] Intersection2 intersect(const Circle2&, const Arc2&,
    math::Scalar tolerance = defaultGeometricTolerance);
[[nodiscard]] Intersection2 intersect(const Arc2&, const Arc2&,
    math::Scalar tolerance = defaultGeometricTolerance);
}
