#include "core/sketch/SketchPlane.h"

#include "core/geometry/GeometricTolerance.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace microsw::sketch
{
namespace
{
math::Vector3 validatedUnit(const math::Vector3& axis)
{
    if (!std::isfinite(axis.x()) || !std::isfinite(axis.y()) || !std::isfinite(axis.z()))
        throw std::invalid_argument{"SketchPlane axes must be finite"};
    const auto scale = std::max({std::abs(axis.x()), std::abs(axis.y()), std::abs(axis.z())});
    if (scale <= geometry::defaultGeometricTolerance
        && std::hypot(axis.x(), axis.y(), axis.z()) <= geometry::defaultGeometricTolerance)
        throw std::invalid_argument{"SketchPlane axes must be non-degenerate"};
    const auto scaled = math::Vector3{axis.x() / scale, axis.y() / scale, axis.z() / scale};
    return scaled / std::hypot(scaled.x(), scaled.y(), scaled.z());
}

math::Scalar checked(math::Scalar value)
{
    if (!std::isfinite(value))
        throw std::overflow_error{"SketchPlane conversion is not representable"};
    return value;
}
}

SketchPlane::SketchPlane(const geometry::Point3& origin,
                         const math::Vector3& xAxis,
                         const math::Vector3& yAxis)
    : origin_{origin}, xAxis_{validatedUnit(xAxis)}, yAxis_{validatedUnit(yAxis)}
{
    if (std::abs(math::dot(xAxis_, yAxis_)) > geometry::defaultGeometricTolerance)
        throw std::invalid_argument{"SketchPlane axes must be orthogonal"};
}

math::Vector3 SketchPlane::normal() const noexcept
{
    return math::cross(xAxis_, yAxis_);
}

geometry::Point3 SketchPlane::toWorld(const geometry::Point2& point) const
{
    return {checked(origin_.x() + point.x() * xAxis_.x() + point.y() * yAxis_.x()),
            checked(origin_.y() + point.x() * xAxis_.y() + point.y() * yAxis_.y()),
            checked(origin_.z() + point.x() * xAxis_.z() + point.y() * yAxis_.z())};
}

math::Vector3 SketchPlane::toWorld(const math::Vector2& vector) const
{
    const math::Vector3 result{
        checked(vector.x() * xAxis_.x() + vector.y() * yAxis_.x()),
        checked(vector.x() * xAxis_.y() + vector.y() * yAxis_.y()),
        checked(vector.x() * xAxis_.z() + vector.y() * yAxis_.z())};
    return result;
}

geometry::Point2 SketchPlane::toLocal(const geometry::Point3& point) const
{
    const auto displacement = point - origin_;
    return {checked(math::dot(displacement, xAxis_)),
            checked(math::dot(displacement, yAxis_))};
}

geometry::Plane SketchPlane::plane() const
{
    return {origin_, normal()};
}

}
