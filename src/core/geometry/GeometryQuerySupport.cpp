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

math::Scalar scalarResult(Component value)
{
    const auto result = std::ldexp(value.fraction, value.exponent);
    if (!std::isfinite(result))
        throw std::overflow_error{"Geometric metric result is not representable"};
    return result;
}

Component projectionSum(std::array<Component, 7> terms)
{
    std::sort(terms.begin(), terms.end(), [](Component a, Component b) {
        if (a.fraction == 0) return false;
        if (b.fraction == 0) return true;
        return a.exponent > b.exponent;
    });
    Component result{};
    for (const auto term : terms) result = add(result, term);
    return result;
}

Component projectionNumerator(const Point3& origin, const Components& vector,
                              const Point3& point)
{
    const Components p{Component{point.x()}, Component{point.y()}, Component{point.z()}};
    const Components o{Component{origin.x()}, Component{origin.y()}, Component{origin.z()}};
    std::array<Component, 7> terms{};
    for (std::size_t i = 0; i < 3; ++i)
    {
        terms[2 * i] = multiply(p[i], vector[i]);
        terms[2 * i + 1] = negate(multiply(o[i], vector[i]));
    }
    // Keep origin contributions until after cancellation between query axes.
    return projectionSum(terms);
}

Point3 projected(const Point3& origin, const Components& vector,
                 const Point3& point, bool ontoPlane = false)
{
    const Components p{Component{point.x()}, Component{point.y()}, Component{point.z()}};
    const Components o{Component{origin.x()}, Component{origin.y()}, Component{origin.z()}};
    const auto squaredLength = inner(vector, vector);
    std::array<math::Scalar, 3> result{};
    for (std::size_t i = 0; i < 3; ++i)
    {
        // Expand O + D*dot(P-O,D)/dot(D,D) BEFORE any subtraction.
        // The diagonal origin coefficient is the sum of the other D[j]^2,
        // not 1-D[i]^2. For a plane, exchange the origin/query coefficients.
        // Thus neither a small origin nor a small query coordinate is first
        // erased by forming P-O or P-residual. Scaled products also retain
        // tiny parameters and intermediates outside Scalar's exponent range.
        std::array<Component, 7> terms{};
        terms[0] = multiply(ontoPlane ? o[i] : p[i], multiply(vector[i], vector[i]));
        std::size_t next = 1;
        for (std::size_t j = 0; j < 3; ++j)
        {
            if (j == i) continue;
            const auto crossCoefficient = multiply(vector[i], vector[j]);
            terms[next++] = multiply(ontoPlane ? p[i] : o[i], multiply(vector[j], vector[j]));
            terms[next++] = multiply(ontoPlane ? o[j] : p[j], crossCoefficient);
            terms[next++] = negate(multiply(ontoPlane ? p[j] : o[j], crossCoefficient));
        }
        const auto numerator = projectionSum(terms);
        result[i] = scalarResult(Component{numerator.fraction / squaredLength.fraction,
                                          numerator.exponent - squaredLength.exponent});
    }
    return {result[0], result[1], result[2]};
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

Point3 nearestSegment(const Point3& a, const Point3& b, const Point3& point)
{
    const auto displacement = offset(b, a);
    if (within(norm(displacement), defaultGeometricTolerance))
        return a;
    const auto numerator = projectionNumerator(a, displacement, point);
    if (numerator.fraction <= 0) return a;
    const auto denominator = inner(displacement, displacement);
    const Component t{numerator.fraction / denominator.fraction,
                      numerator.exponent - denominator.exponent};
    if (!within(t, 1.0)) return b;
    if (t.exponent == 1 && t.fraction == 0.5) return b;
    return projected(a, displacement, point);
}

Point3 nearestLine(const Point3& origin, const math::Vector3& direction,
                   const Point3& point, bool forwardOnly)
{
    const auto vector = components(direction);
    const auto t = projectionNumerator(origin, vector, point);
    if (forwardOnly && t.fraction < 0) return origin;
    return projected(origin, vector, point);
}

Point3 nearestPlane(const Point3& origin, const math::Vector3& normal, const Point3& point)
{
    const auto vector = components(normal);
    return projected(origin, vector, point, true);
}

math::Scalar pointMetric(const Point3& a, const Point3& b)
{
    return scalarResult(norm(offset(a, b)));
}

math::Scalar lineMetric(const Point3& origin, const math::Vector3& direction,
                        const Point3& point, bool forwardOnly)
{
    const auto displacement = offset(point, origin);
    const auto vector = components(direction);
    if (forwardOnly && inner(displacement, vector).fraction < 0)
        return scalarResult(norm(displacement));
    // Evaluate the perpendicular residual directly: a distant projection
    // need not be materialized merely to obtain a small representable distance.
    return scalarResult(norm(outer(displacement, vector)));
}

math::Scalar planeSignedMetric(const Point3& origin, const math::Vector3& normal,
                               const Point3& point)
{
    return scalarResult(inner(offset(point, origin), components(normal)));
}
}
