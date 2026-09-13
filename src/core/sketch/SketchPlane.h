#pragma once

#include "core/geometry/Plane.h"
#include "core/geometry/Point2.h"

namespace microsw::sketch
{

class SketchPlane
{
public:
    SketchPlane() = default;
    SketchPlane(const geometry::Point3& origin,
                const math::Vector3& xAxis,
                const math::Vector3& yAxis);

    [[nodiscard]] const geometry::Point3& origin() const noexcept { return origin_; }
    [[nodiscard]] const math::Vector3& xAxis() const noexcept { return xAxis_; }
    [[nodiscard]] const math::Vector3& yAxis() const noexcept { return yAxis_; }
    [[nodiscard]] math::Vector3 normal() const noexcept;

    [[nodiscard]] geometry::Point3 toWorld(const geometry::Point2& point) const;
    [[nodiscard]] math::Vector3 toWorld(const math::Vector2& vector) const;
    [[nodiscard]] geometry::Point2 toLocal(const geometry::Point3& point) const;
    [[nodiscard]] geometry::Plane plane() const;

private:
    geometry::Point3 origin_{};
    math::Vector3 xAxis_{1, 0, 0};
    math::Vector3 yAxis_{0, 1, 0};
};

}
