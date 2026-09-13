#include "core/geometry/Arc2.h"
#include "core/geometry/GeometryQuerySupport.h"

#include <cmath>
#include <numbers>
#include <stdexcept>

namespace microsw::geometry
{
namespace
{
constexpr auto twoPi = 2.0 * std::numbers::pi_v<math::Scalar>;

math::Scalar positiveModulo(math::Scalar angle) noexcept
{
    auto result = std::fmod(angle, twoPi);
    if (result < 0) result += twoPi;
    return result;
}

math::Scalar pointDistance(const Point2& a, const Point2& b)
{
    const auto result = std::hypot(a.x() - b.x(), a.y() - b.y());
    if (!std::isfinite(result))
        throw std::overflow_error{"Arc2 distance is not representable"};
    return result;
}
}

Arc2::Arc2(const Point2& center, math::Scalar radius,
           math::Scalar startAngle, math::Scalar sweepAngle)
    : circle_{center, radius}, startAngle_{startAngle}, sweepAngle_{sweepAngle}
{
    if (!std::isfinite(startAngle) || !std::isfinite(sweepAngle)
        || std::abs(sweepAngle) <= 0 || std::abs(sweepAngle) >= twoPi)
        throw std::invalid_argument{"Arc2 angles must be finite and sweep magnitude must be in (0, 2*pi)"};
}

Point2 Arc2::startPoint() const { return circle_.pointAt(startAngle_); }
Point2 Arc2::endPoint() const { return circle_.pointAt(startAngle_ + sweepAngle_); }

Point2 Arc2::pointAt(math::Scalar t) const
{
    detail::validateParameter(t);
    if (t < 0 || t > 1)
        throw std::domain_error{"Arc2 parameter must be in [0,1]"};
    return circle_.pointAt(startAngle_ + t * sweepAngle_);
}

bool Arc2::containsDirection(math::Scalar angle) const noexcept
{
    if (sweepAngle_ > 0)
        return positiveModulo(angle - startAngle_) <= sweepAngle_;
    return positiveModulo(startAngle_ - angle) <= -sweepAngle_;
}

Point2 Arc2::closestPoint(const Point2& point) const
{
    const auto dx = point.x() - center().x();
    const auto dy = point.y() - center().y();
    if ((dx != 0 || dy != 0) && std::isfinite(dx) && std::isfinite(dy))
    {
        const auto angle = std::atan2(dy, dx);
        if (containsDirection(angle)) return circle_.pointAt(angle);
    }

    const auto start = startPoint();
    const auto end = endPoint();
    return pointDistance(point, start) <= pointDistance(point, end) ? start : end;
}

math::Scalar Arc2::distance(const Point2& point) const
{
    return pointDistance(point, closestPoint(point));
}

bool Arc2::contains(const Point2& point, math::Scalar tolerance) const
{
    detail::validateTolerance(tolerance);
    return distance(point) <= tolerance;
}

}
