#pragma once
#include "features/FeatureId.h"
#include "core/topology/TopologyIds.h"
#include <cstddef>
namespace microsw::interaction {
struct BodyInstanceContext { std::size_t index{}; friend bool operator==(BodyInstanceContext,BodyInstanceContext)=default; };
struct TopologyFaceTarget { features::FeatureId feature; BodyInstanceContext body; topology::FaceId face; friend bool operator==(const TopologyFaceTarget&,const TopologyFaceTarget&)=default; };
struct TopologyEdgeTarget { features::FeatureId feature; BodyInstanceContext body; topology::EdgeId edge; friend bool operator==(const TopologyEdgeTarget&,const TopologyEdgeTarget&)=default; };
}
