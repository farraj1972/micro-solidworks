#include "core/geometry/Segment2.h"
#include "core/geometry/Segment3.h"
#include "core/geometry/Line2.h"
#include "core/geometry/Line3.h"
#include "core/geometry/Ray2.h"
#include "core/geometry/Ray3.h"
#include "core/geometry/Plane.h"
#include <gtest/gtest.h>
#include <array>
#include <cmath>
#include <limits>
#include <stdexcept>
namespace
{
using namespace microsw::geometry;
using microsw::math::Vector2;
using microsw::math::Vector3;
using microsw::math::Scalar;

Scalar projectionSeparation(const Point2& a, const Point2& b)
{
    return std::hypot(a.x() - b.x(), a.y() - b.y());
}

Scalar projectionSeparation(const Point3& a, const Point3& b)
{
    return std::hypot(a.x() - b.x(), a.y() - b.y(), a.z() - b.z());
}

template<class Primitive, class Point>
void expectProjection(const Primitive& primitive, const Point& query, const Point& expected)
{
    const auto closest = closestPoint(primitive, query);
    EXPECT_TRUE(areCoincident(closest, expected, 0));
    EXPECT_TRUE(primitive.contains(closest, 1e-12));
    EXPECT_NEAR(distance(primitive, closest), 0, 1e-12);
    const auto separation = projectionSeparation(query, closest);
    ASSERT_GT(separation, 0);
    EXPECT_NEAR(distance(primitive, query) / separation, 1, 1e-14);
}

TEST(Line2Metrics, ReconstructionPreservesSmallOriginAcrossSignsAndAxes)
{
    for (int axis = 0; axis < 2; ++axis)
    for (const Scalar small : {-1.0, 1.0})
    for (const Scalar large : {-1e20, 1e20})
    {
        SCOPED_TRACE(::testing::Message() << "axis=" << axis << " origin=" << small << " query=" << large);
        std::array<Scalar, 2> a{}, b{}, p{}, c{}, d{};
        a[axis] = b[axis] = c[axis] = small;
        p[axis] = large;
        const int along = (axis + 1) % 2;
        b[along] = 2;
        p[along] = c[along] = d[along] = 1;
        const Point2 origin{a[0], a[1]};
        const Line2 primitive{origin, Vector2{d[0], d[1]}};
        expectProjection(primitive,
            Point2{p[0], p[1]},
            Point2{c[0], c[1]});
    }
}

TEST(Line2Metrics, DiagonalCancellationRetainsOriginContribution)
{
    for (const Scalar sign : {-1.0, 1.0})
    {
        const Line2 primitive{Point2{0, -2 * sign},
            Vector2{sign, sign}};
        expectProjection(primitive, Point2{1e20 * sign, -1e20 * sign},
            Point2{sign, -sign});
    }
}

TEST(Ray2Metrics, ReconstructionPreservesSmallOriginAcrossSignsAndAxes)
{
    for (int axis = 0; axis < 2; ++axis)
    for (const Scalar small : {-1.0, 1.0})
    for (const Scalar large : {-1e20, 1e20})
    {
        SCOPED_TRACE(::testing::Message() << "axis=" << axis << " origin=" << small << " query=" << large);
        std::array<Scalar, 2> a{}, b{}, p{}, c{}, d{};
        a[axis] = b[axis] = c[axis] = small;
        p[axis] = large;
        const int along = (axis + 1) % 2;
        b[along] = 2;
        p[along] = c[along] = d[along] = 1;
        const Point2 origin{a[0], a[1]};
        const Ray2 primitive{origin, Vector2{d[0], d[1]}};
        expectProjection(primitive,
            Point2{p[0], p[1]},
            Point2{c[0], c[1]});
    }
}

TEST(Ray2Metrics, DiagonalCancellationRetainsOriginContribution)
{
    for (const Scalar sign : {-1.0, 1.0})
    {
        const Ray2 primitive{Point2{0, -2 * sign},
            Vector2{sign, sign}};
        expectProjection(primitive, Point2{1e20 * sign, -1e20 * sign},
            Point2{sign, -sign});
    }
}

TEST(Segment2Metrics, ReconstructionPreservesSmallOriginAcrossSignsAndAxes)
{
    for (int axis = 0; axis < 2; ++axis)
    for (const Scalar small : {-1.0, 1.0})
    for (const Scalar large : {-1e20, 1e20})
    {
        SCOPED_TRACE(::testing::Message() << "axis=" << axis << " origin=" << small << " query=" << large);
        std::array<Scalar, 2> a{}, b{}, p{}, c{}, d{};
        a[axis] = b[axis] = c[axis] = small;
        p[axis] = large;
        const int along = (axis + 1) % 2;
        b[along] = 2;
        p[along] = c[along] = d[along] = 1;
        const Point2 origin{a[0], a[1]};
        const Segment2 primitive{origin, Point2{b[0], b[1]}};
        expectProjection(primitive,
            Point2{p[0], p[1]},
            Point2{c[0], c[1]});
    }
}

TEST(Segment2Metrics, DiagonalCancellationRetainsOriginContribution)
{
    for (const Scalar sign : {-1.0, 1.0})
    {
        const Segment2 primitive{Point2{0, -2 * sign},
            Point2{2 * sign, 0}};
        expectProjection(primitive, Point2{1e20 * sign, -1e20 * sign},
            Point2{sign, -sign});
    }
}

TEST(Line3Metrics, ReconstructionPreservesSmallOriginAcrossSignsAndAxes)
{
    for (int axis = 0; axis < 3; ++axis)
    for (const Scalar small : {-1.0, 1.0})
    for (const Scalar large : {-1e20, 1e20})
    {
        SCOPED_TRACE(::testing::Message() << "axis=" << axis << " origin=" << small << " query=" << large);
        std::array<Scalar, 3> a{}, b{}, p{}, c{}, d{};
        a[axis] = b[axis] = c[axis] = small;
        p[axis] = large;
        const int along = (axis + 1) % 3;
        b[along] = 2;
        p[along] = c[along] = d[along] = 1;
        const Point3 origin{a[0], a[1], a[2]};
        const Line3 primitive{origin, Vector3{d[0], d[1], d[2]}};
        expectProjection(primitive,
            Point3{p[0], p[1], p[2]},
            Point3{c[0], c[1], c[2]});
    }
}

TEST(Line3Metrics, DiagonalCancellationRetainsOriginContribution)
{
    for (const Scalar sign : {-1.0, 1.0})
    {
        const Line3 primitive{Point3{0, -2 * sign, 1},
            Vector3{sign, sign, 0}};
        expectProjection(primitive, Point3{1e20 * sign, -1e20 * sign, -1e20},
            Point3{sign, -sign, 1});
    }
}

TEST(Ray3Metrics, ReconstructionPreservesSmallOriginAcrossSignsAndAxes)
{
    for (int axis = 0; axis < 3; ++axis)
    for (const Scalar small : {-1.0, 1.0})
    for (const Scalar large : {-1e20, 1e20})
    {
        SCOPED_TRACE(::testing::Message() << "axis=" << axis << " origin=" << small << " query=" << large);
        std::array<Scalar, 3> a{}, b{}, p{}, c{}, d{};
        a[axis] = b[axis] = c[axis] = small;
        p[axis] = large;
        const int along = (axis + 1) % 3;
        b[along] = 2;
        p[along] = c[along] = d[along] = 1;
        const Point3 origin{a[0], a[1], a[2]};
        const Ray3 primitive{origin, Vector3{d[0], d[1], d[2]}};
        expectProjection(primitive,
            Point3{p[0], p[1], p[2]},
            Point3{c[0], c[1], c[2]});
    }
}

TEST(Ray3Metrics, DiagonalCancellationRetainsOriginContribution)
{
    for (const Scalar sign : {-1.0, 1.0})
    {
        const Ray3 primitive{Point3{0, -2 * sign, 1},
            Vector3{sign, sign, 0}};
        expectProjection(primitive, Point3{1e20 * sign, -1e20 * sign, -1e20},
            Point3{sign, -sign, 1});
    }
}

TEST(Segment3Metrics, ReconstructionPreservesSmallOriginAcrossSignsAndAxes)
{
    for (int axis = 0; axis < 3; ++axis)
    for (const Scalar small : {-1.0, 1.0})
    for (const Scalar large : {-1e20, 1e20})
    {
        SCOPED_TRACE(::testing::Message() << "axis=" << axis << " origin=" << small << " query=" << large);
        std::array<Scalar, 3> a{}, b{}, p{}, c{}, d{};
        a[axis] = b[axis] = c[axis] = small;
        p[axis] = large;
        const int along = (axis + 1) % 3;
        b[along] = 2;
        p[along] = c[along] = d[along] = 1;
        const Point3 origin{a[0], a[1], a[2]};
        const Segment3 primitive{origin, Point3{b[0], b[1], b[2]}};
        expectProjection(primitive,
            Point3{p[0], p[1], p[2]},
            Point3{c[0], c[1], c[2]});
    }
}

TEST(Segment3Metrics, DiagonalCancellationRetainsOriginContribution)
{
    for (const Scalar sign : {-1.0, 1.0})
    {
        const Segment3 primitive{Point3{0, -2 * sign, 1},
            Point3{2 * sign, 0, 1}};
        expectProjection(primitive, Point3{1e20 * sign, -1e20 * sign, -1e20},
            Point3{sign, -sign, 1});
    }
}

TEST(PlaneMetrics, ReconstructionPreservesSmallOriginAcrossSignsAndAxes)
{
    for (int axis = 0; axis < 3; ++axis)
    for (const Scalar small : {-1.0, 1.0})
    for (const Scalar large : {-1e20, 1e20})
    {
        SCOPED_TRACE(::testing::Message() << "axis=" << axis << " origin=" << small << " query=" << large);
        std::array<Scalar, 3> o{}, n{}, p{}, c{};
        o[axis] = c[axis] = small;
        n[axis] = 1;
        p[axis] = large;
        p[(axis + 1) % 3] = c[(axis + 1) % 3] = -large;
        p[(axis + 2) % 3] = c[(axis + 2) % 3] = 1;
        const Plane primitive{Point3{o[0], o[1], o[2]}, Vector3{n[0], n[1], n[2]}};
        const Point3 query{p[0], p[1], p[2]}, expected{c[0], c[1], c[2]};
        expectProjection(primitive, query, expected);
        EXPECT_DOUBLE_EQ(signedDistance(primitive, closestPoint(primitive, query)), 0);
    }
}

TEST(PlaneMetrics, DiagonalCancellationRetainsOriginContribution)
{
    for (const Scalar sign : {-1.0, 1.0})
    {
        const Plane primitive{Point3{sign, sign, 0}, Vector3{1, 1, 0}};
        const Point3 query{sign * 1e20, sign * 1e20, 1}, expected{sign, sign, 1};
        expectProjection(primitive, query, expected);
        EXPECT_DOUBLE_EQ(signedDistance(primitive, closestPoint(primitive, query)), 0);
    }
}

TEST(ProjectionMetrics, HugeOriginsPreserveTinyQueryCoordinatesAcrossAxes)
{
    const auto m = std::numeric_limits<Scalar>::max();
    for (int axis = 0; axis < 3; ++axis)
    for (const Scalar sign : {-1.0, 1.0})
    {
        std::array<Scalar, 3> a{}, b{}, d{}, p{}, c{};
        a[axis] = -sign * m;
        b[axis] = sign * m;
        d[axis] = sign;
        p[axis] = c[axis] = sign * 1e-100;
        p[(axis + 1) % 3] = 5e-10;
        const Point3 origin{a[0], a[1], a[2]}, end{b[0], b[1], b[2]};
        const Vector3 direction{d[0], d[1], d[2]};
        const Point3 query{p[0], p[1], p[2]}, expected{c[0], c[1], c[2]};
        expectProjection(Line3{origin, direction}, query, expected);
        expectProjection(Ray3{origin, direction}, query, expected);
        expectProjection(Segment3{origin, end}, query, expected);
    }
}

TEST(ProjectionMetrics, CancelledDiagonalParameterStillHonorsRayAndSegmentBounds)
{
    const Point2 query2{1e20, -1e20};
    const Point3 query3{1e20, -1e20, 0};
    const Ray2 ray2{Point2{0, 2}, Vector2{1, 1}};
    const Ray3 ray3{Point3{0, 2, 0}, Vector3{1, 1, 0}};
    expectProjection(ray2, query2, ray2.origin());
    expectProjection(ray3, query3, ray3.origin());
    const Segment2 segment2{Point2{0, -4}, Point2{1, -3}};
    const Segment3 segment3{Point3{0, -4, 0}, Point3{1, -3, 0}};
    expectProjection(segment2, query2, segment2.b());
    expectProjection(segment3, query3, segment3.b());
}

TEST(Segment2Metrics, AxisProjectionDomainAndDistance)
{
    const Segment2 primitive{Point2{1, 0}, Point2{5, 0}};
    for (const auto point : {Point2{1, 0}, Point2{3, 0}, Point2{5, 0}})
    {
        EXPECT_TRUE(areCoincident(closestPoint(primitive, point), point, 0));
        EXPECT_DOUBLE_EQ(distance(primitive, point), 0);
    }
    EXPECT_TRUE(areCoincident(closestPoint(primitive, Point2{3, 4}), Point2{3, 0}, 0));
    EXPECT_DOUBLE_EQ(distance(primitive, Point2{3, 4}), 4);
    EXPECT_TRUE(areCoincident(closestPoint(primitive, Point2{-2, 0}), Point2{1, 0}, 0));
    EXPECT_DOUBLE_EQ(distance(primitive, Point2{-2, 0}), 3);
    EXPECT_TRUE(areCoincident(closestPoint(primitive, Point2{8, 0}), Point2{5, 0}, 0));
    EXPECT_DOUBLE_EQ(distance(primitive, Point2{8, 0}), 3);
}

TEST(Segment2Metrics, MetricResidualDoesNotSnapToTolerance)
{
    const Segment2 primitive{Point2{}, Point2{4, 0}};
    const auto point = Point2{2, 5e-10};
    EXPECT_TRUE(primitive.contains(point));
    EXPECT_DOUBLE_EQ(distance(primitive, point), 5e-10);
    EXPECT_TRUE(areCoincident(closestPoint(primitive, point), Point2{2, 0}, 0));
}

TEST(Segment2Metrics, DiagonalTranslatedProjectionIsConsistent)
{
    const Segment2 primitive{Point2{2, -1}, Point2{5, 3}};
    // Offset (4,-3) is perpendicular to the direction (3,4).
    const auto onPrimitive = primitive.pointAt(0.5);
    const auto point = onPrimitive + Vector2{4, -3};
    const auto closest = closestPoint(primitive, point);
    EXPECT_TRUE(areCoincident(closest, onPrimitive, 1e-12));
    EXPECT_TRUE(primitive.contains(closest));
    EXPECT_NEAR(distance(primitive, point), 5, 1e-12);
    EXPECT_NEAR(distance(primitive, closest), 0, 1e-12);
}

TEST(Segment2Metrics, ExtremeOffsetsRemainRepresentableWhenResultDoes)
{
    const auto m = std::numeric_limits<Scalar>::max();
    const Segment2 primitive{Point2{-m, 0}, Point2{m, 0}};
    EXPECT_TRUE(areCoincident(closestPoint(primitive, Point2{m, 2}), Point2{m, 0}, 0));
    EXPECT_DOUBLE_EQ(distance(primitive, Point2{m, 2}), 2);
    EXPECT_DOUBLE_EQ(distance(primitive, Point2{m, 5e-10}), 5e-10);
}

TEST(Segment2Metrics, NonrepresentableDistanceThrows)
{
    const auto m = std::numeric_limits<Scalar>::max();
    const Segment2 primitive{Point2{0, -m}, Point2{1, -m}};
    EXPECT_THROW((void)distance(primitive, Point2{0, m}), std::overflow_error);
}

TEST(Segment2Metrics, DegenerateSegmentReturnsAWithoutDirectionFailure)
{
    const Segment2 primitive{Point2{2, 3}, Point2{2, 3}};
    EXPECT_TRUE(areCoincident(closestPoint(primitive, Point2{5, 7}), primitive.a(), 0));
    EXPECT_DOUBLE_EQ(distance(primitive, Point2{5, 7}), 5);
    const Segment2 nearZero{Point2{}, Point2{5e-10, 0}};
    EXPECT_TRUE(areCoincident(closestPoint(nearZero, nearZero.b()), nearZero.a(), 0));
    EXPECT_DOUBLE_EQ(distance(nearZero, nearZero.b()), 5e-10);
}

TEST(Segment2Metrics, SubnormalProjectionParameterDoesNotEraseFiniteResult)
{
    const auto m = std::numeric_limits<Scalar>::max();
    const Segment2 primitive{Point2{}, Point2{m, 0}};
    const auto point = Point2{1e-100, 1e-100};
    EXPECT_NEAR(closestPoint(primitive, point).x() / 1e-100, 1, 1e-12);
    EXPECT_NEAR(distance(primitive, point) / 1e-100, 1, 1e-12);
}

TEST(Segment3Metrics, AxisProjectionDomainAndDistance)
{
    const Segment3 primitive{Point3{1, 0, 0}, Point3{5, 0, 0}};
    for (const auto point : {Point3{1, 0, 0}, Point3{3, 0, 0}, Point3{5, 0, 0}})
    {
        EXPECT_TRUE(areCoincident(closestPoint(primitive, point), point, 0));
        EXPECT_DOUBLE_EQ(distance(primitive, point), 0);
    }
    EXPECT_TRUE(areCoincident(closestPoint(primitive, Point3{3, 4, 0}), Point3{3, 0, 0}, 0));
    EXPECT_DOUBLE_EQ(distance(primitive, Point3{3, 4, 0}), 4);
    EXPECT_TRUE(areCoincident(closestPoint(primitive, Point3{-2, 0, 0}), Point3{1, 0, 0}, 0));
    EXPECT_DOUBLE_EQ(distance(primitive, Point3{-2, 0, 0}), 3);
    EXPECT_TRUE(areCoincident(closestPoint(primitive, Point3{8, 0, 0}), Point3{5, 0, 0}, 0));
    EXPECT_DOUBLE_EQ(distance(primitive, Point3{8, 0, 0}), 3);
}

TEST(Segment3Metrics, MetricResidualDoesNotSnapToTolerance)
{
    const Segment3 primitive{Point3{}, Point3{4, 0, 0}};
    const auto point = Point3{2, 5e-10, 0};
    EXPECT_TRUE(primitive.contains(point));
    EXPECT_DOUBLE_EQ(distance(primitive, point), 5e-10);
    EXPECT_TRUE(areCoincident(closestPoint(primitive, point), Point3{2, 0, 0}, 0));
}

TEST(Segment3Metrics, DiagonalTranslatedProjectionIsConsistent)
{
    const Segment3 primitive{Point3{2, -1, 0}, Point3{5, 3, 0}};
    // Offset (4,-3) is perpendicular to the direction (3,4).
    const auto onPrimitive = primitive.pointAt(0.5);
    const auto point = onPrimitive + Vector3{4, -3, 0};
    const auto closest = closestPoint(primitive, point);
    EXPECT_TRUE(areCoincident(closest, onPrimitive, 1e-12));
    EXPECT_TRUE(primitive.contains(closest));
    EXPECT_NEAR(distance(primitive, point), 5, 1e-12);
    EXPECT_NEAR(distance(primitive, closest), 0, 1e-12);
}

TEST(Segment3Metrics, ExtremeOffsetsRemainRepresentableWhenResultDoes)
{
    const auto m = std::numeric_limits<Scalar>::max();
    const Segment3 primitive{Point3{-m, 0, 0}, Point3{m, 0, 0}};
    EXPECT_TRUE(areCoincident(closestPoint(primitive, Point3{m, 2, 0}), Point3{m, 0, 0}, 0));
    EXPECT_DOUBLE_EQ(distance(primitive, Point3{m, 2, 0}), 2);
    EXPECT_DOUBLE_EQ(distance(primitive, Point3{m, 5e-10, 0}), 5e-10);
}

TEST(Segment3Metrics, NonrepresentableDistanceThrows)
{
    const auto m = std::numeric_limits<Scalar>::max();
    const Segment3 primitive{Point3{0, -m, 0}, Point3{1, -m, 0}};
    EXPECT_THROW((void)distance(primitive, Point3{0, m, 0}), std::overflow_error);
}

TEST(Segment3Metrics, DegenerateSegmentReturnsAWithoutDirectionFailure)
{
    const Segment3 primitive{Point3{2, 3, 0}, Point3{2, 3, 0}};
    EXPECT_TRUE(areCoincident(closestPoint(primitive, Point3{5, 7, 0}), primitive.a(), 0));
    EXPECT_DOUBLE_EQ(distance(primitive, Point3{5, 7, 0}), 5);
    const Segment3 nearZero{Point3{}, Point3{5e-10, 0, 0}};
    EXPECT_TRUE(areCoincident(closestPoint(nearZero, nearZero.b()), nearZero.a(), 0));
    EXPECT_DOUBLE_EQ(distance(nearZero, nearZero.b()), 5e-10);
}

TEST(Segment3Metrics, SubnormalProjectionParameterDoesNotEraseFiniteResult)
{
    const auto m = std::numeric_limits<Scalar>::max();
    const Segment3 primitive{Point3{}, Point3{m, 0, 0}};
    const auto point = Point3{1e-100, 1e-100, 0};
    EXPECT_NEAR(closestPoint(primitive, point).x() / 1e-100, 1, 1e-12);
    EXPECT_NEAR(distance(primitive, point) / 1e-100, 1, 1e-12);
}

TEST(Line2Metrics, AxisProjectionDomainAndDistance)
{
    const Line2 primitive{Point2{1, 0}, Vector2{1, 0}};
    for (const auto point : {Point2{1, 0}, Point2{3, 0}, Point2{5, 0}})
    {
        EXPECT_TRUE(areCoincident(closestPoint(primitive, point), point, 0));
        EXPECT_DOUBLE_EQ(distance(primitive, point), 0);
    }
    EXPECT_TRUE(areCoincident(closestPoint(primitive, Point2{3, 4}), Point2{3, 0}, 0));
    EXPECT_DOUBLE_EQ(distance(primitive, Point2{3, 4}), 4);
    EXPECT_TRUE(areCoincident(closestPoint(primitive, Point2{-2, 0}), Point2{-2, 0}, 0));
    EXPECT_DOUBLE_EQ(distance(primitive, Point2{-2, 0}), 0);
    EXPECT_TRUE(areCoincident(closestPoint(primitive, Point2{8, 0}), Point2{8, 0}, 0));
    EXPECT_DOUBLE_EQ(distance(primitive, Point2{8, 0}), 0);
}

TEST(Line2Metrics, MetricResidualDoesNotSnapToTolerance)
{
    const Line2 primitive{Point2{}, Vector2{1, 0}};
    const auto point = Point2{2, 5e-10};
    EXPECT_TRUE(primitive.contains(point));
    EXPECT_DOUBLE_EQ(distance(primitive, point), 5e-10);
    EXPECT_TRUE(areCoincident(closestPoint(primitive, point), Point2{2, 0}, 0));
}

TEST(Line2Metrics, DiagonalTranslatedProjectionIsConsistent)
{
    const Line2 primitive{Point2{2, -1}, Vector2{3, 4}};
    // Offset (4,-3) is perpendicular to the direction (3,4).
    const auto onPrimitive = primitive.pointAt(2.5);
    const auto point = onPrimitive + Vector2{4, -3};
    const auto closest = closestPoint(primitive, point);
    EXPECT_TRUE(areCoincident(closest, onPrimitive, 1e-12));
    EXPECT_TRUE(primitive.contains(closest));
    EXPECT_NEAR(distance(primitive, point), 5, 1e-12);
    EXPECT_NEAR(distance(primitive, closest), 0, 1e-12);
}

TEST(Line2Metrics, ExtremeOffsetsRemainRepresentableWhenResultDoes)
{
    const auto m = std::numeric_limits<Scalar>::max();
    const Line2 primitive{Point2{-m, 0}, Vector2{1, 0}};
    EXPECT_TRUE(areCoincident(closestPoint(primitive, Point2{m, 2}), Point2{m, 0}, 0));
    EXPECT_DOUBLE_EQ(distance(primitive, Point2{m, 2}), 2);
    EXPECT_DOUBLE_EQ(distance(primitive, Point2{m, 5e-10}), 5e-10);
}

TEST(Line2Metrics, NonrepresentableDistanceThrows)
{
    const auto m = std::numeric_limits<Scalar>::max();
    const Line2 primitive{Point2{0, -m}, Vector2{1, 0}};
    EXPECT_THROW((void)distance(primitive, Point2{0, m}), std::overflow_error);
}

TEST(Line2Metrics, NonrepresentableClosestPointThrows)
{
    const auto m = std::numeric_limits<Scalar>::max();
    const Line2 primitive{Point2{m, 0}, Vector2{1, 1}};
    // Projection has x = 1.5 * max, although its perpendicular distance is finite.
    EXPECT_THROW((void)closestPoint(primitive, Point2{m, m}), std::overflow_error);
    EXPECT_TRUE(std::isfinite(distance(primitive, Point2{m, m})));
}

TEST(Line3Metrics, AxisProjectionDomainAndDistance)
{
    const Line3 primitive{Point3{1, 0, 0}, Vector3{1, 0, 0}};
    for (const auto point : {Point3{1, 0, 0}, Point3{3, 0, 0}, Point3{5, 0, 0}})
    {
        EXPECT_TRUE(areCoincident(closestPoint(primitive, point), point, 0));
        EXPECT_DOUBLE_EQ(distance(primitive, point), 0);
    }
    EXPECT_TRUE(areCoincident(closestPoint(primitive, Point3{3, 4, 0}), Point3{3, 0, 0}, 0));
    EXPECT_DOUBLE_EQ(distance(primitive, Point3{3, 4, 0}), 4);
    EXPECT_TRUE(areCoincident(closestPoint(primitive, Point3{-2, 0, 0}), Point3{-2, 0, 0}, 0));
    EXPECT_DOUBLE_EQ(distance(primitive, Point3{-2, 0, 0}), 0);
    EXPECT_TRUE(areCoincident(closestPoint(primitive, Point3{8, 0, 0}), Point3{8, 0, 0}, 0));
    EXPECT_DOUBLE_EQ(distance(primitive, Point3{8, 0, 0}), 0);
}

TEST(Line3Metrics, MetricResidualDoesNotSnapToTolerance)
{
    const Line3 primitive{Point3{}, Vector3{1, 0, 0}};
    const auto point = Point3{2, 5e-10, 0};
    EXPECT_TRUE(primitive.contains(point));
    EXPECT_DOUBLE_EQ(distance(primitive, point), 5e-10);
    EXPECT_TRUE(areCoincident(closestPoint(primitive, point), Point3{2, 0, 0}, 0));
}

TEST(Line3Metrics, DiagonalTranslatedProjectionIsConsistent)
{
    const Line3 primitive{Point3{2, -1, 0}, Vector3{3, 4, 0}};
    // Offset (4,-3) is perpendicular to the direction (3,4).
    const auto onPrimitive = primitive.pointAt(2.5);
    const auto point = onPrimitive + Vector3{4, -3, 0};
    const auto closest = closestPoint(primitive, point);
    EXPECT_TRUE(areCoincident(closest, onPrimitive, 1e-12));
    EXPECT_TRUE(primitive.contains(closest));
    EXPECT_NEAR(distance(primitive, point), 5, 1e-12);
    EXPECT_NEAR(distance(primitive, closest), 0, 1e-12);
}

TEST(Line3Metrics, ExtremeOffsetsRemainRepresentableWhenResultDoes)
{
    const auto m = std::numeric_limits<Scalar>::max();
    const Line3 primitive{Point3{-m, 0, 0}, Vector3{1, 0, 0}};
    EXPECT_TRUE(areCoincident(closestPoint(primitive, Point3{m, 2, 0}), Point3{m, 0, 0}, 0));
    EXPECT_DOUBLE_EQ(distance(primitive, Point3{m, 2, 0}), 2);
    EXPECT_DOUBLE_EQ(distance(primitive, Point3{m, 5e-10, 0}), 5e-10);
}

TEST(Line3Metrics, NonrepresentableDistanceThrows)
{
    const auto m = std::numeric_limits<Scalar>::max();
    const Line3 primitive{Point3{0, -m, 0}, Vector3{1, 0, 0}};
    EXPECT_THROW((void)distance(primitive, Point3{0, m, 0}), std::overflow_error);
}

TEST(Line3Metrics, NonrepresentableClosestPointThrows)
{
    const auto m = std::numeric_limits<Scalar>::max();
    const Line3 primitive{Point3{m, 0, 0}, Vector3{1, 1, 0}};
    // Projection has x = 1.5 * max, although its perpendicular distance is finite.
    EXPECT_THROW((void)closestPoint(primitive, Point3{m, m, 0}), std::overflow_error);
    EXPECT_TRUE(std::isfinite(distance(primitive, Point3{m, m, 0})));
}

TEST(Ray2Metrics, AxisProjectionDomainAndDistance)
{
    const Ray2 primitive{Point2{1, 0}, Vector2{1, 0}};
    for (const auto point : {Point2{1, 0}, Point2{3, 0}, Point2{5, 0}})
    {
        EXPECT_TRUE(areCoincident(closestPoint(primitive, point), point, 0));
        EXPECT_DOUBLE_EQ(distance(primitive, point), 0);
    }
    EXPECT_TRUE(areCoincident(closestPoint(primitive, Point2{3, 4}), Point2{3, 0}, 0));
    EXPECT_DOUBLE_EQ(distance(primitive, Point2{3, 4}), 4);
    EXPECT_TRUE(areCoincident(closestPoint(primitive, Point2{-2, 0}), Point2{1, 0}, 0));
    EXPECT_DOUBLE_EQ(distance(primitive, Point2{-2, 0}), 3);
    EXPECT_TRUE(areCoincident(closestPoint(primitive, Point2{8, 0}), Point2{8, 0}, 0));
    EXPECT_DOUBLE_EQ(distance(primitive, Point2{8, 0}), 0);
}

TEST(Ray2Metrics, MetricResidualDoesNotSnapToTolerance)
{
    const Ray2 primitive{Point2{}, Vector2{1, 0}};
    const auto point = Point2{2, 5e-10};
    EXPECT_TRUE(primitive.contains(point));
    EXPECT_DOUBLE_EQ(distance(primitive, point), 5e-10);
    EXPECT_TRUE(areCoincident(closestPoint(primitive, point), Point2{2, 0}, 0));
}

TEST(Ray2Metrics, DiagonalTranslatedProjectionIsConsistent)
{
    const Ray2 primitive{Point2{2, -1}, Vector2{3, 4}};
    // Offset (4,-3) is perpendicular to the direction (3,4).
    const auto onPrimitive = primitive.pointAt(2.5);
    const auto point = onPrimitive + Vector2{4, -3};
    const auto closest = closestPoint(primitive, point);
    EXPECT_TRUE(areCoincident(closest, onPrimitive, 1e-12));
    EXPECT_TRUE(primitive.contains(closest));
    EXPECT_NEAR(distance(primitive, point), 5, 1e-12);
    EXPECT_NEAR(distance(primitive, closest), 0, 1e-12);
}

TEST(Ray2Metrics, ExtremeOffsetsRemainRepresentableWhenResultDoes)
{
    const auto m = std::numeric_limits<Scalar>::max();
    const Ray2 primitive{Point2{-m, 0}, Vector2{1, 0}};
    EXPECT_TRUE(areCoincident(closestPoint(primitive, Point2{m, 2}), Point2{m, 0}, 0));
    EXPECT_DOUBLE_EQ(distance(primitive, Point2{m, 2}), 2);
    EXPECT_DOUBLE_EQ(distance(primitive, Point2{m, 5e-10}), 5e-10);
}

TEST(Ray2Metrics, NonrepresentableDistanceThrows)
{
    const auto m = std::numeric_limits<Scalar>::max();
    const Ray2 primitive{Point2{0, -m}, Vector2{1, 0}};
    EXPECT_THROW((void)distance(primitive, Point2{0, m}), std::overflow_error);
}

TEST(Ray2Metrics, SlightlyNegativeParameterClampsExactlyToOrigin)
{
    const Ray2 primitive{Point2{}, Vector2{1, 0}};
    const auto point = Point2{-5e-10, 0};
    EXPECT_TRUE(primitive.contains(point));
    EXPECT_TRUE(areCoincident(closestPoint(primitive, point), primitive.origin(), 0));
    EXPECT_DOUBLE_EQ(distance(primitive, point), 5e-10);
}

TEST(Ray2Metrics, NonrepresentableClosestPointThrows)
{
    const auto m = std::numeric_limits<Scalar>::max();
    const Ray2 primitive{Point2{m, 0}, Vector2{1, 1}};
    // Projection has x = 1.5 * max, although its perpendicular distance is finite.
    EXPECT_THROW((void)closestPoint(primitive, Point2{m, m}), std::overflow_error);
    EXPECT_TRUE(std::isfinite(distance(primitive, Point2{m, m})));
}

TEST(Ray3Metrics, AxisProjectionDomainAndDistance)
{
    const Ray3 primitive{Point3{1, 0, 0}, Vector3{1, 0, 0}};
    for (const auto point : {Point3{1, 0, 0}, Point3{3, 0, 0}, Point3{5, 0, 0}})
    {
        EXPECT_TRUE(areCoincident(closestPoint(primitive, point), point, 0));
        EXPECT_DOUBLE_EQ(distance(primitive, point), 0);
    }
    EXPECT_TRUE(areCoincident(closestPoint(primitive, Point3{3, 4, 0}), Point3{3, 0, 0}, 0));
    EXPECT_DOUBLE_EQ(distance(primitive, Point3{3, 4, 0}), 4);
    EXPECT_TRUE(areCoincident(closestPoint(primitive, Point3{-2, 0, 0}), Point3{1, 0, 0}, 0));
    EXPECT_DOUBLE_EQ(distance(primitive, Point3{-2, 0, 0}), 3);
    EXPECT_TRUE(areCoincident(closestPoint(primitive, Point3{8, 0, 0}), Point3{8, 0, 0}, 0));
    EXPECT_DOUBLE_EQ(distance(primitive, Point3{8, 0, 0}), 0);
}

TEST(Ray3Metrics, MetricResidualDoesNotSnapToTolerance)
{
    const Ray3 primitive{Point3{}, Vector3{1, 0, 0}};
    const auto point = Point3{2, 5e-10, 0};
    EXPECT_TRUE(primitive.contains(point));
    EXPECT_DOUBLE_EQ(distance(primitive, point), 5e-10);
    EXPECT_TRUE(areCoincident(closestPoint(primitive, point), Point3{2, 0, 0}, 0));
}

TEST(Ray3Metrics, DiagonalTranslatedProjectionIsConsistent)
{
    const Ray3 primitive{Point3{2, -1, 0}, Vector3{3, 4, 0}};
    // Offset (4,-3) is perpendicular to the direction (3,4).
    const auto onPrimitive = primitive.pointAt(2.5);
    const auto point = onPrimitive + Vector3{4, -3, 0};
    const auto closest = closestPoint(primitive, point);
    EXPECT_TRUE(areCoincident(closest, onPrimitive, 1e-12));
    EXPECT_TRUE(primitive.contains(closest));
    EXPECT_NEAR(distance(primitive, point), 5, 1e-12);
    EXPECT_NEAR(distance(primitive, closest), 0, 1e-12);
}

TEST(Ray3Metrics, ExtremeOffsetsRemainRepresentableWhenResultDoes)
{
    const auto m = std::numeric_limits<Scalar>::max();
    const Ray3 primitive{Point3{-m, 0, 0}, Vector3{1, 0, 0}};
    EXPECT_TRUE(areCoincident(closestPoint(primitive, Point3{m, 2, 0}), Point3{m, 0, 0}, 0));
    EXPECT_DOUBLE_EQ(distance(primitive, Point3{m, 2, 0}), 2);
    EXPECT_DOUBLE_EQ(distance(primitive, Point3{m, 5e-10, 0}), 5e-10);
}

TEST(Ray3Metrics, NonrepresentableDistanceThrows)
{
    const auto m = std::numeric_limits<Scalar>::max();
    const Ray3 primitive{Point3{0, -m, 0}, Vector3{1, 0, 0}};
    EXPECT_THROW((void)distance(primitive, Point3{0, m, 0}), std::overflow_error);
}

TEST(Ray3Metrics, SlightlyNegativeParameterClampsExactlyToOrigin)
{
    const Ray3 primitive{Point3{}, Vector3{1, 0, 0}};
    const auto point = Point3{-5e-10, 0, 0};
    EXPECT_TRUE(primitive.contains(point));
    EXPECT_TRUE(areCoincident(closestPoint(primitive, point), primitive.origin(), 0));
    EXPECT_DOUBLE_EQ(distance(primitive, point), 5e-10);
}

TEST(Ray3Metrics, NonrepresentableClosestPointThrows)
{
    const auto m = std::numeric_limits<Scalar>::max();
    const Ray3 primitive{Point3{m, 0, 0}, Vector3{1, 1, 0}};
    // Projection has x = 1.5 * max, although its perpendicular distance is finite.
    EXPECT_THROW((void)closestPoint(primitive, Point3{m, m, 0}), std::overflow_error);
    EXPECT_TRUE(std::isfinite(distance(primitive, Point3{m, m, 0})));
}

TEST(PlaneMetrics, SignedDistanceOrientationAndProjection)
{
    const Plane plane{Point3{1, 2, 3}, Vector3{0, 0, 2}};
    const Plane reverse{plane.origin(), -plane.normal()};
    for (Scalar height : {-4.0, 0.0, 4.0})
    {
        const Point3 point{5, -7, 3 + height};
        const auto closest = closestPoint(plane, point);
        EXPECT_TRUE(areCoincident(closest, Point3{5, -7, 3}, 0));
        EXPECT_DOUBLE_EQ(signedDistance(plane, point), height);
        EXPECT_DOUBLE_EQ(signedDistance(reverse, point), -height);
        EXPECT_DOUBLE_EQ(distance(plane, point), std::abs(height));
        EXPECT_DOUBLE_EQ(distance(reverse, point), distance(plane, point));
        EXPECT_TRUE(areCoincident(closestPoint(reverse, point), closest, 0));
        EXPECT_TRUE(plane.contains(closest));
        EXPECT_DOUBLE_EQ(distance(plane, closest), 0);
    }
}

TEST(PlaneMetrics, SmallResidualDoesNotSnap)
{
    const Plane plane{Point3{}, Vector3{0, 0, 1}};
    const Point3 point{2, -3, 5e-10};
    EXPECT_TRUE(plane.contains(point));
    EXPECT_DOUBLE_EQ(distance(plane, point), 5e-10);
    EXPECT_DOUBLE_EQ(signedDistance(plane, point), 5e-10);
    EXPECT_TRUE(areCoincident(closestPoint(plane, point), Point3{2, -3, 0}, 0));
}

TEST(PlaneMetrics, DiagonalNormalAndTangentDisplacement)
{
    const Plane plane{Point3{1, 2, 3}, Vector3{3, -4, 12}};
    const auto tangentPoint = plane.origin() + Vector3{4, 3, 0};
    const auto point = tangentPoint + 5 * plane.normal();
    const auto closest = closestPoint(plane, point);
    EXPECT_TRUE(areCoincident(closest, tangentPoint, 1e-12));
    EXPECT_NEAR(signedDistance(plane, point), 5, 1e-12);
    EXPECT_NEAR(distance(plane, closest), 0, 1e-12);
    EXPECT_TRUE(plane.contains(closest));
    EXPECT_TRUE(microsw::math::almostEqual(microsw::math::cross(point - closest, plane.normal()), Vector3{}));
}

TEST(PlaneMetrics, ExtremeSignedOffsetMayOverflowWhileClosestIsFinite)
{
    const auto m = std::numeric_limits<Scalar>::max();
    const Plane plane{Point3{-m, 0, 0}, Vector3{1, 0, 0}};
    const Point3 point{m, 2, 3};
    EXPECT_THROW((void)signedDistance(plane, point), std::overflow_error);
    EXPECT_THROW((void)distance(plane, point), std::overflow_error);
    EXPECT_TRUE(areCoincident(closestPoint(plane, point), Point3{-m, 2, 3}, 0));
}

TEST(PlaneMetrics, TangentialExtremesPreserveSmallMetricOffset)
{
    const auto m = std::numeric_limits<Scalar>::max();
    const Plane plane{Point3{-m, 0, 0}, Vector3{0, 1, 0}};
    const Point3 point{m, 5e-10, m};
    EXPECT_DOUBLE_EQ(distance(plane, point), 5e-10);
    EXPECT_TRUE(areCoincident(closestPoint(plane, point), Point3{m, 0, m}, 0));
}

TEST(PlaneMetrics, NonrepresentableClosestPointThrows)
{
    const auto m = std::numeric_limits<Scalar>::max();
    const Plane plane{Point3{m, 0, 0}, Vector3{1, -1, 0}};
    EXPECT_THROW((void)closestPoint(plane, Point3{m, m, 0}), std::overflow_error);
}

TEST(PlaneMetrics, CancellationRetainsFiniteSignedResidual)
{
    const auto m = std::numeric_limits<Scalar>::max();
    const Plane plane{Point3{}, Vector3{1, -1, 1}};
    EXPECT_NEAR(signedDistance(plane, Point3{m, m, 1}), plane.normal().z(), 1e-12);
}

TEST(ProjectionMetrics, ExtremeOriginDoesNotEraseSmallProjectedCoordinate)
{
    const auto m = std::numeric_limits<Scalar>::max();
    const Point3 point{1, 2, 3}, expected{1, 0, 0};
    const Line3 line{Point3{-m, 0, 0}, Vector3{1, 0, 0}};
    const Ray3 ray{line.origin(), line.direction()};
    const Segment3 segment{line.origin(), Point3{m, 0, 0}};
    EXPECT_TRUE(areCoincident(closestPoint(line, point), expected, 0));
    EXPECT_TRUE(areCoincident(closestPoint(ray, point), expected, 0));
    EXPECT_TRUE(areCoincident(closestPoint(segment, point), expected, 0));
    EXPECT_NEAR(distance(line, point), std::sqrt(13.0), 1e-12);
    EXPECT_NEAR(distance(ray, point), std::sqrt(13.0), 1e-12);
    EXPECT_NEAR(distance(segment, point), std::sqrt(13.0), 1e-12);
    const Point2 point2{1, 2}, expected2{1, 0};
    const Line2 line2{Point2{-m, 0}, Vector2{1, 0}};
    const Ray2 ray2{line2.origin(), line2.direction()};
    const Segment2 segment2{line2.origin(), Point2{m, 0}};
    EXPECT_TRUE(areCoincident(closestPoint(line2, point2), expected2, 0));
    EXPECT_TRUE(areCoincident(closestPoint(ray2, point2), expected2, 0));
    EXPECT_TRUE(areCoincident(closestPoint(segment2, point2), expected2, 0));
}
}
