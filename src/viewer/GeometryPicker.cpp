#include "viewer/GeometryPicker.h"
#include "presentation/WorldGeometry.h"

#include "presentation/GeometryPresentation.h"
#include "viewer/PresentedLines.h"
#include "viewer/ViewProjection.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <stdexcept>
#include <limits>
#include <variant>

namespace microsw::viewer
{
namespace
{
using math::Scalar;

Scalar checked(Scalar value)
{
    if (!std::isfinite(value))
        throw std::overflow_error{"Picking projection arithmetic is not representable"};
    return value;
}

// Local homogeneous coordinates for clipping; not a new Math vector type.
struct ClipVertex
{
    std::array<Scalar, 4> clip;
    Scalar depth;
};

class Projection
{
public:
    explicit Projection(const PickingContext& context)
        : view_{viewMatrix(context.camera)},
          projection_{context.projection.matrix(context.verticalFov,
              context.aspectRatio(), context.nearPlane, context.farPlane)},
          width_{context.width}, height_{context.height} {}

    ClipVertex vertex(const geometry::Point3& point) const
    {
        const std::array<Scalar, 4> world{point.x(), point.y(), point.z(), 1};
        const auto camera = multiply(view_, world);
        return {multiply(projection_, camera), -camera[2]};
    }

    ScreenPoint screen(const ClipVertex& vertex) const
    {
        const auto& c = vertex.clip;
        // Clipping can leave a few rounding bits beyond a boundary.
        const auto x = std::clamp(checked(c[0] / c[3]), -1.0, 1.0);
        const auto y = std::clamp(checked(c[1] / c[3]), -1.0, 1.0);
        return {checked((0.5 * x + 0.5) * width_),
                checked((0.5 - 0.5 * y) * height_), vertex.depth};
    }

private:
    static std::array<Scalar, 4> multiply(
        const math::Matrix4& matrix, const std::array<Scalar, 4>& value)
    {
        std::array<Scalar, 4> result{};
        for (std::size_t row = 0; row < 4; ++row)
        {
            for (std::size_t column = 0; column < 4; ++column)
                result[row] += checked(matrix(row, column) * value[column]);
            result[row] = checked(result[row]);
        }
        return result;
    }

    math::Matrix4 view_, projection_;
    Scalar width_, height_;
};

Scalar planeDistance(const ClipVertex& vertex, std::size_t plane)
{
    // Half-scaled w +/- coordinate avoids overflow in the plane sum.
    const auto coordinate = vertex.clip[plane / 2];
    const auto distance = vertex.clip[3] * 0.5
        + (plane % 2 == 0 ? 0.5 : -0.5) * coordinate;
    // Account only for floating-point projection roundoff at a clip boundary.
    // This is neither Geometry tolerance nor the pixel-based hit tolerance.
    constexpr Scalar roundoff = 32 * std::numeric_limits<Scalar>::epsilon();
    const auto scale = std::max(std::abs(vertex.clip[3]), std::abs(coordinate));
    return std::abs(distance) <= roundoff * scale ? 0.0 : distance;
}

bool inside(const ClipVertex& vertex)
{
    if (vertex.clip[3] <= 0 || vertex.depth <= 0)
        return false;
    for (std::size_t plane = 0; plane < 6; ++plane)
        if (planeDistance(vertex, plane) < 0)
            return false;
    return true;
}

ClipVertex interpolate(const ClipVertex& a, const ClipVertex& b, Scalar t)
{
    ClipVertex result{};
    for (std::size_t i = 0; i < 4; ++i)
        result.clip[i] = checked(std::lerp(a.clip[i], b.clip[i], t));
    result.depth = checked(std::lerp(a.depth, b.depth, t));
    return result;
}

bool clipSegment(ClipVertex& a, ClipVertex& b)
{
    // Clip before dividing by w: endpoints behind the eye or outside the
    // viewport must not discard a segment that crosses the visible frustum.
    for (std::size_t plane = 0; plane < 6; ++plane)
    {
        const auto da = planeDistance(a, plane);
        const auto db = planeDistance(b, plane);
        if (da < 0 && db < 0)
            return false;
        if ((da < 0) != (db < 0))
        {
            const auto scale = std::max(std::abs(da), std::abs(db));
            const auto t = (da / scale) / (da / scale - db / scale);
            const auto intersection = interpolate(a, b, t);
            if (da < 0) a = intersection; else b = intersection;
        }
    }
    return a.clip[3] > 0 && b.clip[3] > 0 && a.depth > 0 && b.depth > 0;
}

struct Candidate
{
    Scalar distance;
    Scalar depth;
};

Candidate pointCandidate(const ScreenPoint& point, const PickingContext& context)
{
    return {checked(std::hypot(context.mouseX - point.x, context.mouseY - point.y)), point.depth};
}

std::optional<Candidate> segmentCandidate(const geometry::Point3& start,
    const geometry::Point3& end, const Projection& projection, const PickingContext& context)
{
    auto a = projection.vertex(start);
    auto b = projection.vertex(end);
    if (!clipSegment(a, b))
        return std::nullopt;
    const auto sa = projection.screen(a);
    const auto sb = projection.screen(b);
    const auto dx = sb.x - sa.x;
    const auto dy = sb.y - sa.y;
    const auto length = checked(std::hypot(dx, dy));
    if (length == 0)
        return Candidate{pointCandidate(sa, context).distance, std::min(sa.depth, sb.depth)};

    const auto along = checked((context.mouseX - sa.x) * (dx / length)
                             + (context.mouseY - sa.y) * (dy / length));
    const auto t = std::clamp(along, 0.0, length) / length;
    auto depth = std::lerp(sa.depth, sb.depth, t);
    if (context.projection.mode() == ProjectionMode::Perspective && t > 0 && t < 1)
    {
        // Screen interpolation is perspective-correct in reciprocal depth.
        const auto scale = std::min(sa.depth, sb.depth);
        depth = checked(scale / ((1.0 - t) * (scale / sa.depth) + t * (scale / sb.depth)));
    }
    return pointCandidate({std::lerp(sa.x, sb.x, t), std::lerp(sa.y, sb.y, t), depth}, context);
}
}

std::optional<ScreenPoint> projectWorldToScreen(
    const geometry::Point3& point, const PickingContext& context)
{
    context.validate();
    const Projection projection{context};
    const auto vertex = projection.vertex(point);
    return inside(vertex) ? std::optional{projection.screen(vertex)} : std::nullopt;
}

std::optional<PickHit> pickGeometry(
    const presentation::GeometryPresentation& presentation, const PickingContext& context)
{
    context.validate();
    if (context.mouseX < 0 || context.mouseX >= context.width
        || context.mouseY < 0 || context.mouseY >= context.height || presentation.empty())
        return std::nullopt;

    const Projection projection{context};
    std::vector<math::Vector3> lineVertices;
    std::size_t lineIndex = 0;
    std::optional<PickHit> best;
    Scalar bestDepth{};
    constexpr Scalar distanceTiePixels = 1e-7; // Numerical tie, not a hit tolerance.

    for (const auto& entity : presentation.entities())
    {
        std::optional<Candidate> candidate;
        const auto world = presentation::worldGeometry(entity);
        if (const auto* point = std::get_if<geometry::Point3>(&world))
        {
            const auto vertex = projection.vertex(*point);
            if (inside(vertex))
                candidate = pointCandidate(projection.screen(vertex), context);
        }
        else if (const auto* segment = std::get_if<geometry::Segment3>(&world))
            candidate = segmentCandidate(segment->a(), segment->b(), projection, context);
        else if (std::holds_alternative<geometry::Line3>(entity.geometry()))
        {
            if (lineVertices.empty())
            {
                // Same view-derived extent as WorkspaceViewport::render, using
                // the render aspect. Derive it only when a Line is present.
                const auto visibleHeight = context.projection.mode() == ProjectionMode::Perspective
                    ? checked(2.0 * context.camera.distance() * std::tan(context.verticalFov / 2.0))
                    : context.projection.visibleHeight();
                const auto visibleScale = checked(visibleHeight * std::max(1.0, context.aspectRatio()));
                lineVertices = presentedLineVertices(presentation, {context.camera.target(), visibleScale});
            }
            const auto& a = lineVertices[lineIndex++];
            const auto& b = lineVertices[lineIndex++];
            candidate = segmentCandidate({a.x(), a.y(), a.z()}, {b.x(), b.y(), b.z()}, projection, context);
        }
        if (!candidate || candidate->distance > context.tolerancePixels)
            continue;
        const bool closer = !best || candidate->distance < best->screenDistance - distanceTiePixels;
        const bool depthWins = best
            && std::abs(candidate->distance - best->screenDistance) <= distanceTiePixels
            && candidate->depth < bestDepth && !math::almostEqual(candidate->depth, bestDepth);
        if (closer || depthWins)
        {
            best = PickHit{entity.id(), candidate->distance};
            bestDepth = candidate->depth;
        }
    }
    return best;
}
}
