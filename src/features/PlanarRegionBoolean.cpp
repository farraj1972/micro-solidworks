#include "features/PlanarRegionBoolean.h"

#include "core/geometry/GeometricTolerance.h"
#include "core/geometry/Intersection2.h"

#include <algorithm>
#include <cmath>
#include <numbers>
#include <stdexcept>
#include <type_traits>
#include <limits>

namespace microsw::features::detail
{
namespace
{
constexpr auto twoPi = 2.0 * std::numbers::pi_v<math::Scalar>;
math::Scalar positiveModulo(math::Scalar value)
{
    auto result = std::fmod(value, twoPi);
    return result < 0 ? result + twoPi : result;
}
math::Scalar parameter(const geometry::Segment2& curve, const geometry::Point2& point)
{
    const auto dx=curve.b().x()-curve.a().x(),dy=curve.b().y()-curve.a().y();
    const auto denominator=dx*dx+dy*dy;
    if(denominator<=geometry::defaultGeometricTolerance*geometry::defaultGeometricTolerance)
        throw std::domain_error{"Cannot split a degenerate segment"};
    return ((point.x()-curve.a().x())*dx+(point.y()-curve.a().y())*dy)/denominator;
}
math::Scalar parameter(const geometry::Arc2& curve, const geometry::Point2& point)
{
    const auto angle=std::atan2(point.y()-curve.center().y(),point.x()-curve.center().x());
    const auto travelled=curve.sweepAngle()>0?positiveModulo(angle-curve.startAngle())
        :positiveModulo(curve.startAngle()-angle);
    return travelled/std::abs(curve.sweepAngle());
}
math::Scalar parameter(const geometry::Circle2& curve, const geometry::Point2& point)
{
    return positiveModulo(std::atan2(point.y()-curve.center().y(),point.x()-curve.center().x()))/twoPi;
}
template<class Curve> std::vector<math::Scalar> parameters(const Curve& curve,const std::vector<geometry::Point2>& points)
{
    std::vector<math::Scalar> result{0,1};
    for(const auto&point:points){const auto value=parameter(curve,point);if(value>geometry::defaultGeometricTolerance&&value<1-geometry::defaultGeometricTolerance)result.push_back(value);}
    std::sort(result.begin(),result.end());
    if constexpr(std::is_same_v<Curve,geometry::Circle2>)
        if(result.size()==2)result.insert(result.begin()+1,0.5);
    result.erase(std::unique(result.begin(),result.end(),[](auto a,auto b){return std::abs(a-b)<=geometry::defaultGeometricTolerance;}),result.end());
    return result;
}

struct Piece { sketch::SketchEntityId source; RegionCurve2 curve; };
geometry::Point2 first(const RegionCurve2&v){return std::visit([](const auto&c){using T=std::decay_t<decltype(c)>;if constexpr(std::is_same_v<T,geometry::Segment2>)return c.a();else if constexpr(std::is_same_v<T,geometry::Arc2>)return c.startPoint();else return c.pointAt(0);},v);}
geometry::Point2 last(const RegionCurve2&v){return std::visit([](const auto&c){using T=std::decay_t<decltype(c)>;if constexpr(std::is_same_v<T,geometry::Segment2>)return c.b();else if constexpr(std::is_same_v<T,geometry::Arc2>)return c.endPoint();else return c.pointAt(0);},v);}
geometry::Point2 middle(const RegionCurve2&v){return std::visit([](const auto&c){using T=std::decay_t<decltype(c)>;if constexpr(std::is_same_v<T,geometry::Circle2>)return c.pointAt(0);else return c.pointAt(0.5);},v);}
RegionCurve2 reversed(const RegionCurve2&v){return std::visit([](const auto&c)->RegionCurve2{using T=std::decay_t<decltype(c)>;if constexpr(std::is_same_v<T,geometry::Segment2>)return geometry::Segment2{c.b(),c.a()};else if constexpr(std::is_same_v<T,geometry::Arc2>)return geometry::Arc2{c.center(),c.radius(),c.startAngle()+c.sweepAngle(),-c.sweepAngle()};else return c;},v);}
bool same(const geometry::Point2&a,const geometry::Point2&b){return geometry::areCoincident(a,b);}

geometry::Intersection2 intersection(const RegionCurve2&a,const RegionCurve2&b)
{
 return std::visit([](const auto&x,const auto&y)->geometry::Intersection2{using A=std::decay_t<decltype(x)>;using B=std::decay_t<decltype(y)>;
  if constexpr(std::is_same_v<A,geometry::Segment2>&&std::is_same_v<B,geometry::Segment2>)return geometry::intersect(x,y);
  else if constexpr(std::is_same_v<A,geometry::Segment2>&&std::is_same_v<B,geometry::Circle2>)return geometry::intersect(x,y);
  else if constexpr(std::is_same_v<A,geometry::Circle2>&&std::is_same_v<B,geometry::Segment2>)return geometry::intersect(y,x);
  else if constexpr(std::is_same_v<A,geometry::Segment2>&&std::is_same_v<B,geometry::Arc2>)return geometry::intersect(x,y);
  else if constexpr(std::is_same_v<A,geometry::Arc2>&&std::is_same_v<B,geometry::Segment2>)return geometry::intersect(y,x);
  else if constexpr(std::is_same_v<A,geometry::Circle2>&&std::is_same_v<B,geometry::Circle2>)return geometry::intersect(x,y);
  else if constexpr(std::is_same_v<A,geometry::Circle2>&&std::is_same_v<B,geometry::Arc2>)return geometry::intersect(x,y);
  else if constexpr(std::is_same_v<A,geometry::Arc2>&&std::is_same_v<B,geometry::Circle2>)return geometry::intersect(y,x);
  else return geometry::intersect(x,y);},a,b);
}

std::vector<Piece> flatten(const PlanarRegionSet&s)
{
 std::vector<Piece> out;auto add=[&](const RegionBoundary&b){for(const auto&u:b.uses){auto c=u.reversed?reversed(u.curve):u.curve;if(std::holds_alternative<geometry::Circle2>(c)){for(auto&p:splitRegionCurve(c,{}))out.push_back({u.source,p});}else out.push_back({u.source,c});}};
 for(const auto&r:s.regions){add(r.outer);for(const auto&h:r.holes)add(h);}return out;
}

std::vector<geometry::Point2> sample(const RegionBoundary&b)
{
 std::vector<geometry::Point2> out;for(const auto&u:b.uses){auto c=u.reversed?reversed(u.curve):u.curve;std::visit([&](const auto&v){using T=std::decay_t<decltype(v)>;if constexpr(std::is_same_v<T,geometry::Segment2>)out.push_back(v.a());else if constexpr(std::is_same_v<T,geometry::Arc2>)for(int i=0;i<32;++i)out.push_back(v.pointAt(double(i)/32));else for(int i=0;i<64;++i)out.push_back(v.pointAt(twoPi*double(i)/64));},c);}return out;
}
bool insidePolygon(const geometry::Point2&p,const std::vector<geometry::Point2>&poly){bool in=false;for(size_t i=0,j=poly.size()-1;i<poly.size();j=i++){const auto&a=poly[i],&b=poly[j];if(((a.y()>p.y())!=(b.y()>p.y()))&&p.x()<(b.x()-a.x())*(p.y()-a.y())/(b.y()-a.y())+a.x())in=!in;}return in;}
bool materialAt(const geometry::Point2&p,const PlanarRegionSet&s){bool material=false;for(const auto&r:s.regions){if(insidePolygon(p,sample(r.outer))){bool hole=false;for(const auto&h:r.holes)if(insidePolygon(p,sample(h)))hole=!hole;if(!hole)material=true;}}return material;}
double area(const RegionBoundary&b){const auto p=sample(b);double a=0;for(size_t i=0;i<p.size();++i){const auto&q=p[(i+1)%p.size()];a+=p[i].x()*q.y()-q.x()*p[i].y();}return a/2;}
void reverseBoundary(RegionBoundary&b){std::reverse(b.uses.begin(),b.uses.end());for(auto&u:b.uses)u.reversed=!u.reversed;}

RegionBooleanResult assemble(std::vector<Piece> kept)
{
 if(kept.empty())return{RegionBooleanStatus::Empty,{}, {}};
 std::vector<RegionBoundary> loops;while(!kept.empty()){RegionBoundary loop;auto current=kept.back();kept.pop_back();const auto start=first(current.curve);auto cursor=last(current.curve);loop.uses.push_back({current.source,current.curve,false});while(!same(cursor,start)){auto it=std::find_if(kept.begin(),kept.end(),[&](const auto&p){return same(first(p.curve),cursor);});if(it==kept.end())return{RegionBooleanStatus::Ambiguous,{},"Boolean boundary did not reconstruct a closed loop"};cursor=last(it->curve);loop.uses.push_back({it->source,it->curve,false});kept.erase(it);}loops.push_back(std::move(loop));}
 struct L{RegionBoundary boundary;double signedArea;int parent{-1};int depth{};};std::vector<L> classified;for(auto&l:loops)classified.push_back({std::move(l),0});for(auto&l:classified)l.signedArea=area(l.boundary);
 for(size_t i=0;i<classified.size();++i){double best=std::numeric_limits<double>::infinity();const auto point=first(classified[i].boundary.uses.front().curve);for(size_t j=0;j<classified.size();++j)if(i!=j&&std::abs(classified[j].signedArea)>std::abs(classified[i].signedArea)&&insidePolygon(point,sample(classified[j].boundary))&&std::abs(classified[j].signedArea)<best){best=std::abs(classified[j].signedArea);classified[i].parent=int(j);}}
 for(auto&l:classified){for(int p=l.parent;p>=0;p=classified[p].parent)++l.depth;const bool ccw=l.depth%2==0;if((l.signedArea>0)!=ccw)reverseBoundary(l.boundary);}
 PlanarRegionSet out;for(size_t i=0;i<classified.size();++i)if(classified[i].depth%2==0){PlanarRegion r{classified[i].boundary,{}};for(size_t j=0;j<classified.size();++j)if(classified[j].parent==int(i)&&classified[j].depth%2==1)r.holes.push_back(classified[j].boundary);out.regions.push_back(std::move(r));}return{RegionBooleanStatus::Success,std::move(out),{}};
}
}

std::vector<RegionCurve2> splitRegionCurve(const RegionCurve2& value,
                                           const std::vector<geometry::Point2>& points)
{
    return std::visit([&](const auto& curve){using T=std::decay_t<decltype(curve)>;
        std::vector<RegionCurve2> result;const auto values=parameters(curve,points);
        for(std::size_t i=1;i<values.size();++i){const auto a=values[i-1],b=values[i];
            if constexpr(std::is_same_v<T,geometry::Segment2>)result.emplace_back(geometry::Segment2{curve.pointAt(a),curve.pointAt(b)});
            else if constexpr(std::is_same_v<T,geometry::Arc2>)result.emplace_back(geometry::Arc2{curve.center(),curve.radius(),curve.startAngle()+a*curve.sweepAngle(),(b-a)*curve.sweepAngle()});
            else result.emplace_back(geometry::Arc2{curve.center(),curve.radius(),a*twoPi,(b-a)*twoPi});}
        return result;},value);
}

}

namespace microsw::features
{
RegionBooleanResult subtractPlanarRegions(const PlanarRegionSet&target,const PlanarRegionSet&cutter)
{
 if(!target.valid()||!cutter.valid())return{RegionBooleanStatus::Ambiguous,{},"Invalid planar region input"};
 try{
  auto a=detail::flatten(target),b=detail::flatten(cutter);std::vector<std::vector<geometry::Point2>>ap(a.size()),bp(b.size());
  for(size_t i=0;i<a.size();++i)for(size_t j=0;j<b.size();++j){const auto x=detail::intersection(a[i].curve,b[j].curve);if(x.kind==geometry::IntersectionKind::Overlap||x.kind==geometry::IntersectionKind::Coincident||x.kind==geometry::IntersectionKind::Ambiguous)return{RegionBooleanStatus::Ambiguous,{},"Ambiguous coincident planar boundaries"};for(const auto&p:x.points){ap[i].push_back(p);bp[j].push_back(p);}}
  std::vector<detail::Piece> kept;for(size_t i=0;i<a.size();++i)for(auto&c:detail::splitRegionCurve(a[i].curve,ap[i]))if(!detail::materialAt(detail::middle(c),cutter))kept.push_back({a[i].source,c});
  for(size_t i=0;i<b.size();++i)for(auto&c:detail::splitRegionCurve(b[i].curve,bp[i]))if(detail::materialAt(detail::middle(c),target))kept.push_back({b[i].source,detail::reversed(c)});
  return detail::assemble(std::move(kept));
 }catch(const std::exception&e){return{RegionBooleanStatus::NumericalFailure,{},e.what()};}
}

RegionBooleanResult intersectPlanarRegions(const PlanarRegionSet&first,const PlanarRegionSet&second)
{
 if(!first.valid()||!second.valid())return{RegionBooleanStatus::Ambiguous,{},"Invalid planar region input"};
 try{
  auto a=detail::flatten(first),b=detail::flatten(second);std::vector<std::vector<geometry::Point2>>ap(a.size()),bp(b.size());
  for(size_t i=0;i<a.size();++i)for(size_t j=0;j<b.size();++j){const auto x=detail::intersection(a[i].curve,b[j].curve);if(x.kind==geometry::IntersectionKind::Overlap||x.kind==geometry::IntersectionKind::Coincident||x.kind==geometry::IntersectionKind::Ambiguous)return{RegionBooleanStatus::Ambiguous,{},"Ambiguous coincident planar boundaries"};for(const auto&p:x.points){ap[i].push_back(p);bp[j].push_back(p);}}
  std::vector<detail::Piece> kept;for(size_t i=0;i<a.size();++i)for(auto&c:detail::splitRegionCurve(a[i].curve,ap[i]))if(detail::materialAt(detail::middle(c),second))kept.push_back({a[i].source,c});
  for(size_t i=0;i<b.size();++i)for(auto&c:detail::splitRegionCurve(b[i].curve,bp[i]))if(detail::materialAt(detail::middle(c),first))kept.push_back({b[i].source,c});
  return detail::assemble(std::move(kept));
 }catch(const std::exception&e){return{RegionBooleanStatus::NumericalFailure,{},e.what()};}
}

RegionBooleanResult partitionPlanarRegions(const PlanarRegionSet&regions,const PlanarRegionSet&splitter)
{
 if(!regions.valid()||!splitter.valid())return{RegionBooleanStatus::Ambiguous,{},"Invalid planar region input"};
 try{
  const auto cutters=detail::flatten(splitter);PlanarRegionSet output=regions;
  auto partition=[&](RegionBoundary&boundary){RegionBoundary result;for(const auto&use:boundary.uses){auto curve=use.reversed?detail::reversed(use.curve):use.curve;std::vector<geometry::Point2>points;for(const auto&other:cutters){const auto hit=detail::intersection(curve,other.curve);if(hit.kind==geometry::IntersectionKind::Overlap||hit.kind==geometry::IntersectionKind::Coincident||hit.kind==geometry::IntersectionKind::Ambiguous)throw std::invalid_argument{"Ambiguous coincident planar boundaries"};points.insert(points.end(),hit.points.begin(),hit.points.end());}for(auto&piece:detail::splitRegionCurve(curve,points))result.uses.push_back({use.source,std::move(piece),false});}boundary=std::move(result);};
  for(auto&region:output.regions){partition(region.outer);for(auto&hole:region.holes)partition(hole);}return{RegionBooleanStatus::Success,std::move(output),{}};
 }catch(const std::exception&e){return{RegionBooleanStatus::Ambiguous,{},e.what()};}
}
}
