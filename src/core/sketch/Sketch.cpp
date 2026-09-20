#include "core/sketch/Sketch.h"

#include "core/geometry/GeometricTolerance.h"

#include <cmath>
#include <stdexcept>
#include <set>
#include <type_traits>

namespace microsw::sketch
{

SketchEntityId Sketch::add(SketchGeometry geometry)
{
    const auto id = SketchEntityId{static_cast<SketchEntityId::Value>(entities_.size())};
    if (!id.isValid()) throw std::overflow_error{"Sketch entity ID space exhausted"};
    entities_.emplace_back(std::in_place, id, std::move(geometry));
    ++activeCount_;
    return id;
}

SketchEntityId Sketch::addLine(const geometry::Segment2& geometry)
{
    return add(SketchLine{geometry});
}

SketchEntityId Sketch::addCircle(const geometry::Circle2& geometry)
{
    return add(SketchCircle{geometry});
}

SketchEntityId Sketch::addArc(const geometry::Arc2& geometry)
{
    return add(SketchArc{geometry});
}

std::array<SketchEntityId, 4> Sketch::addRectangle(
    const geometry::Point2& firstCorner,
    const geometry::Point2& oppositeCorner)
{
    if (std::abs(oppositeCorner.x() - firstCorner.x()) <= geometry::defaultGeometricTolerance
        || std::abs(oppositeCorner.y() - firstCorner.y()) <= geometry::defaultGeometricTolerance)
        throw std::invalid_argument{"Rectangle width and height must exceed geometric tolerance"};

    const geometry::Point2 second{oppositeCorner.x(), firstCorner.y()};
    const geometry::Point2 fourth{firstCorner.x(), oppositeCorner.y()};
    const auto originalSize = entities_.size();
    const auto originalActiveCount = activeCount_;
    if (originalSize > SketchEntityId::invalidValue - 4)
        throw std::overflow_error{"Sketch entity ID space exhausted"};
    entities_.reserve(originalSize + 4);
    try
    {
        return {addLine({firstCorner, second}), addLine({second, oppositeCorner}),
                addLine({oppositeCorner, fourth}), addLine({fourth, firstCorner})};
    }
    catch (...)
    {
        entities_.resize(originalSize);
        activeCount_ = originalActiveCount;
        throw;
    }
}

void Sketch::replace(SketchEntityId id, SketchEntityType expected, SketchGeometry geometry)
{
    const auto& current = find(id);
    if (current.type() != expected)
        throw std::invalid_argument{"Sketch entity replacement must preserve entity type"};
    entities_[id.value()].emplace(id, std::move(geometry));
}

void Sketch::replaceLine(SketchEntityId id, const geometry::Segment2& geometry)
{
    replace(id, SketchEntityType::Line, SketchLine{geometry});
}

void Sketch::replaceCircle(SketchEntityId id, const geometry::Circle2& geometry)
{
    replace(id, SketchEntityType::Circle, SketchCircle{geometry});
}

void Sketch::replaceArc(SketchEntityId id, const geometry::Arc2& geometry)
{
    replace(id, SketchEntityType::Arc, SketchArc{geometry});
}

void Sketch::remove(SketchEntityId id)
{
    (void)find(id);
    for (const auto& constraint : constraints_)
    {
        if (!constraint) continue;
        bool referenced = false;
        std::visit([&](const auto& value) {
            using T = std::decay_t<decltype(value)>;
            if constexpr (std::is_same_v<T, Horizontal> || std::is_same_v<T, Vertical>
                || std::is_same_v<T, LineLength>) referenced = value.line.entity == id;
            else if constexpr (std::is_same_v<T, CircleRadius>) referenced = value.radius.entity == id;
            else referenced = value.first.entity == id || value.second.entity == id;
        }, constraint->value());
        if (referenced) throw std::invalid_argument{"Cannot remove an entity referenced by a live constraint"};
    }
    entities_[id.value()].reset();
    --activeCount_;
}

namespace
{
bool isPointKind(SubElementKind kind)
{
    return kind == SubElementKind::LineStart || kind == SubElementKind::LineEnd
        || kind == SubElementKind::CircleCenter;
}
}

void Sketch::validateConstraint(const SketchConstraintValue& value) const
{
    const auto validateRef = [&](SketchElementRef ref) {
        const auto& entity = find(ref.entity);
        const bool compatible = (entity.type() == SketchEntityType::Line
            && (ref.kind == SubElementKind::LineStart || ref.kind == SubElementKind::LineEnd
                || ref.kind == SubElementKind::LineBody))
            || (entity.type() == SketchEntityType::Circle
                && (ref.kind == SubElementKind::CircleCenter || ref.kind == SubElementKind::CircleRadius));
        if (!compatible) throw std::invalid_argument{"Sketch element reference is incompatible with entity type"};
    };
    std::visit([&](const auto& constraint) {
        using T = std::decay_t<decltype(constraint)>;
        if constexpr (std::is_same_v<T, Coincident>)
        {
            validateRef(constraint.first); validateRef(constraint.second);
            if (!isPointKind(constraint.first.kind) || !isPointKind(constraint.second.kind))
                throw std::invalid_argument{"Coincident requires point-like references"};
        }
        else if constexpr (std::is_same_v<T, Horizontal> || std::is_same_v<T, Vertical>
            || std::is_same_v<T, LineLength>)
        {
            validateRef(constraint.line);
            if (constraint.line.kind != SubElementKind::LineBody)
                throw std::invalid_argument{"Line constraint requires Line.Body"};
        }
        else if constexpr (std::is_same_v<T, CircleRadius>)
        {
            validateRef(constraint.radius);
            if (constraint.radius.kind != SubElementKind::CircleRadius)
                throw std::invalid_argument{"CircleRadius requires Circle.Radius"};
        }
        else
        {
            validateRef(constraint.first); validateRef(constraint.second);
            if constexpr (std::is_same_v<T, Parallel> || std::is_same_v<T, Perpendicular>)
            {
                if (constraint.first.kind != SubElementKind::LineBody
                    || constraint.second.kind != SubElementKind::LineBody)
                    throw std::invalid_argument{"Directional relation requires Line.Body references"};
            }
            else if (!isPointKind(constraint.first.kind) || !isPointKind(constraint.second.kind))
                throw std::invalid_argument{"Distance dimension requires point-like references"};
        }
        if constexpr (std::is_same_v<T, HorizontalDistance> || std::is_same_v<T, VerticalDistance>)
        {
            if (!std::isfinite(constraint.value))
                throw std::invalid_argument{"Driving dimension must be finite"};
        }
        else if constexpr (std::is_same_v<T, LineLength> || std::is_same_v<T, CircleRadius>)
        {
            if (!std::isfinite(constraint.value)
                || constraint.value <= geometry::defaultGeometricTolerance)
                throw std::invalid_argument{"Length and radius dimensions must exceed geometric tolerance"};
        }
    }, value);
}

SketchConstraintId Sketch::addConstraint(SketchConstraintValue value)
{
    validateConstraint(value);
    const auto id = SketchConstraintId{static_cast<SketchConstraintId::Value>(constraints_.size())};
    if (!id.isValid()) throw std::overflow_error{"Sketch constraint ID space exhausted"};
    constraints_.emplace_back(std::in_place, id, std::move(value));
    ++activeConstraintCount_;
    return id;
}

void Sketch::removeConstraint(SketchConstraintId id)
{
    (void)findConstraint(id);
    constraints_[id.value()].reset();
    --activeConstraintCount_;
}

void Sketch::setDrivingValue(SketchConstraintId id, math::Scalar value)
{
    const auto& current = findConstraint(id);
    auto replacement = withDrivingValue(current.value(), value);
    validateConstraint(replacement);
    constraints_[id.value()].emplace(id, std::move(replacement));
}

bool Sketch::containsConstraint(SketchConstraintId id) const noexcept
{
    return id.isValid() && id.value() < constraints_.size() && constraints_[id.value()].has_value();
}

const SketchConstraint& Sketch::findConstraint(SketchConstraintId id) const
{
    if (!containsConstraint(id)) throw std::out_of_range{"Invalid or removed SketchConstraintId"};
    return *constraints_[id.value()];
}

std::vector<SketchConstraintId> Sketch::constraintIds() const
{
    std::vector<SketchConstraintId> result;
    result.reserve(activeConstraintCount_);
    for (const auto& constraint : constraints_) if (constraint) result.push_back(constraint->id());
    return result;
}

void Sketch::replaceMany(const std::vector<Replacement>& replacements)
{
    std::set<SketchEntityId::Value> ids;
    for (const auto& [id, geometry] : replacements)
    {
        const auto& current = find(id);
        if (!ids.insert(id.value()).second)
            throw std::invalid_argument{"Duplicate entity in Sketch transaction"};
        const auto type = std::visit([](const auto& value) {
            using T = std::decay_t<decltype(value)>;
            if constexpr (std::is_same_v<T, SketchLine>) return SketchEntityType::Line;
            else if constexpr (std::is_same_v<T, SketchCircle>) return SketchEntityType::Circle;
            else return SketchEntityType::Arc;
        }, geometry);
        if (current.type() != type)
            throw std::invalid_argument{"Sketch transaction must preserve entity types"};
    }
    for (const auto& [id, geometry] : replacements)
        entities_[id.value()].emplace(id, geometry);
}

bool Sketch::contains(SketchEntityId id) const noexcept
{
    return id.isValid() && id.value() < entities_.size() && entities_[id.value()].has_value();
}

const SketchEntity& Sketch::find(SketchEntityId id) const
{
    if (!contains(id))
        throw std::out_of_range{"Invalid or removed SketchEntityId"};
    return *entities_[id.value()];
}

std::vector<SketchEntityId> Sketch::entityIds() const
{
    std::vector<SketchEntityId> result;
    result.reserve(activeCount_);
    for (const auto& entity : entities_)
        if (entity) result.push_back(entity->id());
    return result;
}

}
