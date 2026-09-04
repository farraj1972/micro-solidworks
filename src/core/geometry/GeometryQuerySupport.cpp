#include "core/geometry/GeometryQuerySupport.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <stdexcept>

namespace microsw::geometry::detail
{
namespace
{
// A binary-scaled intermediate retains finite differences beyond Scalar's
// exponent range. Each component has its own exponent, so a huge tangential
// offset does not erase a tiny perpendicular component. No wider native
// floating-point type is assumed (long double is double on MSVC).
struct Component
{
    math::Scalar fraction{};
    int exponent{};

    Component() = default;

    explicit Component(math::Scalar value)
    {
        fraction = std::frexp(value, &exponent);
    }

    Component(math::Scalar value, int power)
    {
        fraction = std::frexp(value, &exponent);
        exponent += power;
    }
};

Component add(Component a, Component b)
{
    if (a.fraction == 0) return b;
    if (b.fraction == 0) return a;
    const int power = std::max(a.exponent, b.exponent);
    return Component{std::ldexp(a.fraction, a.exponent - power)
        + std::ldexp(b.fraction, b.exponent - power), power};
}

Component negate(Component a)
{
    a.fraction = -a.fraction;
    return a;
}

Component multiply(Component a, Component b)
{
    return Component{a.fraction * b.fraction, a.exponent + b.exponent};
}

bool within(Component value, math::Scalar tolerance)
{
    if (value.fraction == 0) return true;
    const Component limit{tolerance};
    if (limit.fraction == 0) return false;
    if (value.exponent != limit.exponent) return value.exponent < limit.exponent;
    return std::abs(value.fraction) <= limit.fraction;
}

using Components = std::array<Component, 3>;

Components components(const math::Vector3& vector)
{
    return {Component{vector.x()}, Component{vector.y()}, Component{vector.z()}};
}

Components offset(const Point3& point, const Point3& origin)
{
    return {add(Component{point.x()}, Component{-origin.x()}),
            add(Component{point.y()}, Component{-origin.y()}),
            add(Component{point.z()}, Component{-origin.z()})};
}

Component inner(const Components& a, const Components& b)
{
    Components terms{multiply(a[0], b[0]), multiply(a[1], b[1]), multiply(a[2], b[2])};
    // Combine the largest terms first, allowing cancellation before a much
    // smaller remaining component is added.
    std::sort(terms.begin(), terms.end(), [](Component x, Component y) {
        if (x.fraction == 0) return false;
        if (y.fraction == 0) return true;
        return x.exponent > y.exponent;
    });
    return add(add(terms[0], terms[1]), terms[2]);
}

Components outer(const Components& a, const Components& b)
{
    return {add(multiply(a[1], b[2]), negate(multiply(a[2], b[1]))),
            add(multiply(a[2], b[0]), negate(multiply(a[0], b[2]))),
            add(multiply(a[0], b[1]), negate(multiply(a[1], b[0])))};
}

Component norm(const Components& values)
{
    int power = -10000;
    for (const auto value : values)
        if (value.fraction != 0) power = std::max(power, value.exponent);
    if (power == -10000) return Component{};
    return Component{std::hypot(
        std::ldexp(values[0].fraction, values[0].exponent - power),
        std::ldexp(values[1].fraction, values[1].exponent - power),
        std::ldexp(values[2].fraction, values[2].exponent - power)), power};
}

Components unit(const Components& values)
{
    const auto magnitude = norm(values);
    Components result{};
    for (std::size_t i = 0; i < 3; ++i)
        result[i] = Component{values[i].fraction / magnitude.fraction,
                              values[i].exponent - magnitude.exponent};
    return result;
}

bool forward(const Components& displacement, const Components& direction,
             math::Scalar tolerance)
{
    const auto along = inner(displacement, direction);
    return along.fraction >= 0 || within(along, tolerance);
}
}

void validateTolerance(math::Scalar tolerance)
{
    if (!std::isfinite(tolerance) || tolerance < 0)
        throw std::invalid_argument{"Query tolerance must be finite and non-negative"};
}

void validateParameter(math::Scalar t)
{
    if (!std::isfinite(t))
        throw std::invalid_argument{"Query parameter must be finite"};
}

bool onSupport(const Point3& origin, const math::Vector3& direction,
               const Point3& point, math::Scalar tolerance)
{
    return within(norm(outer(offset(point, origin), components(direction))), tolerance);
}

bool inForwardHalfSpace(const Point3& origin, const math::Vector3& direction,
                        const Point3& point, math::Scalar tolerance)
{
    return forward(offset(point, origin), components(direction), tolerance);
}

bool onPlane(const Point3& origin, const math::Vector3& normal,
             const Point3& point, math::Scalar tolerance)
{
    return within(inner(offset(point, origin), components(normal)), tolerance);
}

bool inSegment(const Point3& a, const Point3& b, const Point3& point,
               math::Scalar tolerance)
{
    const auto displacement = offset(b, a);
    if (within(norm(displacement), tolerance))
        return areCoincident(a, point, tolerance);
    const auto direction = unit(displacement);
    return within(norm(outer(offset(point, a), direction)), tolerance)
        && forward(offset(point, a), direction, tolerance)
        && forward(offset(b, point), direction, tolerance);
}

bool parallel(const math::Vector3& a, const math::Vector3& b, math::Scalar tolerance)
{
    return within(norm(outer(components(a), components(b))), tolerance);
}

bool perpendicular(const math::Vector3& a, const math::Vector3& b, math::Scalar tolerance)
{
    return within(inner(components(a), components(b)), tolerance);
}
}
