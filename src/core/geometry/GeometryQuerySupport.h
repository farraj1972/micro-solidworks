#pragma once

#include "core/geometry/Point3.h"
#include "core/math/Vector3.h"

// Implementation detail shared by primitive .cpp files, not a query API.
namespace microsw::geometry::detail
{
void validateTolerance(math::Scalar tolerance);
void validateParameter(math::Scalar t);
Point3 nearestSegment(const Point3& a, const Point3& b, const Point3& point);
Point3 nearestLine(const Point3& origin, const math::Vector3& direction,
                   const Point3& point, bool forwardOnly);
Point3 nearestPlane(const Point3& origin, const math::Vector3& normal, const Point3& point);
math::Scalar pointMetric(const Point3& a, const Point3& b);
math::Scalar lineMetric(const Point3& origin, const math::Vector3& direction,
                        const Point3& point, bool forwardOnly);
math::Scalar planeSignedMetric(const Point3& origin, const math::Vector3& normal,
                               const Point3& point);
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
