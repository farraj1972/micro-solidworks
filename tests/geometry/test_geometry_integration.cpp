#include "core/geometry/Segment2.h"
#include "core/geometry/Segment3.h"
#include "core/geometry/Line2.h"
#include "core/geometry/Line3.h"
#include "core/geometry/Ray2.h"
#include "core/geometry/Ray3.h"
#include "core/geometry/Plane.h"
#include "core/math/Tolerance.h"

#include <gtest/gtest.h>

#include <cmath>
#include <limits>
#include <stdexcept>
#include <type_traits>

namespace
{
using namespace microsw::geometry;
using microsw::math::Scalar;
using microsw::math::Vector2;
using microsw::math::Vector3;
using microsw::math::dot;
using microsw::math::cross;
constexpr auto numericTolerance = microsw::math::defaultAbsoluteTolerance;

// These local oracles use ordinary, representable displacements only.
Scalar separation(const Point2& a, const Point2& b)
{
    return std::hypot(a.x() - b.x(), a.y() - b.y());
}

Scalar separation(const Point3& a, const Point3& b)
{
    return std::hypot(a.x() - b.x(), a.y() - b.y(), a.z() - b.z());
}

TEST(GeometryIntegration, PointVectorArithmeticPreservesSemanticBoundary)
{
    const Point2 a2{2, -3}, b2{5, 1};
    const Point3 a3{2, -3, 7}, b3{5, 1, -5};
    static_assert(std::is_same_v<decltype(b2 - a2), Vector2>);
    static_assert(std::is_same_v<decltype(a2 + (b2 - a2)), Point2>);
    static_assert(std::is_same_v<decltype(b3 - a3), Vector3>);
    static_assert(std::is_same_v<decltype(a3 + (b3 - a3)), Point3>);
    EXPECT_TRUE(areCoincident(a2 + (b2 - a2), b2, 0));
    EXPECT_TRUE(areCoincident(a3 + (b3 - a3), b3, 0));
    EXPECT_TRUE(areCoincident(Segment2{a2, b2}.midpoint(), a2 + 0.5 * (b2 - a2), 0));
    EXPECT_TRUE(areCoincident(Segment3{a3, b3}.midpoint(), a3 + 0.5 * (b3 - a3), 0));
}

TEST(GeometryIntegration, GeometricAndNumericPoliciesRemainSeparate)
{
    EXPECT_EQ(defaultGeometricTolerance, 1e-9);
    EXPECT_EQ(microsw::math::defaultAbsoluteTolerance, 1e-12);
    EXPECT_EQ(microsw::math::defaultRelativeTolerance, 1e-12);
    EXPECT_TRUE(areCoincident(Point3{}, Point3{5e-10, 0, 0}));
    EXPECT_FALSE(microsw::math::isNearlyZero(5e-10));
}

TEST(GeometryIntegration, Segment2ParameterProjectionAndDomain)
{
    // Axis-aligned dyadic data makes zero-residual assertions exact.
    const Segment2 segment{Point2{2, -3}, Point2{10, -3}};
    const Line2 support{segment.a(), segment.direction()};
    const Ray2 forward{segment.a(), segment.direction()};
    for (Scalar t : {0.0, 0.25, 0.5, 0.75, 1.0})
    {
        SCOPED_TRACE(t);
        const auto point = segment.pointAt(t);
        EXPECT_TRUE(segment.contains(point, 0));
        EXPECT_EQ(distance(segment, point), 0);
        EXPECT_TRUE(areCoincident(closestPoint(segment, point), point, 0));
        EXPECT_TRUE(areCoincident(support.pointAt(t * segment.length()), point, 0));
        EXPECT_TRUE(forward.contains(point, 0));
        const auto query = point + Vector2{0, 3};
        const auto closest = closestPoint(segment, query);
        EXPECT_TRUE(segment.contains(closest, numericTolerance));
        EXPECT_TRUE(areCoincident(closest, point, numericTolerance));
        EXPECT_NEAR(distance(segment, query), separation(query, closest), numericTolerance);
    }
    for (Scalar t : {-2.0, 12.0})
    {
        const auto query = support.pointAt(t) + Vector2{0, 3};
        const auto expected = t < 0 ? segment.a() : segment.b();
        EXPECT_FALSE(segment.contains(query, 0));
        EXPECT_TRUE(areCoincident(closestPoint(segment, query), expected, 0));
        EXPECT_NEAR(distance(segment, query), separation(query, expected), numericTolerance);
    }
}

TEST(GeometryIntegration, Segment2DegeneracyKeepsMetricsButRejectsDirection)
{
    const auto a = Point2{2, -3};
    const auto query = a + Vector2{3, 4};
    const Segment2 zero{a, a}, regular{a, a + Vector2{1, 0}};
    EXPECT_TRUE(zero.isDegenerate());
    EXPECT_TRUE(zero.contains(a, 0));
    EXPECT_TRUE(areCoincident(closestPoint(zero, query), a, 0));
    EXPECT_DOUBLE_EQ(distance(zero, query), separation(query, a));
    EXPECT_THROW((void)zero.direction(), std::domain_error);
    EXPECT_THROW((void)isParallel(zero, regular), std::domain_error);
    EXPECT_THROW((void)isParallel(regular, zero), std::domain_error);
    EXPECT_THROW((void)isPerpendicular(zero, regular), std::domain_error);
    EXPECT_THROW((void)isPerpendicular(regular, zero), std::domain_error);
    for (Scalar t : {0.0, 0.25, 0.5, 0.75, 1.0})
        EXPECT_TRUE(areCoincident(zero.pointAt(t), a, 0));
}

TEST(GeometryIntegration, Line2ParameterAndProjectionOrthogonality)
{
    const Line2 axis{Point2{2, -3}, Vector2{0, 2}};
    for (Scalar t : {-8.0, -0.5, 0.0, 0.25, 7.0})
    {
        const auto point = axis.pointAt(t);
        EXPECT_TRUE(axis.contains(point, 0));
        EXPECT_EQ(distance(axis, point), 0);
        EXPECT_TRUE(areCoincident(closestPoint(axis, point), point, 0));
    }
    const Line2 diagonal{Point2{2, -3}, Vector2{3, 4}};
    for (const auto query : {Point2{1, 7}, Point2{-4, 2}, Point2{9, -1}})
    {
        const auto closest = closestPoint(diagonal, query);
        const auto residual = query - closest;
        EXPECT_TRUE(diagonal.contains(closest, numericTolerance));
        EXPECT_NEAR(dot(residual, diagonal.direction()), 0, numericTolerance);
        EXPECT_NEAR(distance(diagonal, query), separation(query, closest), numericTolerance);
        EXPECT_NEAR(distance(diagonal, closest), 0, numericTolerance);
    }
}

TEST(GeometryIntegration, Line2OriginTranslationPreservesQueriesAndMetrics)
{
    const Line2 first{Point2{2, -3}, Vector2{3, 4}};
    const Line2 shifted{first.origin() + 10 * first.direction(), first.direction()};
    for (const auto point : {first.pointAt(-2), first.pointAt(3), Point2{9, -2}, Point2{-5, 1}})
    {
        EXPECT_EQ(first.contains(point), shifted.contains(point));
        EXPECT_NEAR(distance(first, point), distance(shifted, point), numericTolerance);
        EXPECT_TRUE(areCoincident(closestPoint(first, point), closestPoint(shifted, point), numericTolerance));
    }
}

TEST(GeometryIntegration, Ray2ForwardAndBehindDomainAgreeWithSupport)
{
    const Ray2 ray{Point2{0, 3}, Vector2{2, 0}};
    const Line2 support{ray.origin(), ray.direction()};
    for (Scalar t : {0.0, 0.25, 1.0, 7.0})
    {
        const auto point = ray.pointAt(t);
        EXPECT_TRUE(ray.contains(point, 0));
        EXPECT_EQ(distance(ray, point), 0);
        EXPECT_TRUE(areCoincident(closestPoint(ray, point), point, 0));
        const auto query = point + Vector2{0, 3};
        EXPECT_TRUE(areCoincident(closestPoint(ray, query), closestPoint(support, query), numericTolerance));
        EXPECT_NEAR(distance(ray, query), separation(query, closestPoint(ray, query)), numericTolerance);
    }
    for (Scalar t : {-4.0, -5e-10})
    {
        const auto point = support.pointAt(t);
        EXPECT_TRUE(support.contains(point, 0));
        EXPECT_FALSE(ray.contains(point, 0));
        EXPECT_EQ(ray.contains(point), t == -5e-10);
        EXPECT_TRUE(areCoincident(closestPoint(ray, point), ray.origin(), 0));
        EXPECT_DOUBLE_EQ(distance(ray, point), -t);
        EXPECT_DOUBLE_EQ(distance(ray, point), separation(point, ray.origin()));
        EXPECT_THROW((void)ray.pointAt(t), std::domain_error);
    }
}

TEST(GeometryIntegration, PredicatesDoNotSnapMetrics2D)
{
    const Segment2 segment{Point2{0, 0}, Point2{4, 0}};
    const Line2 line{segment.a(), segment.direction()};
    const Ray2 ray{segment.a(), segment.direction()};
    for (Scalar residual : {0.0, 5e-10, 2e-9})
    {
        const auto query = Point2{2, residual};
        EXPECT_EQ(segment.contains(query), residual <= defaultGeometricTolerance);
        EXPECT_EQ(line.contains(query), residual <= defaultGeometricTolerance);
        EXPECT_EQ(ray.contains(query), residual <= defaultGeometricTolerance);
        EXPECT_EQ(segment.contains(query, 0), residual == 0);
        EXPECT_EQ(line.contains(query, 0), residual == 0);
        EXPECT_EQ(ray.contains(query, 0), residual == 0);
        EXPECT_DOUBLE_EQ(distance(segment, query), residual);
        EXPECT_DOUBLE_EQ(distance(line, query), residual);
        EXPECT_DOUBLE_EQ(distance(ray, query), residual);
        EXPECT_TRUE(areCoincident(closestPoint(segment, query), segment.pointAt(0.5), 0));
        EXPECT_TRUE(areCoincident(closestPoint(line, query), segment.pointAt(0.5), 0));
        EXPECT_TRUE(areCoincident(closestPoint(ray, query), segment.pointAt(0.5), 0));
    }
}

TEST(GeometryIntegration, Segment3ParameterProjectionAndDomain)
{
    // Axis-aligned dyadic data makes zero-residual assertions exact.
    const Segment3 segment{Point3{2, -3, 4}, Point3{10, -3, 4}};
    const Line3 support{segment.a(), segment.direction()};
    const Ray3 forward{segment.a(), segment.direction()};
    for (Scalar t : {0.0, 0.25, 0.5, 0.75, 1.0})
    {
        SCOPED_TRACE(t);
        const auto point = segment.pointAt(t);
        EXPECT_TRUE(segment.contains(point, 0));
        EXPECT_EQ(distance(segment, point), 0);
        EXPECT_TRUE(areCoincident(closestPoint(segment, point), point, 0));
        EXPECT_TRUE(areCoincident(support.pointAt(t * segment.length()), point, 0));
        EXPECT_TRUE(forward.contains(point, 0));
        const auto query = point + Vector3{0, 3, 4};
        const auto closest = closestPoint(segment, query);
        EXPECT_TRUE(segment.contains(closest, numericTolerance));
        EXPECT_TRUE(areCoincident(closest, point, numericTolerance));
        EXPECT_NEAR(distance(segment, query), separation(query, closest), numericTolerance);
    }
    for (Scalar t : {-2.0, 12.0})
    {
        const auto query = support.pointAt(t) + Vector3{0, 3, 4};
        const auto expected = t < 0 ? segment.a() : segment.b();
        EXPECT_FALSE(segment.contains(query, 0));
        EXPECT_TRUE(areCoincident(closestPoint(segment, query), expected, 0));
        EXPECT_NEAR(distance(segment, query), separation(query, expected), numericTolerance);
    }
}

TEST(GeometryIntegration, Segment3DegeneracyKeepsMetricsButRejectsDirection)
{
    const auto a = Point3{2, -3, 4};
    const auto query = a + Vector3{3, 4, 12};
    const Segment3 zero{a, a}, regular{a, a + Vector3{1, 0, 0}};
    EXPECT_TRUE(zero.isDegenerate());
    EXPECT_TRUE(zero.contains(a, 0));
    EXPECT_TRUE(areCoincident(closestPoint(zero, query), a, 0));
    EXPECT_DOUBLE_EQ(distance(zero, query), separation(query, a));
    EXPECT_THROW((void)zero.direction(), std::domain_error);
    EXPECT_THROW((void)isParallel(zero, regular), std::domain_error);
    EXPECT_THROW((void)isParallel(regular, zero), std::domain_error);
    EXPECT_THROW((void)isPerpendicular(zero, regular), std::domain_error);
    EXPECT_THROW((void)isPerpendicular(regular, zero), std::domain_error);
    for (Scalar t : {0.0, 0.25, 0.5, 0.75, 1.0})
        EXPECT_TRUE(areCoincident(zero.pointAt(t), a, 0));
}

TEST(GeometryIntegration, Line3ParameterAndProjectionOrthogonality)
{
    const Line3 axis{Point3{2, -3, 4}, Vector3{0, 2, 0}};
    for (Scalar t : {-8.0, -0.5, 0.0, 0.25, 7.0})
    {
        const auto point = axis.pointAt(t);
        EXPECT_TRUE(axis.contains(point, 0));
        EXPECT_EQ(distance(axis, point), 0);
        EXPECT_TRUE(areCoincident(closestPoint(axis, point), point, 0));
    }
    const Line3 diagonal{Point3{2, -3, 4}, Vector3{3, 4, 12}};
    for (const auto query : {Point3{1, 7, -2}, Point3{-4, 2, 8}, Point3{9, -1, 3}})
    {
        const auto closest = closestPoint(diagonal, query);
        const auto residual = query - closest;
        EXPECT_TRUE(diagonal.contains(closest, numericTolerance));
        EXPECT_NEAR(dot(residual, diagonal.direction()), 0, numericTolerance);
        EXPECT_NEAR(distance(diagonal, query), separation(query, closest), numericTolerance);
        EXPECT_NEAR(distance(diagonal, closest), 0, numericTolerance);
    }
}

TEST(GeometryIntegration, Line3OriginTranslationPreservesQueriesAndMetrics)
{
    const Line3 first{Point3{2, -3, 4}, Vector3{3, 4, 12}};
    const Line3 shifted{first.origin() + 10 * first.direction(), first.direction()};
    for (const auto point : {first.pointAt(-2), first.pointAt(3), Point3{9, -2, 1}, Point3{-5, 1, 9}})
    {
        EXPECT_EQ(first.contains(point), shifted.contains(point));
        EXPECT_NEAR(distance(first, point), distance(shifted, point), numericTolerance);
        EXPECT_TRUE(areCoincident(closestPoint(first, point), closestPoint(shifted, point), numericTolerance));
    }
}

TEST(GeometryIntegration, Ray3ForwardAndBehindDomainAgreeWithSupport)
{
    const Ray3 ray{Point3{0, 3, 4}, Vector3{2, 0, 0}};
    const Line3 support{ray.origin(), ray.direction()};
    for (Scalar t : {0.0, 0.25, 1.0, 7.0})
    {
        const auto point = ray.pointAt(t);
        EXPECT_TRUE(ray.contains(point, 0));
        EXPECT_EQ(distance(ray, point), 0);
        EXPECT_TRUE(areCoincident(closestPoint(ray, point), point, 0));
        const auto query = point + Vector3{0, 3, 4};
        EXPECT_TRUE(areCoincident(closestPoint(ray, query), closestPoint(support, query), numericTolerance));
        EXPECT_NEAR(distance(ray, query), separation(query, closestPoint(ray, query)), numericTolerance);
    }
    for (Scalar t : {-4.0, -5e-10})
    {
        const auto point = support.pointAt(t);
        EXPECT_TRUE(support.contains(point, 0));
        EXPECT_FALSE(ray.contains(point, 0));
        EXPECT_EQ(ray.contains(point), t == -5e-10);
        EXPECT_TRUE(areCoincident(closestPoint(ray, point), ray.origin(), 0));
        EXPECT_DOUBLE_EQ(distance(ray, point), -t);
        EXPECT_DOUBLE_EQ(distance(ray, point), separation(point, ray.origin()));
        EXPECT_THROW((void)ray.pointAt(t), std::domain_error);
    }
}

TEST(GeometryIntegration, PredicatesDoNotSnapMetrics3D)
{
    const Segment3 segment{Point3{0, 0, 0}, Point3{4, 0, 0}};
    const Line3 line{segment.a(), segment.direction()};
    const Ray3 ray{segment.a(), segment.direction()};
    for (Scalar residual : {0.0, 5e-10, 2e-9})
    {
        const auto query = Point3{2, residual, 0};
        EXPECT_EQ(segment.contains(query), residual <= defaultGeometricTolerance);
        EXPECT_EQ(line.contains(query), residual <= defaultGeometricTolerance);
        EXPECT_EQ(ray.contains(query), residual <= defaultGeometricTolerance);
        EXPECT_EQ(segment.contains(query, 0), residual == 0);
        EXPECT_EQ(line.contains(query, 0), residual == 0);
        EXPECT_EQ(ray.contains(query, 0), residual == 0);
        EXPECT_DOUBLE_EQ(distance(segment, query), residual);
        EXPECT_DOUBLE_EQ(distance(line, query), residual);
        EXPECT_DOUBLE_EQ(distance(ray, query), residual);
        EXPECT_TRUE(areCoincident(closestPoint(segment, query), segment.pointAt(0.5), 0));
        EXPECT_TRUE(areCoincident(closestPoint(line, query), segment.pointAt(0.5), 0));
        EXPECT_TRUE(areCoincident(closestPoint(ray, query), segment.pointAt(0.5), 0));
    }
}

TEST(GeometryIntegration, PlaneProjectionOrientationAndTangentOriginTranslation)
{
    const Point3 origin{2, -3, 4};
    const Vector3 normal{3, 4, 12}, tangent{4, -3, 0};
    const Plane plane{origin, normal}, opposite{origin, -normal};
    const Plane shifted{origin + tangent, normal};
    EXPECT_NEAR(dot(tangent, plane.normal()), 0, numericTolerance);
    for (const auto point : {origin, origin + tangent, Point3{7, 2, -1}, Point3{-5, 9, 3}})
    {
        const auto closest = closestPoint(plane, point);
        EXPECT_TRUE(plane.contains(closest, numericTolerance));
        EXPECT_EQ(distance(plane, point), std::abs(signedDistance(plane, point)));
        EXPECT_NEAR(cross(point - closest, plane.normal()).length(), 0, numericTolerance);
        EXPECT_NEAR(distance(plane, point), separation(point, closest), numericTolerance);
        EXPECT_NEAR(distance(plane, closest), 0, numericTolerance);
        EXPECT_NEAR(signedDistance(plane, point), -signedDistance(opposite, point), numericTolerance);
        EXPECT_NEAR(distance(plane, point), distance(opposite, point), numericTolerance);
        EXPECT_TRUE(areCoincident(closest, closestPoint(opposite, point), numericTolerance));
        EXPECT_EQ(plane.contains(point), shifted.contains(point));
        EXPECT_NEAR(distance(plane, point), distance(shifted, point), numericTolerance);
        EXPECT_NEAR(signedDistance(plane, point), signedDistance(shifted, point), numericTolerance);
        EXPECT_TRUE(areCoincident(closest, closestPoint(shifted, point), numericTolerance));
    }
}

TEST(GeometryIntegration, PlanePredicateMetricAndZeroTolerance)
{
    const Plane plane{Point3{3, -2, 0}, Vector3{0, 0, -2}};
    for (Scalar height : {0.0, 5e-10, -5e-10, 2e-9})
    {
        const Point3 point{7, 4, height};
        EXPECT_EQ(plane.contains(point), std::abs(height) <= defaultGeometricTolerance);
        EXPECT_EQ(plane.contains(point, 0), height == 0);
        EXPECT_DOUBLE_EQ(signedDistance(plane, point), -height);
        EXPECT_DOUBLE_EQ(distance(plane, point), std::abs(height));
        const auto closest = closestPoint(plane, point);
        EXPECT_TRUE(areCoincident(closest, Point3{7, 4, 0}, 0));
        EXPECT_TRUE(plane.contains(closest, 0));
        EXPECT_EQ(distance(plane, closest), 0);
    }
}

TEST(GeometryIntegration, NormalizationAndRelations2D)
{
    const auto origin = Point2{2, -3};
    const auto direction = Vector2{3, 4};
    const Line2 line{origin, direction};
    const Ray2 ray{origin, direction};
    const Segment2 segment{origin, origin + direction};
    for (Scalar scale : {1.0, 8.0, -2.0})
    {
        const Line2 otherLine{origin, scale * direction};
        const Ray2 otherRay{origin, scale * direction};
        const Segment2 otherSegment{origin, origin + scale * direction};
        EXPECT_NEAR(otherLine.direction().length(), 1, numericTolerance);
        EXPECT_NEAR(otherRay.direction().length(), 1, numericTolerance);
        EXPECT_NEAR(otherSegment.direction().length(), 1, numericTolerance);
        EXPECT_TRUE(isParallel(line, otherLine));
        EXPECT_TRUE(isParallel(ray, otherRay));
        EXPECT_TRUE(isParallel(segment, otherSegment));
        const auto point = otherLine.pointAt(2);
        EXPECT_TRUE(line.contains(point));
        EXPECT_NEAR(distance(line, point), 0, numericTolerance);
        EXPECT_EQ(ray.contains(point), scale > 0);
    }
}

TEST(GeometryIntegration, RelationToleranceMatchesUnitResiduals2D)
{
    const auto origin = Point2{0, 0};
    const Line2 line{origin, Vector2{1, 0}};
    const Ray2 ray{origin, Vector2{1, 0}};
    const Segment2 segment{origin, Point2{1, 0}};
    for (Scalar residual : {5e-10, 2e-9})
    {
        const auto nearParallel = Vector2{1, residual};
        const auto nearPerpendicular = Vector2{residual, 1};
        const bool accepted = residual <= defaultGeometricTolerance;
        const Line2 tilted{origin, nearParallel}, orthogonal{origin, nearPerpendicular};
        EXPECT_EQ(isParallel(line, tilted), accepted);
        EXPECT_FALSE(isParallel(line, tilted, 0));
        EXPECT_EQ(isPerpendicular(line, orthogonal), accepted);
        EXPECT_FALSE(isPerpendicular(line, orthogonal, 0));
        EXPECT_EQ(isParallel(ray, Ray2{origin, nearParallel}), accepted);
        EXPECT_EQ(isPerpendicular(ray, Ray2{origin, nearPerpendicular}), accepted);
        EXPECT_EQ(isParallel(segment, Segment2{origin, origin + nearParallel}), accepted);
        EXPECT_EQ(isPerpendicular(segment, Segment2{origin, origin + nearPerpendicular}), accepted);
        // A unit step turns this angular residual into a length residual.
        const auto point = tilted.pointAt(1);
        EXPECT_EQ(line.contains(point), accepted);
        EXPECT_NEAR(distance(line, point) / residual, 1, numericTolerance);
        EXPECT_NEAR(dot(line.direction(), orthogonal.direction()) / residual, 1, numericTolerance);
    }
}

TEST(GeometryIntegration, ExtremeOffsetsKeepCombinedQueriesFinite2D)
{
    const auto maximum = std::numeric_limits<Scalar>::max();
    const Segment2 segment{Point2{-maximum, 0}, Point2{maximum, 0}};
    const Line2 line{segment.a(), segment.direction()};
    const Ray2 ray{segment.a(), segment.direction()};
    for (Scalar t : {0.0, 0.25, 0.5, 0.75, 1.0})
    {
        const auto point = segment.pointAt(t);
        EXPECT_TRUE(std::isfinite(point.x()));
        EXPECT_TRUE(segment.contains(point, 0));
        EXPECT_TRUE(line.contains(point, 0));
        EXPECT_TRUE(ray.contains(point, 0));
        const auto query = point + Vector2{0, 5e-10};
        EXPECT_TRUE(segment.contains(query));
        EXPECT_TRUE(line.contains(query));
        EXPECT_TRUE(ray.contains(query));
        EXPECT_FALSE(segment.contains(query, 0));
        EXPECT_FALSE(line.contains(query, 0));
        EXPECT_FALSE(ray.contains(query, 0));
        EXPECT_TRUE(areCoincident(closestPoint(segment, query), point, 0));
        EXPECT_TRUE(areCoincident(closestPoint(line, query), point, 0));
        EXPECT_TRUE(areCoincident(closestPoint(ray, query), point, 0));
        EXPECT_DOUBLE_EQ(distance(segment, query), 5e-10);
        EXPECT_DOUBLE_EQ(distance(line, query), 5e-10);
        EXPECT_DOUBLE_EQ(distance(ray, query), 5e-10);
    }
}

TEST(GeometryIntegration, OverflowDistanceDoesNotInvalidateFiniteProjection2D)
{
    const auto maximum = std::numeric_limits<Scalar>::max();
    const auto origin = Point2{0, -maximum};
    const auto query = Point2{0, maximum};
    const Segment2 segment{origin, origin + Vector2{1, 0}};
    const Line2 line{origin, segment.direction()};
    const Ray2 ray{origin, segment.direction()};
    EXPECT_FALSE(segment.contains(query));
    EXPECT_FALSE(line.contains(query));
    EXPECT_FALSE(ray.contains(query));
    EXPECT_TRUE(areCoincident(closestPoint(segment, query), origin, 0));
    EXPECT_TRUE(areCoincident(closestPoint(line, query), origin, 0));
    EXPECT_TRUE(areCoincident(closestPoint(ray, query), origin, 0));
    EXPECT_THROW((void)distance(segment, query), std::overflow_error);
    EXPECT_THROW((void)distance(line, query), std::overflow_error);
    EXPECT_THROW((void)distance(ray, query), std::overflow_error);
    EXPECT_EQ(distance(line, origin), 0);
    EXPECT_EQ(distance(ray, origin), 0);
}

TEST(GeometryIntegration, PointAtOverflowLeavesLineAndRayUsable2D)
{
    const auto maximum = std::numeric_limits<Scalar>::max();
    const auto origin = Point2{maximum, 0};
    const Line2 line{origin, Vector2{1, 0}};
    const Ray2 ray{origin, Vector2{1, 0}};
    EXPECT_THROW((void)line.pointAt(maximum), std::overflow_error);
    EXPECT_THROW((void)ray.pointAt(maximum), std::overflow_error);
    EXPECT_TRUE(line.contains(line.pointAt(0), 0));
    EXPECT_TRUE(ray.contains(ray.pointAt(0), 0));
    EXPECT_TRUE(areCoincident(closestPoint(line, origin), origin, 0));
    EXPECT_TRUE(areCoincident(closestPoint(ray, origin), origin, 0));
    EXPECT_EQ(distance(line, origin), 0);
    EXPECT_EQ(distance(ray, origin), 0);
}

TEST(GeometryIntegration, NormalizationAndRelations3D)
{
    const auto origin = Point3{2, -3, 4};
    const auto direction = Vector3{3, 4, 12};
    const Line3 line{origin, direction};
    const Ray3 ray{origin, direction};
    const Segment3 segment{origin, origin + direction};
    for (Scalar scale : {1.0, 8.0, -2.0})
    {
        const Line3 otherLine{origin, scale * direction};
        const Ray3 otherRay{origin, scale * direction};
        const Segment3 otherSegment{origin, origin + scale * direction};
        EXPECT_NEAR(otherLine.direction().length(), 1, numericTolerance);
        EXPECT_NEAR(otherRay.direction().length(), 1, numericTolerance);
        EXPECT_NEAR(otherSegment.direction().length(), 1, numericTolerance);
        EXPECT_TRUE(isParallel(line, otherLine));
        EXPECT_TRUE(isParallel(ray, otherRay));
        EXPECT_TRUE(isParallel(segment, otherSegment));
        const auto point = otherLine.pointAt(2);
        EXPECT_TRUE(line.contains(point));
        EXPECT_NEAR(distance(line, point), 0, numericTolerance);
        EXPECT_EQ(ray.contains(point), scale > 0);
    }
}

TEST(GeometryIntegration, RelationToleranceMatchesUnitResiduals3D)
{
    const auto origin = Point3{0, 0, 0};
    const Line3 line{origin, Vector3{1, 0, 0}};
    const Ray3 ray{origin, Vector3{1, 0, 0}};
    const Segment3 segment{origin, Point3{1, 0, 0}};
    for (Scalar residual : {5e-10, 2e-9})
    {
        const auto nearParallel = Vector3{1, residual, 0};
        const auto nearPerpendicular = Vector3{residual, 1, 0};
        const bool accepted = residual <= defaultGeometricTolerance;
        const Line3 tilted{origin, nearParallel}, orthogonal{origin, nearPerpendicular};
        EXPECT_EQ(isParallel(line, tilted), accepted);
        EXPECT_FALSE(isParallel(line, tilted, 0));
        EXPECT_EQ(isPerpendicular(line, orthogonal), accepted);
        EXPECT_FALSE(isPerpendicular(line, orthogonal, 0));
        EXPECT_EQ(isParallel(ray, Ray3{origin, nearParallel}), accepted);
        EXPECT_EQ(isPerpendicular(ray, Ray3{origin, nearPerpendicular}), accepted);
        EXPECT_EQ(isParallel(segment, Segment3{origin, origin + nearParallel}), accepted);
        EXPECT_EQ(isPerpendicular(segment, Segment3{origin, origin + nearPerpendicular}), accepted);
        // A unit step turns this angular residual into a length residual.
        const auto point = tilted.pointAt(1);
        EXPECT_EQ(line.contains(point), accepted);
        EXPECT_NEAR(distance(line, point) / residual, 1, numericTolerance);
        EXPECT_NEAR(dot(line.direction(), orthogonal.direction()) / residual, 1, numericTolerance);
    }
}

TEST(GeometryIntegration, ExtremeOffsetsKeepCombinedQueriesFinite3D)
{
    const auto maximum = std::numeric_limits<Scalar>::max();
    const Segment3 segment{Point3{-maximum, 0, 0}, Point3{maximum, 0, 0}};
    const Line3 line{segment.a(), segment.direction()};
    const Ray3 ray{segment.a(), segment.direction()};
    for (Scalar t : {0.0, 0.25, 0.5, 0.75, 1.0})
    {
        const auto point = segment.pointAt(t);
        EXPECT_TRUE(std::isfinite(point.x()));
        EXPECT_TRUE(segment.contains(point, 0));
        EXPECT_TRUE(line.contains(point, 0));
        EXPECT_TRUE(ray.contains(point, 0));
        const auto query = point + Vector3{0, 5e-10, 0};
        EXPECT_TRUE(segment.contains(query));
        EXPECT_TRUE(line.contains(query));
        EXPECT_TRUE(ray.contains(query));
        EXPECT_FALSE(segment.contains(query, 0));
        EXPECT_FALSE(line.contains(query, 0));
        EXPECT_FALSE(ray.contains(query, 0));
        EXPECT_TRUE(areCoincident(closestPoint(segment, query), point, 0));
        EXPECT_TRUE(areCoincident(closestPoint(line, query), point, 0));
        EXPECT_TRUE(areCoincident(closestPoint(ray, query), point, 0));
        EXPECT_DOUBLE_EQ(distance(segment, query), 5e-10);
        EXPECT_DOUBLE_EQ(distance(line, query), 5e-10);
        EXPECT_DOUBLE_EQ(distance(ray, query), 5e-10);
    }
}

TEST(GeometryIntegration, OverflowDistanceDoesNotInvalidateFiniteProjection3D)
{
    const auto maximum = std::numeric_limits<Scalar>::max();
    const auto origin = Point3{0, -maximum, 0};
    const auto query = Point3{0, maximum, 0};
    const Segment3 segment{origin, origin + Vector3{1, 0, 0}};
    const Line3 line{origin, segment.direction()};
    const Ray3 ray{origin, segment.direction()};
    EXPECT_FALSE(segment.contains(query));
    EXPECT_FALSE(line.contains(query));
    EXPECT_FALSE(ray.contains(query));
    EXPECT_TRUE(areCoincident(closestPoint(segment, query), origin, 0));
    EXPECT_TRUE(areCoincident(closestPoint(line, query), origin, 0));
    EXPECT_TRUE(areCoincident(closestPoint(ray, query), origin, 0));
    EXPECT_THROW((void)distance(segment, query), std::overflow_error);
    EXPECT_THROW((void)distance(line, query), std::overflow_error);
    EXPECT_THROW((void)distance(ray, query), std::overflow_error);
    EXPECT_EQ(distance(line, origin), 0);
    EXPECT_EQ(distance(ray, origin), 0);
}

TEST(GeometryIntegration, PointAtOverflowLeavesLineAndRayUsable3D)
{
    const auto maximum = std::numeric_limits<Scalar>::max();
    const auto origin = Point3{maximum, 0, 0};
    const Line3 line{origin, Vector3{1, 0, 0}};
    const Ray3 ray{origin, Vector3{1, 0, 0}};
    EXPECT_THROW((void)line.pointAt(maximum), std::overflow_error);
    EXPECT_THROW((void)ray.pointAt(maximum), std::overflow_error);
    EXPECT_TRUE(line.contains(line.pointAt(0), 0));
    EXPECT_TRUE(ray.contains(ray.pointAt(0), 0));
    EXPECT_TRUE(areCoincident(closestPoint(line, origin), origin, 0));
    EXPECT_TRUE(areCoincident(closestPoint(ray, origin), origin, 0));
    EXPECT_EQ(distance(line, origin), 0);
    EXPECT_EQ(distance(ray, origin), 0);
}

TEST(GeometryIntegration, OrthogonalBasesAgreeAcrossPrimitiveKinds)
{
    const Point3 origin{};
    const Vector3 axes[]{Vector3{1, 0, 0}, Vector3{0, 1, 0}, Vector3{0, 0, 1}};
    for (int i = 0; i < 3; ++i)
    {
        const auto a = axes[i], b = axes[(i + 1) % 3];
        const Line3 line{origin, a}, otherLine{origin, b};
        const Ray3 ray{origin, a}, otherRay{origin, b};
        const Segment3 segment{origin, origin + a}, otherSegment{origin, origin + b};
        const Plane plane{origin, a}, otherPlane{origin, b};
        EXPECT_TRUE(isPerpendicular(line, otherLine, 0));
        EXPECT_TRUE(isPerpendicular(ray, otherRay, 0));
        EXPECT_TRUE(isPerpendicular(segment, otherSegment, 0));
        EXPECT_TRUE(isPerpendicular(plane, otherPlane, 0));
        const auto point = otherLine.pointAt(3);
        EXPECT_EQ(distance(line, point), 3);
        EXPECT_TRUE(plane.contains(point, 0));
        EXPECT_EQ(distance(plane, point), 0);
    }
    const Line2 x{Point2{}, Vector2{1, 0}}, y{Point2{}, Vector2{0, 1}};
    EXPECT_TRUE(isPerpendicular(x, y, 0));
    EXPECT_TRUE(isPerpendicular(Ray2{x.origin(), x.direction()}, Ray2{y.origin(), y.direction()}, 0));
    EXPECT_TRUE(isPerpendicular(Segment2{x.origin(), x.pointAt(1)}, Segment2{y.origin(), y.pointAt(1)}, 0));
}

TEST(GeometryIntegration, PlaneNormalizationAndRelationTolerances)
{
    const Point3 origin{};
    const Plane plane{origin, Vector3{3, 4, 12}};
    for (Scalar scale : {1.0, 8.0, -2.0})
    {
        const Plane other{origin, scale * Vector3{3, 4, 12}};
        EXPECT_TRUE(isParallel(plane, other));
        EXPECT_NEAR(other.normal().length(), 1, numericTolerance);
        EXPECT_NEAR(distance(plane, Point3{1, 2, 3}), distance(other, Point3{1, 2, 3}), numericTolerance);
    }
    const Plane x{origin, Vector3{1, 0, 0}};
    for (Scalar residual : {5e-10, 2e-9})
    {
        const Plane tilted{origin, Vector3{1, residual, 0}};
        const Plane orthogonal{origin, Vector3{residual, 1, 0}};
        EXPECT_EQ(isParallel(x, tilted), residual <= defaultGeometricTolerance);
        EXPECT_EQ(isPerpendicular(x, orthogonal), residual <= defaultGeometricTolerance);
        EXPECT_FALSE(isParallel(x, tilted, 0));
        EXPECT_FALSE(isPerpendicular(x, orthogonal, 0));
        EXPECT_NEAR(signedDistance(orthogonal, Point3{1, 0, 0}) / residual, 1, numericTolerance);
    }
}

TEST(GeometryIntegration, ExtremePlaneTangentTranslationPreservesSmallResidual)
{
    const auto maximum = std::numeric_limits<Scalar>::max();
    const Plane first{Point3{-maximum, 0, -maximum}, Vector3{0, 1, 0}};
    const Plane shifted{Point3{maximum, 0, maximum}, Vector3{0, 2, 0}};
    for (Scalar residual : {0.0, 5e-10, -5e-10, 2e-9})
    {
        const Point3 point{maximum, residual, maximum};
        const auto closest = closestPoint(first, point);
        EXPECT_TRUE(areCoincident(closest, Point3{maximum, 0, maximum}, 0));
        EXPECT_TRUE(first.contains(closest, 0));
        EXPECT_EQ(first.contains(point), std::abs(residual) <= defaultGeometricTolerance);
        EXPECT_EQ(first.contains(point, 0), residual == 0);
        EXPECT_EQ(first.contains(point), shifted.contains(point));
        EXPECT_TRUE(areCoincident(closest, closestPoint(shifted, point), 0));
        EXPECT_DOUBLE_EQ(distance(first, point), std::abs(residual));
        EXPECT_DOUBLE_EQ(signedDistance(first, point), residual);
        EXPECT_DOUBLE_EQ(signedDistance(first, point), signedDistance(shifted, point));
    }
}

TEST(GeometryIntegration, PlaneOverflowDistanceStillAllowsFiniteProjection)
{
    const auto maximum = std::numeric_limits<Scalar>::max();
    const Plane plane{Point3{-maximum, 0, 0}, Vector3{1, 0, 0}};
    const Plane opposite{plane.origin(), -plane.normal()};
    const Point3 query{maximum, 2, 3};
    const auto closest = closestPoint(plane, query);
    EXPECT_TRUE(areCoincident(closest, Point3{-maximum, 2, 3}, 0));
    EXPECT_TRUE(plane.contains(closest, 0));
    EXPECT_FALSE(plane.contains(query));
    EXPECT_EQ(distance(plane, closest), 0);
    EXPECT_TRUE(areCoincident(closestPoint(opposite, query), closest, 0));
    EXPECT_THROW((void)signedDistance(plane, query), std::overflow_error);
    EXPECT_THROW((void)signedDistance(opposite, query), std::overflow_error);
    EXPECT_THROW((void)distance(plane, query), std::overflow_error);
}

TEST(GeometryIntegration, Embedded2DAnd3DQueriesHaveIdenticalMeaning)
{
    const Segment2 segment2{Point2{2, -3}, Point2{5, 1}};
    const Segment3 segment3{Point3{2, -3, 7}, Point3{5, 1, 7}};
    const Line2 line2{segment2.a(), segment2.direction()};
    const Line3 line3{segment3.a(), segment3.direction()};
    const Ray2 ray2{segment2.a(), segment2.direction()};
    const Ray3 ray3{segment3.a(), segment3.direction()};
    for (const auto p : {Point2{-4, -5}, Point2{3, 0}, Point2{8, 6}})
    {
        const Point3 q{p.x(), p.y(), 7};
        EXPECT_EQ(segment2.contains(p), segment3.contains(q));
        EXPECT_EQ(line2.contains(p), line3.contains(q));
        EXPECT_EQ(ray2.contains(p), ray3.contains(q));
        EXPECT_NEAR(distance(segment2, p), distance(segment3, q), numericTolerance);
        EXPECT_NEAR(distance(line2, p), distance(line3, q), numericTolerance);
        EXPECT_NEAR(distance(ray2, p), distance(ray3, q), numericTolerance);
        const auto s = closestPoint(segment2, p), l = closestPoint(line2, p), r = closestPoint(ray2, p);
        EXPECT_TRUE(areCoincident(closestPoint(segment3, q), Point3{s.x(), s.y(), 7}, numericTolerance));
        EXPECT_TRUE(areCoincident(closestPoint(line3, q), Point3{l.x(), l.y(), 7}, numericTolerance));
        EXPECT_TRUE(areCoincident(closestPoint(ray3, q), Point3{r.x(), r.y(), 7}, numericTolerance));
    }
}
}
