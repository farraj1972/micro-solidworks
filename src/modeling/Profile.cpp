#include "modeling/Profile.h"

#include "core/geometry/GeometricTolerance.h"
#include "core/math/Vector3.h"

#include <stdexcept>

namespace microsw::modeling
{
Profile::Profile(std::vector<geometry::Point3> boundary, const geometry::Plane& supportPlane)
    : boundary_{std::move(boundary)}, supportPlane_{supportPlane}
{
    if (boundary_.size() < 3)
        throw std::invalid_argument{"Profile requires at least three boundary points"};
    for (const auto& point : boundary_)
        if (!supportPlane_.contains(point))
            throw std::invalid_argument{"Profile boundary must lie on its support Plane"};

    for (std::size_t i = 0; i < boundary_.size(); ++i)
    {
        const auto& a = boundary_[i];
        const auto& b = boundary_[(i + 1) % boundary_.size()];
        const auto& c = boundary_[(i + 2) % boundary_.size()];
        const geometry::Segment3 incoming{a, b};
        const geometry::Segment3 outgoing{b, c};
        if (incoming.isDegenerate() || outgoing.isDegenerate())
            throw std::invalid_argument{"Profile edges must be nondegenerate"};
        const auto turn = math::dot(
            math::cross(incoming.direction(), outgoing.direction()), supportPlane_.normal());
        if (turn <= geometry::defaultGeometricTolerance)
            throw std::invalid_argument{"Profile boundary must be strictly convex and CCW"};
    }
}

std::vector<geometry::Segment3> Profile::segments() const
{
    std::vector<geometry::Segment3> result;
    result.reserve(boundary_.size());
    for (std::size_t i = 0; i < boundary_.size(); ++i)
        result.emplace_back(boundary_[i], boundary_[(i + 1) % boundary_.size()]);
    return result;
}
}
