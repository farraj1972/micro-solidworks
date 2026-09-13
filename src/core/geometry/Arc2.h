#pragma once

#include "core/geometry/Circle2.h"

namespace microsw::geometry
{

class Arc2
{
public:
    Arc2(const Point2& center, math::Scalar radius,
         math::Scalar startAngle, math::Scalar sweepAngle);

    [[nodiscard]] constexpr const Point2& center() const noexcept { return circle_.center(); }
    [[nodiscard]] constexpr math::Scalar radius() const noexcept { return circle_.radius(); }
    [[nodiscard]] constexpr math::Scalar startAngle() const noexcept { return startAngle_; }
    [[nodiscard]] constexpr math::Scalar sweepAngle() const noexcept { return sweepAngle_; }

    [[nodiscard]] Point2 startPoint() const;
    [[nodiscard]] Point2 endPoint() const;
    [[nodiscard]] Point2 pointAt(math::Scalar t) const;
    [[nodiscard]] Point2 closestPoint(const Point2& point) const;
    [[nodiscard]] math::Scalar distance(const Point2& point) const;
    [[nodiscard]] bool contains(const Point2& point,
        math::Scalar tolerance = defaultGeometricTolerance) const;

private:
    [[nodiscard]] bool containsDirection(math::Scalar angle) const noexcept;

    Circle2 circle_;
    math::Scalar startAngle_;
    math::Scalar sweepAngle_;
};

}
