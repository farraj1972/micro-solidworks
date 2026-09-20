#include "app/modeling/ActiveExtrusion.h"
#include "core/sketch/Sketch.h"
#include "presentation/SegmentSet3.h"
#include "constraints/ConstraintSolver.h"
#include "core/geometry/GeometricTolerance.h"

#include <gtest/gtest.h>

#include <algorithm>

namespace
{
using namespace microsw;

sketch::Sketch rectangle()
{
    sketch::Sketch result;
    (void)result.addLine({{0, 0}, {4, 0}});
    (void)result.addLine({{4, 3}, {0, 3}});
    (void)result.addLine({{4, 0}, {4, 3}});
    (void)result.addLine({{0, 3}, {0, 0}});
    return result;
}

TEST(ExtrusionVerticalSlice, RegeneratesOneStableSelectableSolidIdentity)
{
    const auto sketch = rectangle();
    ActiveExtrusion active;
    active.regenerate(sketch, 2);
    ASSERT_TRUE(active.hasSolid());
    ASSERT_NE(active.solid(), nullptr);
    EXPECT_TRUE(active.solid()->isValid());
    const auto id = *active.presentation().visualId();
    ASSERT_EQ(active.presentation().geometry().size(), 1u);
    EXPECT_EQ(std::get<presentation::SegmentSet3>(
        active.presentation().geometry().entities().front().geometry()).segments().size(), 12u);

    active.regenerate(sketch, 5);
    EXPECT_EQ(*active.presentation().visualId(), id);
    EXPECT_EQ(active.solid()->vertices().at(4).point().z(), 5);
}

TEST(ExtrusionVerticalSlice, RejectedRegenerationPreservesPreviousResult)
{
    auto sketch = rectangle();
    ActiveExtrusion active;
    active.regenerate(sketch, 2);
    const auto id = *active.presentation().visualId();
    const auto previousTop = active.solid()->vertices().at(4).point();

    EXPECT_THROW(active.regenerate(sketch, 0), std::invalid_argument);
    EXPECT_EQ(*active.presentation().visualId(), id);
    EXPECT_EQ(active.solid()->vertices().at(4).point().z(), previousTop.z());

    (void)sketch.addCircle({{2, 2}, 1});
    EXPECT_THROW(active.regenerate(sketch, 4), std::invalid_argument);
    EXPECT_EQ(*active.presentation().visualId(), id);
    EXPECT_EQ(active.solid()->vertices().at(4).point().z(), previousTop.z());
}

TEST(ExtrusionVerticalSlice, SolvedRectangleDimensionsFlowToManualRegeneration)
{
    sketch::Sketch sketch;
    const auto lines = sketch.addRectangle({0, 0}, {10, 5});
    const auto start = [](sketch::SketchEntityId id) {
        return sketch::SketchElementRef{id, sketch::SubElementKind::LineStart};
    };
    const auto end = [](sketch::SketchEntityId id) {
        return sketch::SketchElementRef{id, sketch::SubElementKind::LineEnd};
    };
    const auto body = [](sketch::SketchEntityId id) {
        return sketch::SketchElementRef{id, sketch::SubElementKind::LineBody};
    };
    for (std::size_t i = 0; i < lines.size(); ++i)
        (void)sketch.addConstraint(sketch::Coincident{end(lines[i]), start(lines[(i + 1) % lines.size()])});
    (void)sketch.addConstraint(sketch::Horizontal{body(lines[0])});
    (void)sketch.addConstraint(sketch::Horizontal{body(lines[2])});
    (void)sketch.addConstraint(sketch::Vertical{body(lines[1])});
    (void)sketch.addConstraint(sketch::Vertical{body(lines[3])});
    const auto width = sketch.addConstraint(sketch::HorizontalDistance{start(lines[0]), end(lines[0]), 10});
    const auto height = sketch.addConstraint(sketch::VerticalDistance{start(lines[1]), end(lines[1]), 5});
    constraints::SolverPolicy policy;
    policy.residualSatisfactionTolerance = geometry::defaultGeometricTolerance;
    policy.stepTolerance = geometry::defaultGeometricTolerance * 0.01;
    policy.costReductionTolerance = 1e-16;
    policy.initialStateRegularization = 1e-12;
    policy.maxIterations = 200;
    constraints::ConstraintSolver solver{policy};
    ASSERT_EQ(solver.solve(sketch).status, constraints::SolveStatus::Solved);

    ActiveExtrusion active;
    active.regenerate(sketch, 3);
    EXPECT_EQ(active.solid()->vertices().size(), 8u);
    EXPECT_EQ(active.solid()->edges().size(), 12u);
    EXPECT_EQ(active.solid()->faces().size(), 6u);
    const auto id = *active.presentation().visualId();

    sketch.setDrivingValue(width, 14);
    sketch.setDrivingValue(height, 8);
    ASSERT_EQ(solver.solve(sketch).status, constraints::SolveStatus::Solved);
    active.regenerate(sketch, 6);
    EXPECT_EQ(*active.presentation().visualId(), id);
    double minX = 1e9, minY = 1e9, minZ = 1e9;
    double maxX = -1e9, maxY = -1e9, maxZ = -1e9;
    for (const auto& vertex : active.solid()->vertices())
    {
        minX = std::min(minX, vertex.point().x());
        minY = std::min(minY, vertex.point().y());
        minZ = std::min(minZ, vertex.point().z());
        maxX = std::max(maxX, vertex.point().x());
        maxY = std::max(maxY, vertex.point().y());
        maxZ = std::max(maxZ, vertex.point().z());
    }
    EXPECT_NEAR(maxX - minX, 14, 1e-6);
    EXPECT_NEAR(maxY - minY, 8, 1e-6);
    EXPECT_NEAR(maxZ - minZ, 6, 1e-9);
}
}
