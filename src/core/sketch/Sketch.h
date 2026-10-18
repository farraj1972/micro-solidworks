#pragma once

#include "core/sketch/SketchEntity.h"
#include "core/sketch/SketchConstraint.h"
#include "core/sketch/SketchPlane.h"

#include <optional>
#include <array>
#include <vector>
#include <utility>

namespace microsw::sketch
{

class Sketch
{
public:
    Sketch() = default;
    explicit Sketch(const SketchPlane& plane) : plane_{plane} {}

    [[nodiscard]] const SketchPlane& plane() const noexcept { return plane_; }
    [[nodiscard]] Sketch withPlane(const SketchPlane& plane) const;

    SketchEntityId addLine(const geometry::Segment2& geometry);
    SketchEntityId addCircle(const geometry::Circle2& geometry);
    SketchEntityId addArc(const geometry::Arc2& geometry);
    [[nodiscard]] std::array<SketchEntityId, 4> addRectangle(
        const geometry::Point2& firstCorner,
        const geometry::Point2& oppositeCorner);

    void replaceLine(SketchEntityId id, const geometry::Segment2& geometry);
    void replaceCircle(SketchEntityId id, const geometry::Circle2& geometry);
    void replaceArc(SketchEntityId id, const geometry::Arc2& geometry);
    void remove(SketchEntityId id);
    [[nodiscard]] bool contains(SketchEntityId id) const noexcept;

    [[nodiscard]] const SketchEntity& find(SketchEntityId id) const;
    [[nodiscard]] std::vector<SketchEntityId> entityIds() const;
    [[nodiscard]] std::size_t size() const noexcept { return activeCount_; }

    SketchConstraintId addConstraint(SketchConstraintValue value);
    void removeConstraint(SketchConstraintId id);
    void setDrivingValue(SketchConstraintId id, math::Scalar value);
    [[nodiscard]] bool containsConstraint(SketchConstraintId id) const noexcept;
    [[nodiscard]] const SketchConstraint& findConstraint(SketchConstraintId id) const;
    [[nodiscard]] std::vector<SketchConstraintId> constraintIds() const;
    [[nodiscard]] std::size_t constraintCount() const noexcept { return activeConstraintCount_; }

    using Replacement = std::pair<SketchEntityId, SketchGeometry>;
    void replaceMany(const std::vector<Replacement>& replacements);

    // Controlled reconstruction boundary for persistence. Live records retain
    // their explicit aggregate-local IDs; omitted slots remain tombstones.
    [[nodiscard]] static Sketch restore(
        const SketchPlane& plane,
        std::vector<SketchEntity> entities,
        SketchEntityId::Value nextEntityId,
        std::vector<SketchConstraint> constraints,
        SketchConstraintId::Value nextConstraintId);
    [[nodiscard]] SketchEntityId::Value nextEntityId() const noexcept
        { return static_cast<SketchEntityId::Value>(entities_.size()); }
    [[nodiscard]] SketchConstraintId::Value nextConstraintId() const noexcept
        { return static_cast<SketchConstraintId::Value>(constraints_.size()); }

private:
    SketchEntityId add(SketchGeometry geometry);
    void replace(SketchEntityId id, SketchEntityType expected, SketchGeometry geometry);
    void validateConstraint(const SketchConstraintValue& value) const;

    SketchPlane plane_;
    std::vector<std::optional<SketchEntity>> entities_;
    std::size_t activeCount_{};
    std::vector<std::optional<SketchConstraint>> constraints_;
    std::size_t activeConstraintCount_{};
};

}
