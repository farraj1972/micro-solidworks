#pragma once

#include "core/sketch/Sketch.h"

#include <array>
#include <optional>
#include <vector>

namespace microsw
{

enum class SketchTool { Select, Line, Rectangle, Circle, Arc };

class SketchToolController
{
public:
    explicit SketchToolController(sketch::Sketch& sketch) : sketch_{sketch} {}

    void setTool(SketchTool tool) noexcept;
    [[nodiscard]] SketchTool tool() const noexcept { return tool_; }
    [[nodiscard]] std::size_t pendingPointCount() const noexcept { return pending_.size(); }
    [[nodiscard]] std::optional<geometry::Point2> pendingPoint() const noexcept
        { return pending_.empty() ? std::nullopt : std::optional{pending_.back()}; }
    [[nodiscard]] std::vector<sketch::SketchEntityId> click(const geometry::Point2& point);
    [[nodiscard]] std::array<sketch::SketchEntityId, 4> addCenteredRectangle(
        const geometry::Point2& center, math::Scalar width, math::Scalar height);
    void cancel() noexcept { pending_.clear(); }

private:
    sketch::Sketch& sketch_;
    SketchTool tool_{SketchTool::Select};
    std::vector<geometry::Point2> pending_;
};

}
