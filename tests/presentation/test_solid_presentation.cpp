#include "modeling/Extrusion.h"
#include "presentation/SolidPresentation.h"
#include "presentation/SegmentSet3.h"
#include "viewer/PresentedSegments.h"
#include "viewer/GeometryPicker.h"

#include <gtest/gtest.h>

namespace
{
using namespace microsw;

TEST(SolidPresentation, UsesOneStableIdentityAndEveryUniqueTopologyEdge)
{
    const modeling::Profile profile{{{0,0,0},{4,0,0},{4,3,0},{0,3,0}}, {{0,0,0},{0,0,1}}};
    presentation::SolidPresentation presented;
    auto first = modeling::extrude(profile, 2);
    presented.regenerate(first);
    ASSERT_TRUE(presented.visualId());
    const auto id = *presented.visualId();
    ASSERT_EQ(presented.geometry().size(), 1u);
    const auto& set = std::get<presentation::SegmentSet3>(presented.geometry().entities().front().geometry());
    EXPECT_EQ(set.segments().size(), 12u);
    EXPECT_EQ(viewer::presentedSegmentVertices(presented.geometry()).size(), 24u);
    viewer::PickingContext picking;
    picking.camera = viewer::OrbitCamera{{}, 10, 0, 0};
    picking.width = 800;
    picking.height = 600;
    picking.mouseX = 400;
    picking.mouseY = 300;
    const auto hit = viewer::pickGeometry(presented.geometry(), picking);
    ASSERT_TRUE(hit);
    EXPECT_EQ(hit->id, id);

    auto second = modeling::extrude(profile, 5);
    presented.regenerate(second);
    EXPECT_EQ(*presented.visualId(), id);
    EXPECT_EQ(presented.geometry().size(), 1u);
}
}
