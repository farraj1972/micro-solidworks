#include "viewer/ReferenceGrid.h"
#include "core/math/Tolerance.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>

namespace
{

using microsw::viewer::ReferenceGrid;
using microsw::math::Vector3;
using microsw::math::almostEqual;

bool containsSegment(const ReferenceGrid& grid, const Vector3& a, const Vector3& b)
{
    const auto& vertices = grid.vertices();
    for (std::size_t i = 0; i < vertices.size(); i += 2)
    {
        if ((almostEqual(vertices[i], a) && almostEqual(vertices[i + 1], b))
            || (almostEqual(vertices[i], b) && almostEqual(vertices[i + 1], a)))
            return true;
    }
    return false;
}

TEST(ReferenceGridTest, DefaultsProduceFortySegments)
{
    const ReferenceGrid grid;
    EXPECT_TRUE(almostEqual(grid.halfExtent(), 10.0));
    EXPECT_TRUE(almostEqual(grid.spacing(), 1.0));
    EXPECT_EQ(grid.vertices().size(), 80u);
    EXPECT_EQ(grid.vertices().size() % 2, 0u);
}

TEST(ReferenceGridTest, VerticesAreFiniteInXYAndWithinExtent)
{
    for (const ReferenceGrid grid : {ReferenceGrid{}, ReferenceGrid{2.5, 1.0},
                                     ReferenceGrid{1.0, 0.3}})
    {
        EXPECT_EQ(grid.vertices().size() % 2, 0u);
        for (const auto& vertex : grid.vertices())
        {
            EXPECT_TRUE(std::isfinite(vertex.x()));
            EXPECT_TRUE(std::isfinite(vertex.y()));
            EXPECT_TRUE(std::isfinite(vertex.z()));
            EXPECT_DOUBLE_EQ(vertex.z(), 0.0); // Exact construction invariant.
            EXPECT_LE(std::abs(vertex.x()), grid.halfExtent());
            EXPECT_LE(std::abs(vertex.y()), grid.halfExtent());
        }
    }
}

TEST(ReferenceGridTest, EverySegmentHasXAndYReflections)
{
    const ReferenceGrid grid{2.5, 0.5};
    const auto& vertices = grid.vertices();
    for (std::size_t i = 0; i < vertices.size(); i += 2)
    {
        const auto& a = vertices[i];
        const auto& b = vertices[i + 1];
        EXPECT_TRUE(containsSegment(grid, {-a.x(), a.y(), 0.0}, {-b.x(), b.y(), 0.0}));
        EXPECT_TRUE(containsSegment(grid, {a.x(), -a.y(), 0.0}, {b.x(), -b.y(), 0.0}));
    }
}

TEST(ReferenceGridTest, SegmentsAreAxisParallelAndSpanTheExtent)
{
    const ReferenceGrid grid{2.5, 1.0};
    const auto& vertices = grid.vertices();
    for (std::size_t i = 0; i < vertices.size(); i += 2)
    {
        const auto& a = vertices[i];
        const auto& b = vertices[i + 1];
        const bool parallelY = almostEqual(a.x(), b.x());
        const bool parallelX = almostEqual(a.y(), b.y());
        EXPECT_NE(parallelX, parallelY);
        if (parallelY)
        {
            EXPECT_TRUE(almostEqual(a.y(), -grid.halfExtent()));
            EXPECT_TRUE(almostEqual(b.y(), grid.halfExtent()));
        }
        else
        {
            EXPECT_TRUE(almostEqual(a.x(), -grid.halfExtent()));
            EXPECT_TRUE(almostEqual(b.x(), grid.halfExtent()));
        }
    }
}

TEST(ReferenceGridTest, BothCentralLinesAreOmitted)
{
    const ReferenceGrid grid;
    for (std::size_t i = 0; i < grid.vertices().size(); i += 2)
    {
        const auto& a = grid.vertices()[i];
        const auto& b = grid.vertices()[i + 1];
        EXPECT_FALSE(almostEqual(a.x(), 0.0) && almostEqual(b.x(), 0.0));
        EXPECT_FALSE(almostEqual(a.y(), 0.0) && almostEqual(b.y(), 0.0));
    }
}

TEST(ReferenceGridTest, NonIntegralExtentKeepsSpacingAndFullSegmentEndpoints)
{
    const ReferenceGrid grid{2.5, 1.0};
    ASSERT_EQ(grid.vertices().size(), 16u);
    for (const double position : {-2.0, -1.0, 1.0, 2.0})
    {
        EXPECT_TRUE(containsSegment(grid, {position, -2.5, 0.0}, {position, 2.5, 0.0}));
        EXPECT_TRUE(containsSegment(grid, {-2.5, position, 0.0}, {2.5, position, 0.0}));
    }
}

TEST(ReferenceGridTest, FractionalSpacingIsDeterministicWithoutAccumulatedDrift)
{
    const ReferenceGrid first{1.0, 0.3};
    const ReferenceGrid second{1.0, 0.3};
    ASSERT_EQ(first.vertices().size(), 24u);
    ASSERT_EQ(first.vertices().size(), second.vertices().size());
    for (std::size_t i = 0; i < first.vertices().size(); ++i)
        EXPECT_TRUE(almostEqual(first.vertices()[i], second.vertices()[i]));
    for (int i = 1; i <= 3; ++i)
    {
        const double d = i * first.spacing();
        for (const double p : {-d, d})
        {
            EXPECT_TRUE(containsSegment(first, {p, -1.0, 0.0}, {p, 1.0, 0.0}));
            EXPECT_TRUE(containsSegment(first, {-1.0, p, 0.0}, {1.0, p, 0.0}));
        }
    }
}

TEST(ReferenceGridTest, SpacingLargerThanExtentProducesNoNonCentralLines)
{
    const ReferenceGrid grid{0.5, 1.0};
    EXPECT_TRUE(grid.vertices().empty());
}

TEST(ReferenceGridTest, ExactBoundaryIsIncludedButOutsideBoundaryIsNot)
{
    EXPECT_EQ((ReferenceGrid{1.0, 1.0}).vertices().size(), 8u);
    EXPECT_TRUE((ReferenceGrid{std::nextafter(1.0, 0.0), 1.0}).vertices().empty());
}

TEST(ReferenceGridTest, NonPositiveParametersAreRejected)
{
    for (const double invalid : {0.0, -0.0, -1.0})
    {
        EXPECT_THROW((ReferenceGrid{invalid, 1.0}), std::invalid_argument);
        EXPECT_THROW((ReferenceGrid{10.0, invalid}), std::invalid_argument);
    }
}

TEST(ReferenceGridTest, NonFiniteParametersAreRejected)
{
    for (const double invalid : {std::numeric_limits<double>::quiet_NaN(),
                                std::numeric_limits<double>::infinity(),
                                -std::numeric_limits<double>::infinity()})
    {
        EXPECT_THROW((ReferenceGrid{invalid, 1.0}), std::invalid_argument);
        EXPECT_THROW((ReferenceGrid{10.0, invalid}), std::invalid_argument);
    }
}

TEST(ReferenceGridTest, ExcessiveCountsAndOverflowFailBeforeAllocation)
{
    EXPECT_THROW((ReferenceGrid{1.0e9, 1.0}), std::length_error);
    EXPECT_THROW((ReferenceGrid{std::numeric_limits<double>::max(), 1.0}), std::length_error);
    EXPECT_THROW((ReferenceGrid{1.0, std::numeric_limits<double>::denorm_min()}), std::length_error);
}

}
