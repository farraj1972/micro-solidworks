#pragma once
#include "core/math/Vector3.h"
#include <memory>
#include <span>
namespace microsw::rendering {
struct LitVertex { math::Vector3 position; math::Vector3 normal; };
class TriangleRenderer {
public:
 TriangleRenderer(); ~TriangleRenderer();
 TriangleRenderer(const TriangleRenderer&)=delete; TriangleRenderer& operator=(const TriangleRenderer&)=delete;
 void setVertices(std::span<const LitVertex> vertices); void draw() const;
 [[nodiscard]] std::size_t vertexCount()const noexcept;
private: class Impl; std::unique_ptr<Impl> impl_;
}; }
