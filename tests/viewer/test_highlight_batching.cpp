#include "viewer/VisualState.h"
#include "viewer/HighlightColors.h"
#include "viewer/PresentedPoints.h"
#include "viewer/PresentedSegments.h"
#include "viewer/PresentedLines.h"
#include "presentation/GeometryPresentation.h"

#include <gtest/gtest.h>
#include <array>

namespace
{
using namespace microsw::viewer;
using microsw::presentation::GeometryPresentation;
using microsw::presentation::VisualEntityId;
using microsw::geometry::Point3;
using microsw::geometry::Segment3;
using microsw::geometry::Line3;
using microsw::math::Vector3;

TEST(VisualState, ResolvesNoneHoverSelectionAndSelectedPrecedence)
{
    const VisualEntityId a{1}, b{2}, other{3};
    EXPECT_EQ(visualStateFor(a, {}, {}), VisualState::Normal);
    EXPECT_EQ(visualStateFor(a, a, {}), VisualState::Hovered);
    EXPECT_EQ(visualStateFor(a, {}, a), VisualState::Selected);
    EXPECT_EQ(visualStateFor(a, a, a), VisualState::Selected);
    EXPECT_EQ(visualStateFor(a, a, b), VisualState::Hovered);
    EXPECT_EQ(visualStateFor(b, a, b), VisualState::Selected);
    EXPECT_EQ(visualStateFor(other, a, b), VisualState::Normal);
}

void expectVertices(const std::vector<Vector3>& actual, const std::vector<Vector3>& expected)
{
    ASSERT_EQ(actual.size(), expected.size());
    for (std::size_t i = 0; i < actual.size(); ++i)
        EXPECT_TRUE(microsw::math::almostEqual(actual[i], expected[i]));
}

// The unfiltered B4 adapters are the coordinate oracle; state only partitions data.
void checkPartition(int kind)
{
    GeometryPresentation presentation;
    std::vector<VisualEntityId> ids;
    for (int i = 0; i < 4; ++i)
    {
        const Point3 a{double(i), 2, 3}, b{double(i), 4, 5};
        if (kind == 0) ids.push_back(presentation.add(a));
        if (kind == 1) ids.push_back(presentation.add(Segment3{a, b}));
        if (kind == 2) ids.push_back(presentation.add(Line3{a, Vector3{0, 1, 1}}));
        // Unrelated primitive alternatives must not enter this batch.
        if (kind == 0) (void)presentation.add(Segment3{a, b});
        else (void)presentation.add(a);
    }
    for (const LinePresentationContext context : {LinePresentationContext{{}, 2},
                                                   LinePresentationContext{{9, -3, 4}, 5}})
    {
        const auto vertices = [&](std::optional<VisualStateFilter> filter = {})
        {
            if (kind == 0) return presentedPointVertices(presentation, filter);
            if (kind == 1) return presentedSegmentVertices(presentation, filter);
            return presentedLineVertices(presentation, context, filter);
        };
        const auto original = vertices();
        const std::size_t stride = kind == 0 ? 1 : 2;
        ASSERT_EQ(original.size(), 4 * stride);
        for (bool sameIdentity : {false, true})
        {
            std::size_t total = 0;
            for (auto state : {VisualState::Normal, VisualState::Hovered, VisualState::Selected})
            {
                const auto batch = vertices(VisualStateFilter{state,
                    sameIdentity ? ids[2] : ids[1], ids[2]});
                std::vector<Vector3> expected;
                for (std::size_t i = 0; i < ids.size(); ++i)
                {
                    const auto expectedState = i == 2 ? VisualState::Selected
                        : (i == 1 && !sameIdentity ? VisualState::Hovered : VisualState::Normal);
                    if (state == expectedState)
                        expected.insert(expected.end(), original.begin() + i * stride,
                            original.begin() + (i + 1) * stride);
                }
                expectVertices(batch, expected);
                total += batch.size();
            }
            EXPECT_EQ(total, original.size());
        }
        expectVertices(vertices(VisualStateFilter{VisualState::Normal, {}, {}}), original);
    }
}

TEST(HighlightBatching, PointsHaveExclusiveMembershipAndUnchangedCoordinates) { checkPartition(0); }
TEST(HighlightBatching, SegmentsPreservePairsAndInsertionOrderInEachState) { checkPartition(1); }
TEST(HighlightBatching, LinesPreserveViewDerivedEndpointsAcrossStateChanges) { checkPartition(2); }

TEST(HighlightBatching, TransitionsRestoreNormalAndHoverAfterSelectionClears)
{
    GeometryPresentation presentation;
    const auto a = presentation.add(Point3{1, 2, 3});
    const auto b = presentation.add(Point3{4, 5, 6});
    struct Step { std::optional<VisualEntityId> hover, selected; std::array<std::size_t, 3> counts; };
    for (const auto& step : {Step{{}, {}, {2, 0, 0}}, Step{a, {}, {1, 1, 0}},
             Step{a, a, {1, 0, 1}}, Step{b, a, {0, 1, 1}}, Step{{}, a, {1, 0, 1}},
             Step{{}, {}, {2, 0, 0}}, Step{a, {}, {1, 1, 0}}})
    {
        std::size_t index = 0;
        for (auto state : {VisualState::Normal, VisualState::Hovered, VisualState::Selected})
            EXPECT_EQ(presentedPointVertices(presentation,
                VisualStateFilter{state, step.hover, step.selected}).size(), step.counts[index++]);
    }
}

TEST(HighlightBatching, EmptyPresentationAndUnknownIdentitiesAreSafe)
{
    GeometryPresentation presentation;
    for (auto state : {VisualState::Normal, VisualState::Hovered, VisualState::Selected})
    {
        const VisualStateFilter filter{state, VisualEntityId{100}, VisualEntityId{200}};
        EXPECT_TRUE(presentedPointVertices(presentation, filter).empty());
        EXPECT_TRUE(presentedSegmentVertices(presentation, filter).empty());
        EXPECT_TRUE(presentedLineVertices(presentation, {{}, 1}, filter).empty());
    }
    (void)presentation.add(Point3{});
    EXPECT_EQ(presentedPointVertices(presentation,
        VisualStateFilter{VisualState::Normal, VisualEntityId{100}, VisualEntityId{200}}).size(), 1U);
}

TEST(HighlightColors, NormalPaletteIsPreservedAndHighlightColorsAreDistinct)
{
    EXPECT_TRUE(microsw::math::almostEqual(normalPointColor, Vector3{1, 0.85, 0.2}));
    EXPECT_TRUE(microsw::math::almostEqual(normalSegmentColor, Vector3{0.8, 0.8, 0.85}));
    EXPECT_TRUE(microsw::math::almostEqual(normalLineColor, Vector3{0.2, 0.75, 0.85}));
    for (const auto normal : {normalPointColor, normalSegmentColor, normalLineColor})
    {
        EXPECT_TRUE(microsw::math::almostEqual(highlightColor(VisualState::Normal, normal), normal));
        EXPECT_FALSE(microsw::math::almostEqual(highlightColor(VisualState::Hovered, normal), normal));
        EXPECT_FALSE(microsw::math::almostEqual(highlightColor(VisualState::Selected, normal), normal));
        EXPECT_FALSE(microsw::math::almostEqual(hoveredGeometryColor, selectedGeometryColor));
    }
}
}
