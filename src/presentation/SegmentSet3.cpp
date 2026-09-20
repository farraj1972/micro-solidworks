#include "presentation/SegmentSet3.h"

#include <stdexcept>
#include <utility>

namespace microsw::presentation
{
SegmentSet3::SegmentSet3(std::vector<geometry::Segment3> segments) : segments_{std::move(segments)}
{
    if (segments_.empty()) throw std::invalid_argument{"SegmentSet3 requires at least one segment"};
}
}
