#include "presentation/SolidPresentation.h"

#include "presentation/SegmentSet3.h"

#include <stdexcept>
#include <utility>

namespace microsw::presentation
{
void SolidPresentation::regenerate(const topology::Solid& solid)
{
    if (!solid.isValid()) throw std::invalid_argument{"Cannot present an invalid Solid"};
    std::vector<geometry::Segment3> segments;
    segments.reserve(solid.edges().size());
    for (std::size_t i = 0; i < solid.edges().size(); ++i)
        segments.push_back(solid.segment(topology::EdgeId{static_cast<topology::EdgeId::Value>(i)}));
    SegmentSet3 wireframe{std::move(segments)};
    if (visualId_) (void)geometry_.setGeometry(*visualId_, std::move(wireframe));
    else visualId_ = geometry_.add(wireframe);
}
}
