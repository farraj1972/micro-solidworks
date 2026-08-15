#include "app/logging/Logger.h"

#include <gtest/gtest.h>

TEST(Logging, SupportsRequiredLevels)
{
    EXPECT_NO_THROW(microsw::Logger::info("Logging test info"));
    EXPECT_NO_THROW(microsw::Logger::warn("Logging test warning"));
    EXPECT_NO_THROW(microsw::Logger::error("Logging test error"));
}
