#pragma once

#include "core/sketch/Sketch.h"

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
    [[nodiscard]] std::vector<sketch::SketchEntityId> click(const geometry::Point2& point);
    void cancel() noexcept { pending_.clear(); }

private:
    sketch::Sketch& sketch_;
    SketchTool tool_{SketchTool::Select};
    std::vector<geometry::Point2> pending_;
};

}
