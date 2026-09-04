#pragma once

#include "core/geometry/Point3.h"
#include "core/math/Vector3.h"

// Implementation detail shared by primitive .cpp files, not a query API.
namespace microsw::geometry::detail
{
void validateTolerance(math::Scalar tolerance);
void validateParameter(math::Scalar t);
bool onSupport(const Point3& origin, const math::Vector3& direction,
               const Point3& point, math::Scalar tolerance);
bool inForwardHalfSpace(const Point3& origin, const math::Vector3& direction,
                        const Point3& point, math::Scalar tolerance);
bool onPlane(const Point3& origin, const math::Vector3& normal,
             const Point3& point, math::Scalar tolerance);
bool inSegment(const Point3& a, const Point3& b, const Point3& point,
               math::Scalar tolerance);
bool parallel(const math::Vector3& a, const math::Vector3& b, math::Scalar tolerance);
bool perpendicular(const math::Vector3& a, const math::Vector3& b, math::Scalar tolerance);
}
