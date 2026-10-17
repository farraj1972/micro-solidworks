#include "features/PlanarRegion.h"
#include "core/geometry/Intersection2.h"
#include "core/geometry/GeometricTolerance.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <numbers>

namespace microsw::features
{
namespace
{
using geometry::Point2;
struct Raw { sketch::SketchEntityId id; RegionCurve2 curve; Point2 a; Point2 b; };
struct Loop { RegionBoundary boundary; std::vector<Point2> polygon; double area{}; int parent{-1}; int depth{}; };
bool same(const Point2&a,const Point2&b){return geometry::areCoincident(a,b);}
Point2 start(const RegionBoundaryUse& u)
{
    return std::visit([&](const auto& c)->Point2{using T=std::decay_t<decltype(c)>;
        if constexpr(std::is_same_v<T,geometry::Segment2>)return u.reversed?c.b():c.a();
        else if constexpr(std::is_same_v<T,geometry::Arc2>)return u.reversed?c.endPoint():c.startPoint();
        else return c.pointAt(0);},u.curve);
}
Point2 end(const RegionBoundaryUse& u)
{
    return std::visit([&](const auto& c)->Point2{using T=std::decay_t<decltype(c)>;
        if constexpr(std::is_same_v<T,geometry::Segment2>)return u.reversed?c.a():c.b();
        else if constexpr(std::is_same_v<T,geometry::Arc2>)return u.reversed?c.startPoint():c.endPoint();
        else return c.pointAt(0);},u.curve);
}
std::vector<Point2> sample(const RegionBoundary& b)
{
    std::vector<Point2> out;
    for(const auto& u:b.uses)std::visit([&](const auto& c){using T=std::decay_t<decltype(c)>;
        if constexpr(std::is_same_v<T,geometry::Segment2>)out.push_back(start(u));
        else if constexpr(std::is_same_v<T,geometry::Arc2>){for(int i=0;i<16;++i){double q=double(i)/16;out.push_back(u.reversed?c.pointAt(1-q):c.pointAt(q));}}
        else {for(int i=0;i<64;++i){double angle=2*std::numbers::pi*double(u.reversed?-i:i)/64;out.push_back(c.pointAt(angle));}}
    },u.curve);
    return out;
}
double signedArea(const std::vector<Point2>& p){double a=0;for(size_t i=0;i<p.size();++i){const auto&q=p[(i+1)%p.size()];a+=p[i].x()*q.y()-q.x()*p[i].y();}return a/2;}
bool inside(const Point2&p,const std::vector<Point2>& poly){bool in=false;for(size_t i=0,j=poly.size()-1;i<poly.size();j=i++){const auto&a=poly[i];const auto&b=poly[j];if(((a.y()>p.y())!=(b.y()>p.y()))&&(p.x()<(b.x()-a.x())*(p.y()-a.y())/(b.y()-a.y())+a.x()))in=!in;}return in;}
void reverseBoundary(RegionBoundary& b){std::reverse(b.uses.begin(),b.uses.end());for(auto&u:b.uses)u.reversed=!u.reversed;}
geometry::Intersection2 cross(const RegionCurve2&a,const RegionCurve2&b)
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
PlanarRegionSet fail(RegionStatus s,std::string m,std::vector<sketch::SketchEntityId> ids={}){return{{},{s,std::move(m),std::move(ids)}};}
}

PlanarRegionSet extractPlanarRegions(const sketch::Sketch& sketch)
{
    if(sketch.size()==0)return fail(RegionStatus::Empty,"Sketch contains no closed contour");
    std::vector<Raw> open;std::vector<Loop> loops;
    for(auto id:sketch.entityIds())
    {
        const auto&e=sketch.find(id);
        if(e.type()==sketch::SketchEntityType::Circle){const auto&c=std::get<sketch::SketchCircle>(e.geometry()).geometry;RegionBoundary b{{{id,c,false}}};loops.push_back({b,sample(b)});}
        else if(e.type()==sketch::SketchEntityType::Line){const auto&s=std::get<sketch::SketchLine>(e.geometry()).geometry;if(s.isDegenerate())return fail(RegionStatus::Degenerate,"Degenerate line in profile",{id});open.push_back({id,s,s.a(),s.b()});}
        else {const auto&a=std::get<sketch::SketchArc>(e.geometry()).geometry;if(same(a.startPoint(),a.endPoint()))return fail(RegionStatus::Degenerate,"Degenerate arc in profile",{id});open.push_back({id,a,a.startPoint(),a.endPoint()});}
    }
    for (std::size_t i=0;i<open.size();++i)
    {
        int degree=0;
        for(const auto& e:open){if(same(open[i].a,e.a)||same(open[i].a,e.b))++degree;}
        if(degree>2)return fail(RegionStatus::Branch,"Branching contour detected",{open[i].id});
        if(degree<2)return fail(RegionStatus::OpenContour,"Open contour detected. Close the highlighted sketch geometry or use Thin Feature.",{open[i].id});
    }
    while(!open.empty())
    {
        RegionBoundary b;auto first=open.front();open.erase(open.begin());b.uses.push_back({first.id,first.curve,false});auto cursor=first.b;
        while(!same(cursor,start(b.uses.front())))
        {
            std::vector<std::pair<size_t,bool>> matches;
            for(size_t i=0;i<open.size();++i){if(same(cursor,open[i].a))matches.push_back({i,false});if(same(cursor,open[i].b))matches.push_back({i,true});}
            if(matches.empty())return fail(RegionStatus::OpenContour,"Open contour detected. Close the highlighted sketch geometry or use Thin Feature.",{b.uses.back().source});
            if(matches.size()!=1)return fail(RegionStatus::Branch,"Branching contour detected",{b.uses.back().source});
            const auto [i,rev]=matches.front();b.uses.push_back({open[i].id,open[i].curve,rev});cursor=rev?open[i].a:open[i].b;open.erase(open.begin()+static_cast<std::ptrdiff_t>(i));
        }
        loops.push_back({b,sample(b)});
    }
    std::vector<RegionBoundaryUse> all;for(const auto&l:loops)for(const auto&u:l.boundary.uses)all.push_back(u);
    for(size_t i=0;i<all.size();++i)for(size_t j=i+1;j<all.size();++j)
    {
        const auto r=cross(all[i].curve,all[j].curve);if(r.kind==geometry::IntersectionKind::None)continue;
        const bool allowedEndpoint=r.kind==geometry::IntersectionKind::Endpoint&&!r.points.empty()&&std::all_of(r.points.begin(),r.points.end(),[&](const auto& p){
            return (same(p,start(all[i]))||same(p,end(all[i])))&&(same(p,start(all[j]))||same(p,end(all[j])));});
        if(allowedEndpoint)continue;
        if(r.kind==geometry::IntersectionKind::Overlap||r.kind==geometry::IntersectionKind::Coincident)return fail(RegionStatus::Overlap,"Overlapping or duplicate boundaries",{all[i].source,all[j].source});
        return fail(r.kind==geometry::IntersectionKind::Proper?RegionStatus::SelfIntersection:RegionStatus::Ambiguous,"Intersecting or ambiguously touching boundaries",{all[i].source,all[j].source});
    }
    for(auto&l:loops){l.area=signedArea(l.polygon);if(std::abs(l.area)<=geometry::defaultGeometricTolerance)return fail(RegionStatus::Degenerate,"Zero-area contour");}
    for(size_t i=0;i<loops.size();++i){double best=std::numeric_limits<double>::infinity();for(size_t j=0;j<loops.size();++j)if(i!=j&&std::abs(loops[j].area)>std::abs(loops[i].area)&&inside(loops[i].polygon.front(),loops[j].polygon)){if(std::abs(loops[j].area)<best){best=std::abs(loops[j].area);loops[i].parent=int(j);}}}
    for(auto&l:loops){int p=l.parent;while(p>=0){++l.depth;p=loops[p].parent;}}
    for(auto&l:loops){const bool wantCcw=l.depth%2==0;if((l.area>0)!=wantCcw){reverseBoundary(l.boundary);l.polygon=sample(l.boundary);l.area=-l.area;}}
    PlanarRegionSet result;for(size_t i=0;i<loops.size();++i)if(loops[i].depth%2==0){PlanarRegion region{loops[i].boundary,{}};for(size_t j=0;j<loops.size();++j)if(loops[j].parent==int(i)&&loops[j].depth%2==1)region.holes.push_back(loops[j].boundary);result.regions.push_back(std::move(region));}
    if(result.regions.empty())return fail(RegionStatus::Empty,"No material region detected");return result;
}
}
