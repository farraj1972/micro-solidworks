#include "core/geometry/Segment3.h"

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
        throw std::overflow_error{"Segment3 length result is not finite"};
    return value;
}

}

math::Scalar Segment3::length() const
{
    const auto displacement = b_ - a_;
    return checkedResult(std::hypot(displacement.x(), displacement.y(), displacement.z()));
}

math::Scalar Segment3::squaredLength() const
{
    const auto magnitude = length();
    return checkedResult(magnitude * magnitude);
}

Point3 Segment3::midpoint() const
{
    // Coordinate-wise affine midpoint avoids both A+B and B-A overflow.
    // For finite endpoints it lies between them and is always representable.
    return {std::midpoint(a_.x(), b_.x()), std::midpoint(a_.y(), b_.y()), std::midpoint(a_.z(), b_.z())};
}

bool Segment3::isDegenerate(math::Scalar tolerance) const
{
    return areCoincident(a_, b_, tolerance);
}

math::Vector3 Segment3::direction() const
{
    if (isDegenerate())
        throw std::domain_error{"A degenerate Segment3 has no direction"};

    auto dx = b_.x() - a_.x();
    auto dy = b_.y() - a_.y();
    auto dz = b_.z() - a_.z();
    if (!std::isfinite(dx) || !std::isfinite(dy) || !std::isfinite(dz))
    {
        // Only an overflowing difference needs endpoint halving. Apply the
        // same factor to every component to preserve the displacement ratio.
        dx = b_.x() * 0.5 - a_.x() * 0.5;
        dy = b_.y() * 0.5 - a_.y() * 0.5;
        dz = b_.z() * 0.5 - a_.z() * 0.5;
    }
    // Normalize scaled components: neither squaring nor the norm can overflow.
    const auto scale = std::max({std::abs(dx), std::abs(dy), std::abs(dz)});
    dx /= scale;
    dy /= scale;
    dz /= scale;
    const auto magnitude = std::hypot(dx, dy, dz);
    return {dx / magnitude, dy / magnitude, dz / magnitude};
}

}
