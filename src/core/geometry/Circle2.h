#pragma once

#include "core/geometry/Point2.h"

namespace microsw::geometry
{

class Circle2
{
public:
    Circle2(const Point2& center, math::Scalar radius);

    [[nodiscard]] constexpr const Point2& center() const noexcept { return center_; }
    [[nodiscard]] constexpr math::Scalar radius() const noexcept { return radius_; }

    [[nodiscard]] Point2 pointAt(math::Scalar angle) const;
    [[nodiscard]] Point2 closestPoint(const Point2& point) const;
    [[nodiscard]] math::Scalar distance(const Point2& point) const;
    [[nodiscard]] bool contains(const Point2& point,
        math::Scalar tolerance = defaultGeometricTolerance) const;

private:
    Point2 center_;
    math::Scalar radius_;
};

}
