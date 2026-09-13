#pragma once

#include "core/geometry/Point3.h"

#include <utility>
#include <vector>

namespace microsw::presentation
{

class Polyline3
{
public:
    explicit Polyline3(std::vector<geometry::Point3> points);
    [[nodiscard]] const std::vector<geometry::Point3>& points() const noexcept { return points_; }

private:
    std::vector<geometry::Point3> points_;
};

}
