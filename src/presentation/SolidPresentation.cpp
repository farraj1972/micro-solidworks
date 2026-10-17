#include "presentation/SolidPresentation.h"

#include "presentation/SegmentSet3.h"

#include <stdexcept>
#include <type_traits>
#include <utility>

namespace microsw::presentation
{
SegmentSet3 deriveSolidWireframe(const topology::Solid& solid)
{
    if (!solid.isValid()) throw std::invalid_argument{"Cannot present an invalid Solid"};
    std::vector<geometry::Segment3> segments;
    segments.reserve(solid.edges().size());
    for (const auto& edge : solid.edges())
        std::visit([&](const auto& geometry) {
            using T = std::decay_t<decltype(geometry)>;
            if constexpr (std::is_same_v<T, geometry::Segment3>) segments.push_back(geometry);
            else
            {
                auto previous = geometry.pointAt(0);
                for (int i = 1; i <= 24; ++i)
                {
                    const auto current = geometry.pointAt(static_cast<double>(i) / 24.0);
                    segments.emplace_back(previous, current); previous = current;
                }
            }
        }, edge.geometry());
    return SegmentSet3{std::move(segments)};
}

void SolidPresentation::regenerate(const topology::Solid& solid)
{
    auto wireframe = deriveSolidWireframe(solid);
    if (visualId_) (void)geometry_.setGeometry(*visualId_, std::move(wireframe));
    else visualId_ = geometry_.add(wireframe);
}
}
