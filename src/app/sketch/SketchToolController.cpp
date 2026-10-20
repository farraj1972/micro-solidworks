#include "app/sketch/SketchToolController.h"

#include <cmath>
#include <numbers>
#include <stdexcept>

namespace microsw
{

void SketchToolController::setTool(SketchTool tool) noexcept
{
    tool_ = tool;
    pending_.clear();
}

std::array<sketch::SketchEntityId, 4> SketchToolController::addCenteredRectangle(
    const geometry::Point2& center, math::Scalar width, math::Scalar height)
{
    if (!std::isfinite(width) || !std::isfinite(height)
        || width <= geometry::defaultGeometricTolerance
        || height <= geometry::defaultGeometricTolerance)
        throw std::invalid_argument{"Rectangle width and height must be positive finite values"};
    const geometry::Point2 first{center.x() - width / 2, center.y() - height / 2};
    const geometry::Point2 opposite{center.x() + width / 2, center.y() + height / 2};
    const auto ids = sketch_.addRectangle(first, opposite);
    pending_.clear();
    return ids;
}

std::vector<sketch::SketchEntityId> SketchToolController::click(const geometry::Point2& point)
{
    if (tool_ == SketchTool::Select) return {};
    pending_.push_back(point);
    try
    {
        if (tool_ == SketchTool::Line && pending_.size() == 2)
        {
            const auto id = sketch_.addLine({pending_[0], pending_[1]});
            pending_.clear();
            return {id};
        }
        if (tool_ == SketchTool::Rectangle && pending_.size() == 2)
        {
            const auto ids = sketch_.addRectangle(pending_[0], pending_[1]);
            pending_.clear();
            return {ids.begin(), ids.end()};
        }
        if (tool_ == SketchTool::Circle && pending_.size() == 2)
        {
            const auto radius = std::hypot(pending_[1].x() - pending_[0].x(),
                                           pending_[1].y() - pending_[0].y());
            const auto id = sketch_.addCircle({pending_[0], radius});
            pending_.clear();
            return {id};
        }
        if (tool_ == SketchTool::Arc && pending_.size() == 3)
        {
            const auto sx = pending_[1].x() - pending_[0].x();
            const auto sy = pending_[1].y() - pending_[0].y();
            const auto ex = pending_[2].x() - pending_[0].x();
            const auto ey = pending_[2].y() - pending_[0].y();
            const auto radius = std::hypot(sx, sy);
            const auto start = std::atan2(sy, sx);
            constexpr auto twoPi = 2.0 * std::numbers::pi_v<math::Scalar>;
            auto sweep = std::fmod(std::atan2(ey, ex) - start, twoPi);
            if (sweep <= 0) sweep += twoPi;
            const auto id = sketch_.addArc({pending_[0], radius, start, sweep});
            pending_.clear();
            return {id};
        }
    }
    catch (...)
    {
        pending_.clear();
        throw;
    }
    return {};
}

}
