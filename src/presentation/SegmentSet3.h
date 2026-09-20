#pragma once

#include "core/geometry/Segment3.h"

#include <vector>

namespace microsw::presentation
{
class SegmentSet3
{
public:
    explicit SegmentSet3(std::vector<geometry::Segment3> segments);
    [[nodiscard]] const std::vector<geometry::Segment3>& segments() const noexcept { return segments_; }
private:
    std::vector<geometry::Segment3> segments_;
};
}
