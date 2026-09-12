#pragma once

#include <cstdint>
#include <limits>

namespace microsw::topology
{

template<class Tag>
class TopologyId
{
public:
    using Value = std::uint32_t;
    static constexpr Value invalidValue = std::numeric_limits<Value>::max();
    constexpr TopologyId() noexcept = default;
    explicit constexpr TopologyId(Value value) noexcept : value_{value} {}
    [[nodiscard]] constexpr Value value() const noexcept { return value_; }
    [[nodiscard]] constexpr bool isValid() const noexcept { return value_ != invalidValue; }
    friend constexpr bool operator==(TopologyId, TopologyId) noexcept = default;
private:
    Value value_{invalidValue};
};

struct VertexIdTag;
struct EdgeIdTag;
struct WireIdTag;
struct FaceIdTag;
struct ShellIdTag;
using VertexId = TopologyId<VertexIdTag>;
using EdgeId = TopologyId<EdgeIdTag>;
using WireId = TopologyId<WireIdTag>;
using FaceId = TopologyId<FaceIdTag>;
using ShellId = TopologyId<ShellIdTag>;

}
