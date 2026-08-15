#include "app/logging/Logger.h"

int main()
{
    microsw::Logger::info("Application starting");
    microsw::Logger::info("Application shutting down");

    return 0;
}
