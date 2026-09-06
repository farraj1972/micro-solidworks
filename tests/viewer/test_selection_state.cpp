#include "viewer/SelectionState.h"

#include <gtest/gtest.h>

namespace
{
using microsw::viewer::SelectionState;
using microsw::presentation::VisualEntityId;

TEST(SelectionState, DefaultsToNoneAndSelectsIdentity)
{
    SelectionState state;
    EXPECT_FALSE(state.selected());
    state.select(VisualEntityId{7});
    EXPECT_EQ(state.selected(), VisualEntityId{7});
}

TEST(SelectionState, SameIdentityDoesNotToggleAndAnotherReplacesIt)
{
    SelectionState state;
    state.select(VisualEntityId{7});
    state.select(VisualEntityId{7});
    EXPECT_EQ(state.selected(), VisualEntityId{7});
    state.select(VisualEntityId{19});
    EXPECT_EQ(state.selected(), VisualEntityId{19});
}

TEST(SelectionState, ClearRemovesIdentityAndIsRepeatable)
{
    SelectionState state;
    state.select(VisualEntityId{7});
    state.clear();
    EXPECT_FALSE(state.selected());
    state.clear();
    EXPECT_FALSE(state.selected());
}
}
