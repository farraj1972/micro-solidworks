#include "app/logging/Logger.h"

#include <spdlog/spdlog.h>

namespace microsw
{
void Logger::info(const std::string_view message)
{
    spdlog::info("{}", message);
}

void Logger::warn(const std::string_view message)
{
    spdlog::warn("{}", message);
}

void Logger::error(const std::string_view message)
{
    spdlog::error("{}", message);
}
}
