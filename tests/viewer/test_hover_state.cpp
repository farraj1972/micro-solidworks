#include "viewer/HoverState.h"

#include <gtest/gtest.h>

namespace
{
using microsw::viewer::HoverState;
using microsw::presentation::VisualEntityId;

TEST(HoverState, DefaultsToNoneAndAcceptsOneIdentity)
{
    HoverState state;
    EXPECT_FALSE(state.hovered());
    state.update(VisualEntityId{7});
    ASSERT_TRUE(state.hovered());
    EXPECT_EQ(*state.hovered(), VisualEntityId{7});
}

TEST(HoverState, SameIdentityIsStableAndDifferentIdentityReplacesIt)
{
    HoverState state;
    state.update(VisualEntityId{7});
    state.update(VisualEntityId{7});
    EXPECT_EQ(state.hovered(), VisualEntityId{7});
    state.update(VisualEntityId{19});
    EXPECT_EQ(state.hovered(), VisualEntityId{19});
}

TEST(HoverState, NoCandidateAndExplicitClearRemoveIdentity)
{
    HoverState state;
    state.update(VisualEntityId{7});
    state.update(std::nullopt);
    EXPECT_FALSE(state.hovered());
    state.update(VisualEntityId{19});
    state.clear();
    EXPECT_FALSE(state.hovered());
    state.clear();
    EXPECT_FALSE(state.hovered());
}
}
