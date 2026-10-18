#include "features/AxialSubtraction.h"

#include "core/geometry/GeometricTolerance.h"
#include "features/AxialCut.h"
#include "features/GeneralExtrusion.h"
#include "features/PlanarRegionBoolean.h"

#include <algorithm>
#include <cmath>
#include <numbers>
#include <stdexcept>
#include <type_traits>

namespace microsw::features
{
namespace
{
constexpr auto tolerance = geometry::defaultGeometricTolerance;

RegionCurve2 reframeCurve(const RegionCurve2& value,
                          const sketch::SketchPlane& source,
                          const sketch::SketchPlane& target)
{
    const auto orientation = math::dot(source.normal(), target.normal()) >= 0 ? 1.0 : -1.0;
    return std::visit([&](const auto& curve) -> RegionCurve2 {
        using T = std::decay_t<decltype(curve)>;
        if constexpr (std::is_same_v<T, geometry::Segment2>)
            return geometry::Segment2{target.toLocal(source.toWorld(curve.a())),
                                      target.toLocal(source.toWorld(curve.b()))};
        else
        {
            const auto center = target.toLocal(source.toWorld(curve.center()));
            const auto start = target.toLocal(source.toWorld(curve.pointAt(0)));
            const auto vector = start - center;
            const auto angle = std::atan2(vector.y(), vector.x());
            if constexpr (std::is_same_v<T, geometry::Arc2>)
                return geometry::Arc2{center, curve.radius(), angle,
                                      curve.sweepAngle() * orientation};
            else
                return geometry::Circle2{center, curve.radius()};
        }
    }, value);
}

PlanarRegionSet reframe(const PlanarRegionSet& source,
                        const sketch::SketchPlane& sourcePlane,
                        const sketch::SketchPlane& targetPlane)
{
    auto result = source;
    const auto convert = [&](RegionBoundary& boundary) {
        for (auto& use : boundary.uses)
        {
            use.curve = reframeCurve(use.curve, sourcePlane, targetPlane);
            if (math::dot(sourcePlane.normal(), targetPlane.normal()) < 0)
                use.reversed = !use.reversed;
        }
    };
    for (auto& region : result.regions)
    {
        convert(region.outer);
        for (auto& hole : region.holes) convert(hole);
    }
    return result;
}

struct Piece
{
    geometry::Point2 start;
    geometry::Point2 end;
    std::variant<geometry::Segment2, geometry::Arc2> curve;
};

std::vector<Piece> pieces(const RegionBoundary& boundary)
{
    std::vector<Piece> result;
    for (const auto& use : boundary.uses)
        std::visit([&](const auto& curve) {
            using T = std::decay_t<decltype(curve)>;
            if constexpr (std::is_same_v<T, geometry::Segment2>)
            {
                if (use.reversed) result.push_back({curve.b(),curve.a(),geometry::Segment2{curve.b(),curve.a()}});
                else result.push_back({curve.a(),curve.b(),curve});
            }
            else if constexpr (std::is_same_v<T, geometry::Arc2>)
            {
                const auto angle=use.reversed?curve.startAngle()+curve.sweepAngle():curve.startAngle();
                const auto sweep=use.reversed?-curve.sweepAngle():curve.sweepAngle();
                const geometry::Arc2 directed{curve.center(),curve.radius(),angle,sweep};
                result.push_back({directed.startPoint(),directed.endPoint(),directed});
            }
            else
            {
                const auto sweep=use.reversed?-std::numbers::pi:std::numbers::pi;
                const geometry::Arc2 first{curve.center(),curve.radius(),0,sweep};
                const geometry::Arc2 second{curve.center(),curve.radius(),sweep,sweep};
                result.push_back({first.startPoint(),first.endPoint(),first});
                result.push_back({second.startPoint(),second.endPoint(),second});
            }
        },use.curve);
    return result;
}

topology::EdgeOrientation flipped(topology::EdgeOrientation value)
{
    return value==topology::EdgeOrientation::Forward
        ? topology::EdgeOrientation::Reversed : topology::EdgeOrientation::Forward;
}

class BoundaryBuilder
{
public:
    BoundaryBuilder(const sketch::SketchPlane& plane) : plane_{plane} {}

    topology::Solid finish(std::vector<topology::OrientedFaceUse> faces)
    {
        const auto shell=solid_.addShell(std::move(faces));
        solid_.setGeneralRootShell(shell);
        if(!solid_.isValid())throw std::runtime_error{"Axial subtraction produced an invalid Solid"};
        return std::move(solid_);
    }

    void addCaps(const PlanarRegionSet& regions,math::Scalar offset,
                 topology::FaceOrientation orientation,
                 std::vector<topology::OrientedFaceUse>& faces)
    {
        for(const auto&region:regions.regions)
        {
            const auto outer=wire(region.outer,offset);std::vector<topology::WireId>holes;
            for(const auto&hole:region.holes)holes.push_back(wire(hole,offset));
            const geometry::Plane support{plane_.origin()+plane_.normal()*offset,plane_.normal()};
            faces.push_back({solid_.addFace(outer,std::move(holes),support),orientation});
        }
    }

    void addSides(const PlanarRegionSet& regions,math::Scalar low,math::Scalar high,
                  std::vector<topology::OrientedFaceUse>& faces)
    {
        const auto add=[&](const RegionBoundary&boundary){for(const auto&piece:pieces(boundary))
        {
            const auto bottom=edge(piece,low),top=edge(piece,high);
            const auto end=vertical(piece.end,low,high),start=vertical(piece.start,low,high);
            const auto sideWire=solid_.addWire({bottom,end,
                {top.edge,flipped(top.orientation)},{start.edge,flipped(start.orientation)}});
            const auto face=std::visit([&](const auto&curve)->topology::FaceId{
                using T=std::decay_t<decltype(curve)>;
                if constexpr(std::is_same_v<T,geometry::Segment2>)
                {
                    const auto tangent=plane_.toWorld(curve.b())-plane_.toWorld(curve.a());
                    return solid_.addFace(sideWire,{plane_.toWorld(curve.a()),math::cross(tangent,plane_.normal())});
                }
                else return solid_.addCylindricalFace(sideWire,
                    {plane_.toWorld(curve.center()),plane_.normal(),curve.radius()});
            },piece.curve);
            faces.push_back({face,topology::FaceOrientation::Forward});
        }};
        for(const auto&region:regions.regions){add(region.outer);for(const auto&hole:region.holes)add(hole);}
    }

private:
    topology::VertexId vertex(const geometry::Point3& point)
    {
        for(std::size_t i=0;i<solid_.vertices().size();++i)
            if(geometry::areCoincident(solid_.vertices()[i].point(),point))
                return topology::VertexId{static_cast<topology::VertexId::Value>(i)};
        return solid_.addVertex(point);
    }

    topology::OrientedEdgeUse edge(const Piece& piece,math::Scalar offset)
    {
        const auto a=plane_.toWorld(piece.start)+plane_.normal()*offset;
        const auto b=plane_.toWorld(piece.end)+plane_.normal()*offset;
        const auto va=vertex(a),vb=vertex(b);
        geometry::Point3 midpoint{};
        bool circular=false;
        std::visit([&](const auto&curve){using T=std::decay_t<decltype(curve)>;
            if constexpr(std::is_same_v<T,geometry::Arc2>){midpoint=plane_.toWorld(curve.pointAt(0.5))+plane_.normal()*offset;circular=true;}
            else midpoint={};},piece.curve);
        for(std::size_t i=0;i<solid_.edges().size();++i)
        {
            const auto&e=solid_.edges()[i];const bool forward=e.start()==va&&e.end()==vb;
            const bool reverse=e.start()==vb&&e.end()==va;if(!forward&&!reverse)continue;
            if(circular!=std::holds_alternative<geometry::CircularArc3>(e.geometry()))continue;
            if(circular&&!geometry::areCoincident(std::get<geometry::CircularArc3>(e.geometry()).pointAt(0.5),midpoint))continue;
            return{topology::EdgeId{static_cast<topology::EdgeId::Value>(i)},forward?topology::EdgeOrientation::Forward:topology::EdgeOrientation::Reversed};
        }
        topology::EdgeId id;
        if(circular)
        {
            const auto&arc=std::get<geometry::Arc2>(piece.curve);
            id=solid_.addCircularEdge(va,vb,{plane_.toWorld(arc.center())+plane_.normal()*offset,
                plane_.normal(),plane_.xAxis(),arc.radius(),arc.startAngle(),arc.sweepAngle()});
        }
        else id=solid_.addEdge(va,vb);
        return{id,topology::EdgeOrientation::Forward};
    }

    topology::OrientedEdgeUse vertical(const geometry::Point2&point,math::Scalar low,math::Scalar high)
    {
        const auto a=plane_.toWorld(point)+plane_.normal()*low;
        const auto b=plane_.toWorld(point)+plane_.normal()*high;
        const auto va=vertex(a),vb=vertex(b);
        for(std::size_t i=0;i<solid_.edges().size();++i)
        {
            const auto&e=solid_.edges()[i];
            if(!std::holds_alternative<geometry::Segment3>(e.geometry()))continue;
            if(e.start()==va&&e.end()==vb)return{topology::EdgeId{static_cast<topology::EdgeId::Value>(i)},topology::EdgeOrientation::Forward};
            if(e.start()==vb&&e.end()==va)return{topology::EdgeId{static_cast<topology::EdgeId::Value>(i)},topology::EdgeOrientation::Reversed};
        }
        return{solid_.addEdge(va,vb),topology::EdgeOrientation::Forward};
    }

    topology::WireId wire(const RegionBoundary& boundary,math::Scalar offset)
    {
        std::vector<topology::OrientedEdgeUse>uses;
        for(const auto&piece:pieces(boundary))uses.push_back(edge(piece,offset));
        return solid_.addWire(std::move(uses));
    }

    const sketch::SketchPlane& plane_;
    topology::Solid solid_;
};

topology::Solid buildBlind(const PlanarRegionSet& lower,const PlanarRegionSet& upper,
                           const PlanarRegionSet& interfaceRegion,
                           const sketch::SketchPlane& plane,math::Scalar split,
                           math::Scalar height)
{
    BoundaryBuilder builder{plane};std::vector<topology::OrientedFaceUse>faces;
    builder.addCaps(lower,0,topology::FaceOrientation::Reversed,faces);
    builder.addSides(lower,0,split,faces);
    builder.addCaps(interfaceRegion,split,topology::FaceOrientation::Forward,faces);
    builder.addSides(upper,split,height,faces);
    builder.addCaps(upper,height,topology::FaceOrientation::Forward,faces);
    return builder.finish(std::move(faces));
}
}

AxialSubtractionResult subtractAxialRegions(const PlanarRegionSet& targetRegions,
 const sketch::SketchPlane& targetPlane,math::Scalar targetDistance,
 const PlanarRegionSet& cutterRegions,const sketch::SketchPlane& cutterPlane,
 const math::Vector3& cutterDirection,math::Scalar cutterDistance)
{
 if(!targetRegions.valid()||!cutterRegions.valid()||!std::isfinite(targetDistance)
    ||!std::isfinite(cutterDistance)||targetDistance<=tolerance||cutterDistance<=tolerance)
    return{AxialSubtractionStatus::RegionFailure,{},"Invalid axial subtraction input"};
 if(classifyAxialCut(cutterDirection,targetPlane.normal())==AxialCutAlignment::Unsupported)
    return{AxialSubtractionStatus::UnsupportedTarget,{},"Cut direction is not axial to the target"};
 try{
  const auto cutter=reframe(cutterRegions,cutterPlane,targetPlane);
  const auto start=math::dot(cutterPlane.origin()-targetPlane.origin(),targetPlane.normal());
  const auto sign=math::dot(cutterDirection,targetPlane.normal())>=0?1.0:-1.0;
  const auto finish=start+sign*cutterDistance;
  const auto low=std::max(0.0,std::min(start,finish));const auto high=std::min(targetDistance,std::max(start,finish));
  if(high-low<=tolerance)return{AxialSubtractionStatus::NoIntersection,{},"Cut volume does not overlap the target"};
  const auto overlap=intersectPlanarRegions(targetRegions,cutter);
  if(overlap.status==RegionBooleanStatus::Empty)return{AxialSubtractionStatus::NoIntersection,{},"Cut profile does not intersect target material"};
  if(!overlap.succeeded())return{AxialSubtractionStatus::RegionFailure,{},overlap.diagnostic};
  const auto difference=subtractPlanarRegions(targetRegions,cutter);
  if(!difference.succeeded())return{AxialSubtractionStatus::RegionFailure,{},difference.diagnostic};
  if(low<=tolerance&&high>=targetDistance-tolerance)
  {
   if(difference.status==RegionBooleanStatus::Empty)return{AxialSubtractionStatus::SuccessEmpty,{},{}};
   auto result=extrudeRegions(difference.regions,targetPlane,targetDistance);
   if(!result.succeeded())return{AxialSubtractionStatus::TopologyFailure,{},result.diagnostic};
   return{AxialSubtractionStatus::Success,std::move(result.solids),{}};
  }
  if(low>tolerance&&high<targetDistance-tolerance)
    return{AxialSubtractionStatus::UnsupportedTarget,{},"Internal axial cuts are outside the blind Cut Extrude contract"};
  const auto partitioned=partitionPlanarRegions(targetRegions,cutter);
  if(!partitioned.succeeded())return{AxialSubtractionStatus::RegionFailure,{},partitioned.diagnostic};
  std::vector<topology::Solid>solids;
  for(const auto&target:partitioned.regions.regions)
  {
   PlanarRegionSet single{{target},{}};const auto cut=subtractPlanarRegions(single,cutter);
   const auto removed=intersectPlanarRegions(single,cutter);
   if(removed.status==RegionBooleanStatus::Empty){auto unchanged=extrudeRegions(single,targetPlane,targetDistance);if(!unchanged.succeeded())return{AxialSubtractionStatus::TopologyFailure,{},unchanged.diagnostic};solids.push_back(std::move(unchanged.solids.front()));continue;}
   if(!cut.succeeded()||!removed.succeeded())return{AxialSubtractionStatus::RegionFailure,{},"Planar layer reconstruction failed"};
   if(cut.status==RegionBooleanStatus::Empty)return{AxialSubtractionStatus::UnsupportedTarget,{},"Partial-depth removal of an entire target cross-section is not supported"};
   if(high>=targetDistance-tolerance)solids.push_back(buildBlind(single,cut.regions,removed.regions,targetPlane,low,targetDistance));
   else
   {
    BoundaryBuilder builder{targetPlane};std::vector<topology::OrientedFaceUse>faces;
    builder.addCaps(cut.regions,0,topology::FaceOrientation::Reversed,faces);
    builder.addSides(cut.regions,0,high,faces);
    builder.addCaps(removed.regions,high,topology::FaceOrientation::Reversed,faces);
    builder.addSides(single,high,targetDistance,faces);
    builder.addCaps(single,targetDistance,topology::FaceOrientation::Forward,faces);
    solids.push_back(builder.finish(std::move(faces)));
   }
  }
  return solids.empty()?AxialSubtractionResult{AxialSubtractionStatus::SuccessEmpty,{},{}}
                       :AxialSubtractionResult{AxialSubtractionStatus::Success,std::move(solids),{}};
 }catch(const std::exception&e){return{AxialSubtractionStatus::TopologyFailure,{},e.what()};}
}
}
