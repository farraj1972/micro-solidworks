#include "features/AxialCut.h"

#include <gtest/gtest.h>

using microsw::features::AxialCutAlignment;
using microsw::features::classifyAxialCut;
using microsw::math::Vector3;

TEST(AxialCut, AcceptsParallelAndAntiParallelDirections)
{
    EXPECT_EQ(classifyAxialCut({0, 0, 4}, {0, 0, 1}),
        AxialCutAlignment::Parallel);
    EXPECT_EQ(classifyAxialCut({0, 0, -2}, {0, 0, 1}),
        AxialCutAlignment::AntiParallel);
}

TEST(AxialCut, UsesToleranceAndRejectsObliqueDirections)
{
    EXPECT_EQ(classifyAxialCut({5.0e-10, 0, 1}, {0, 0, 1}),
        AxialCutAlignment::Parallel);
    EXPECT_EQ(classifyAxialCut({2.0e-9, 0, 1}, {0, 0, 1}),
        AxialCutAlignment::Unsupported);
    EXPECT_EQ(classifyAxialCut({1, 0, 1}, {0, 0, 1}),
        AxialCutAlignment::Unsupported);
}

TEST(AxialCut, RejectsInvalidDirections)
{
    EXPECT_THROW((void)classifyAxialCut({}, {0, 0, 1}), std::domain_error);
    EXPECT_THROW((void)classifyAxialCut({0, 0, 1}, {}), std::domain_error);
}
