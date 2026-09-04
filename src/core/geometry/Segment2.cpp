#include "core/geometry/Segment2.h"

#include <algorithm>
#include <cmath>
#include <numeric>
#include <stdexcept>

namespace microsw::geometry
{
namespace
{

math::Scalar checkedResult(math::Scalar value)
{
    if (!std::isfinite(value))
        throw std::overflow_error{"Segment2 length result is not finite"};
    return value;
}

}

math::Scalar Segment2::length() const
{
    const auto displacement = b_ - a_;
    return checkedResult(std::hypot(displacement.x(), displacement.y()));
}

math::Scalar Segment2::squaredLength() const
{
    const auto magnitude = length();
    return checkedResult(magnitude * magnitude);
}

Point2 Segment2::midpoint() const
{
    // Coordinate-wise affine midpoint avoids both A+B and B-A overflow.
    // For finite endpoints it lies between them and is always representable.
    return {std::midpoint(a_.x(), b_.x()), std::midpoint(a_.y(), b_.y())};
}

bool Segment2::isDegenerate(math::Scalar tolerance) const
{
    return areCoincident(a_, b_, tolerance);
}

math::Vector2 Segment2::direction() const
{
    if (isDegenerate())
        throw std::domain_error{"A degenerate Segment2 has no direction"};

    auto dx = b_.x() - a_.x();
    auto dy = b_.y() - a_.y();
    if (!std::isfinite(dx) || !std::isfinite(dy))
    {
        // Only an overflowing difference needs endpoint halving. Apply the
        // same factor to every component to preserve the displacement ratio.
        dx = b_.x() * 0.5 - a_.x() * 0.5;
        dy = b_.y() * 0.5 - a_.y() * 0.5;
    }
    // Normalize scaled components: neither squaring nor the norm can overflow.
    const auto scale = std::max({std::abs(dx), std::abs(dy)});
    dx /= scale;
    dy /= scale;
    const auto magnitude = std::hypot(dx, dy);
    return {dx / magnitude, dy / magnitude};
}

}
