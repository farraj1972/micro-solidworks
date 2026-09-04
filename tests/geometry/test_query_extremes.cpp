#include "core/geometry/Line3.h"
#include "core/geometry/Ray3.h"
#include "core/geometry/Segment3.h"
#include "core/geometry/Plane.h"
#include <gtest/gtest.h>
#include <limits>
namespace
{
using namespace microsw::geometry;
using microsw::math::Vector3;
using microsw::math::Scalar;

TEST(PrimitiveQueryExtremes, DotCancellationPreservesSmallRemainingTerm)
{
    const auto m = std::numeric_limits<Scalar>::max();
    const Plane plane{Point3{}, Vector3{1, -1, 1}};
    EXPECT_TRUE(plane.contains(Point3{m, m, 0}, 0));
    EXPECT_FALSE(plane.contains(Point3{m, m, 1}, 0));
    EXPECT_TRUE(plane.contains(Point3{m, m, 5e-10}));
    EXPECT_FALSE(plane.contains(Point3{m, m, 4e-9}));
}

TEST(PrimitiveQueryExtremes, DotAndCrossBeyondScalarRangeCompareFalse)
{
    const auto m = std::numeric_limits<Scalar>::max();
    const Plane plane{Point3{-m, -m, -m}, Vector3{1, 1, 1}};
    EXPECT_FALSE(plane.contains(Point3{m, m, m}, m));
    const Line3 line{Point3{0, -m, -m}, Vector3{1, 0, 0}};
    EXPECT_FALSE(line.contains(Point3{0, m, m}, m));
}

TEST(PrimitiveQueryExtremes, ThreeDimensionalPerpendicularOffset)
{
    const Line3 line{Point3{}, Vector3{1, 0, 0}};
    const Ray3 ray{Point3{}, Vector3{1, 0, 0}};
    const Segment3 segment{Point3{}, Point3{1, 0, 0}};
    for (const auto point : {Point3{0.5, 0, 2e-9}, Point3{0.5, 0.8e-9, 0.8e-9}})
    {
        EXPECT_FALSE(line.contains(point));
        EXPECT_FALSE(ray.contains(point));
        EXPECT_FALSE(segment.contains(point));
    }
    EXPECT_TRUE(line.contains(Point3{0.5, 0, 1e-9}));
    EXPECT_TRUE(ray.contains(Point3{0.5, 0, 1e-9}));
    EXPECT_TRUE(segment.contains(Point3{0.5, 0, 1e-9}));
}

TEST(PrimitiveQueryExtremes, SegmentReversalAndIndependentEndpointSlop)
{
    const Segment3 a{Point3{}, Point3{2, 0, 0}}, b{a.b(), a.a()};
    for (const auto point : {Point3{}, Point3{2, 0, 0}, Point3{1, 0, 0},
             Point3{-5e-10, 5e-10, 0}, Point3{2 + 5e-10, 5e-10, 0},
             Point3{-2e-9, 0, 0}, Point3{1, 0, 2e-9}})
        EXPECT_EQ(a.contains(point), b.contains(point));
    EXPECT_TRUE(a.contains(Point3{-5e-10, 5e-10, 0}));
    EXPECT_FALSE(a.contains(Point3{-2e-9, 0, 0}));
}

TEST(PrimitiveQueryExtremes, RelationsIncludeZAxis)
{
    const Line3 x{Point3{}, Vector3{1, 0, 0}}, z{Point3{}, Vector3{0, 0, 1}};
    const Ray3 rx{Point3{}, Vector3{1, 0, 0}}, rz{Point3{}, Vector3{0, 0, -1}};
    const Plane px{Point3{}, Vector3{1, 0, 0}}, pz{Point3{}, Vector3{0, 0, 1}};
    const Segment3 sx{Point3{}, Point3{1, 0, 0}}, sz{Point3{}, Point3{0, 0, 1}};
    EXPECT_TRUE(isPerpendicular(x, z, 0));
    EXPECT_TRUE(isPerpendicular(rx, rz, 0));
    EXPECT_TRUE(isPerpendicular(px, pz, 0));
    EXPECT_TRUE(isPerpendicular(sx, sz, 0));
    EXPECT_FALSE(isParallel(x, z));
    EXPECT_FALSE(isParallel(rx, rz));
    EXPECT_FALSE(isParallel(px, pz));
    EXPECT_FALSE(isParallel(sx, sz));
}

TEST(PrimitiveQueryExtremes, CustomDegeneracyMatchesEndpointCoincidence)
{
    const Segment3 segment{Point3{}, Point3{0.5, 0, 0}};
    const Point3 point{0, 0.75, 0};
    EXPECT_EQ(segment.contains(point, 1), areCoincident(segment.a(), point, 1));
    EXPECT_FALSE(segment.contains(point, 0));
    EXPECT_TRUE(areCoincident(segment.pointAt(0.5), Point3{0.25, 0, 0}, 0));
}
}
