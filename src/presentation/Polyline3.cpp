#include "presentation/Polyline3.h"

#include <stdexcept>

namespace microsw::presentation
{
Polyline3::Polyline3(std::vector<geometry::Point3> points) : points_{std::move(points)}
{
    if (points_.size() < 2)
        throw std::invalid_argument{"Polyline3 requires at least two points"};
}
}
