#include "presentation/SolidDisplay.h"

#include <algorithm>
#include <cmath>
#include <numbers>
#include <limits>
#include <stdexcept>
#include <type_traits>

namespace microsw::presentation
{
namespace
{
constexpr int curveSegments = 24;

bool samePlane(const geometry::Plane& a, const geometry::Plane& b)
{
    return geometry::isParallel(a, b) && a.contains(b.origin());
}

bool isCoplanarSeam(const topology::Solid& solid, topology::EdgeId edge)
{
    std::vector<const topology::Face*> incident;
    const auto containsEdge = [&](topology::WireId wire) {
        const auto& uses = solid.find(wire).uses();
        return std::any_of(uses.begin(), uses.end(),
            [&](const topology::OrientedEdgeUse& use) { return use.edge == edge; });
    };
    for (const auto& face : solid.faces())
    {
        bool contains = containsEdge(face.outerWire());
        for (const auto wire : face.innerWires()) contains = contains || containsEdge(wire);
        if (contains) incident.push_back(&face);
    }
    if (incident.size() != 2) return false;
    const auto* first = std::get_if<geometry::Plane>(&incident[0]->support());
    const auto* second = std::get_if<geometry::Plane>(&incident[1]->support());
    return first && second && samePlane(*first, *second);
}

std::vector<geometry::Point3> edgePoints(const topology::Edge& edge, bool reversed)
{
    std::vector<geometry::Point3> result;
    std::visit([&](const auto& curve)
    {
        using T = std::decay_t<decltype(curve)>;
        if constexpr (std::is_same_v<T, geometry::Segment3>)
            result = {curve.a(), curve.b()};
        else
            for (int i = 0; i <= curveSegments; ++i)
                result.push_back(curve.pointAt(static_cast<double>(i) / curveSegments));
    }, edge.geometry());
    if (reversed) std::reverse(result.begin(), result.end());
    return result;
}

std::vector<geometry::Point3> wirePoints(const topology::Solid& solid, topology::WireId id)
{
    std::vector<geometry::Point3> result;
    for (const auto& use : solid.find(id).uses())
    {
        auto points = edgePoints(solid.find(use.edge), use.orientation == topology::EdgeOrientation::Reversed);
        if (!result.empty()) points.erase(points.begin());
        result.insert(result.end(), points.begin(), points.end());
    }
    if (result.size() > 1 && geometry::areCoincident(result.front(), result.back())) result.pop_back();
    return result;
}

math::Vector3 triangleNormal(const geometry::Point3& a, const geometry::Point3& b,
    const geometry::Point3& c, const math::Vector3& preferred)
{
    auto n = math::cross(b - a, c - a).normalized();
    return math::dot(n, preferred) < 0 ? -n : n;
}

struct ProjectedPoint { geometry::Point3 world; double x{},y{}; };
double signedArea(const std::vector<ProjectedPoint>& p){double a=0;for(std::size_t i=0,j=p.size()-1;i<p.size();j=i++)a+=p[j].x*p[i].y-p[i].x*p[j].y;return a/2;}
double cross2(const ProjectedPoint&a,const ProjectedPoint&b,const ProjectedPoint&c){return(b.x-a.x)*(c.y-a.y)-(b.y-a.y)*(c.x-a.x);}
bool inTriangle(const ProjectedPoint&p,const ProjectedPoint&a,const ProjectedPoint&b,const ProjectedPoint&c){const auto x=cross2(a,b,p),y=cross2(b,c,p),z=cross2(c,a,p);return x>=-1e-10&&y>=-1e-10&&z>=-1e-10;}
bool same2(const ProjectedPoint&a,const ProjectedPoint&b){return std::abs(a.x-b.x)<=1e-10&&std::abs(a.y-b.y)<=1e-10;}
std::vector<std::array<geometry::Point3,3>> triangulate(std::vector<ProjectedPoint> polygon)
{
    std::vector<std::array<geometry::Point3,3>> result;if(polygon.size()<3)return result;
    if(signedArea(polygon)<0)std::reverse(polygon.begin(),polygon.end());
    std::vector<std::size_t> indices(polygon.size());for(std::size_t i=0;i<indices.size();++i)indices[i]=i;
    for(std::size_t guard=0;indices.size()>2&&guard<polygon.size()*polygon.size();++guard){bool clipped=false;
        for(std::size_t k=0;k<indices.size();++k){const auto ia=indices[(k+indices.size()-1)%indices.size()],ib=indices[k],ic=indices[(k+1)%indices.size()];const auto&a=polygon[ia];const auto&b=polygon[ib];const auto&c=polygon[ic];if(cross2(a,b,c)<=1e-10)continue;bool contains=false;for(auto index:indices)if(index!=ia&&index!=ib&&index!=ic&&!same2(polygon[index],a)&&!same2(polygon[index],b)&&!same2(polygon[index],c)&&inTriangle(polygon[index],a,b,c)){contains=true;break;}if(contains)continue;result.push_back({a.world,b.world,c.world});indices.erase(indices.begin()+static_cast<std::ptrdiff_t>(k));clipped=true;break;}if(!clipped)break;}
    return result;
}

void appendPlanar(const topology::Solid& solid, const topology::Face& face,
    interaction::TopologyFaceTarget target, const math::Vector3& outward, std::vector<FaceTriangle>& out)
{
    const auto outer = wirePoints(solid, face.outerWire());
    if (outer.size() < 3) return;
    const auto basisNormal = outward;
    const auto& plane = std::get<geometry::Plane>(face.support());
    const auto axisX = std::abs(plane.normal().x()) < 0.9
        ? math::cross(plane.normal(), {1,0,0}).normalized()
        : math::cross(plane.normal(), {0,1,0}).normalized();
    const auto axisY = math::cross(plane.normal(), axisX);
    const auto project=[&](const geometry::Point3&p){return ProjectedPoint{p,math::dot(p-plane.origin(),axisX),math::dot(p-plane.origin(),axisY)};};
    std::vector<ProjectedPoint> polygon;for(const auto&p:outer)polygon.push_back(project(p));if(signedArea(polygon)<0)std::reverse(polygon.begin(),polygon.end());
    for(const auto holeId:face.innerWires()){
        std::vector<ProjectedPoint> hole;for(const auto&p:wirePoints(solid,holeId))hole.push_back(project(p));if(signedArea(hole)>0)std::reverse(hole.begin(),hole.end());
        const auto h=static_cast<std::size_t>(std::distance(hole.begin(),std::max_element(hole.begin(),hole.end(),[](const auto&a,const auto&b){return a.x<b.x;})));
        std::size_t o=0;double best=std::numeric_limits<double>::infinity();for(std::size_t i=0;i<polygon.size();++i){const auto dx=polygon[i].x-hole[h].x,dy=polygon[i].y-hole[h].y,d=dx*dx+dy*dy;if(d<best){best=d;o=i;}}
        std::vector<ProjectedPoint> merged;for(std::size_t i=0;i<=o;++i)merged.push_back(polygon[i]);for(std::size_t i=0;i<hole.size();++i)merged.push_back(hole[(h+i)%hole.size()]);merged.push_back(hole[h]);merged.push_back(polygon[o]);for(std::size_t i=o+1;i<polygon.size();++i)merged.push_back(polygon[i]);polygon=std::move(merged);
    }
    for(const auto& triangle:triangulate(std::move(polygon)))out.push_back({triangle,triangleNormal(triangle[0],triangle[1],triangle[2],basisNormal),target});
}

void appendCylinder(const topology::Solid& solid, const topology::Face& face,
    interaction::TopologyFaceTarget target, bool reversed, std::vector<FaceTriangle>& out)
{
    const auto& cylinder = std::get<geometry::Cylinder>(face.support());
    const auto radialNormal = [&](const geometry::Point3& a, const geometry::Point3& b,
        const geometry::Point3& c)
    {
        const auto centroid = geometry::Point3{
            (a.x() + b.x() + c.x()) / 3.0,
            (a.y() + b.y() + c.y()) / 3.0,
            (a.z() + b.z() + c.z()) / 3.0};
        const auto fromAxisOrigin = centroid - cylinder.origin();
        auto radial = (fromAxisOrigin - cylinder.axis() * math::dot(fromAxisOrigin, cylinder.axis())).normalized();
        if (reversed) radial = -radial;
        return radial;
    };
    std::vector<std::vector<geometry::Point3>> arcs;
    for(const auto& use:solid.find(face.outerWire()).uses())
        if(std::holds_alternative<geometry::CircularArc3>(solid.find(use.edge).geometry()))
            arcs.push_back(edgePoints(solid.find(use.edge),use.orientation==topology::EdgeOrientation::Reversed));
    if(arcs.size()!=2) return;
    const auto pointDistance=[](const geometry::Point3&a,const geometry::Point3&b){return (b-a).length();};
    if(pointDistance(arcs[0].front(),arcs[1].front()) > pointDistance(arcs[0].front(),arcs[1].back()))
        std::reverse(arcs[1].begin(),arcs[1].end());
    for(std::size_t i=0;i+1<std::min(arcs[0].size(),arcs[1].size());++i)
    {
        const auto& a=arcs[0][i]; const auto& b=arcs[0][i+1];
        const auto& c=arcs[1][i+1]; const auto& d=arcs[1][i];
        out.push_back({{a,b,c},triangleNormal(a,b,c,radialNormal(a,b,c)),target});
        out.push_back({{a,c,d},triangleNormal(a,c,d,radialNormal(a,c,d)),target});
    }
}
}

SolidDisplayData deriveSolidDisplay(const topology::Solid& solid, features::FeatureId feature,
    interaction::BodyInstanceContext body)
{
    if(!solid.isValid()||!feature.isValid()) throw std::invalid_argument{"Solid display requires valid inputs"};
    SolidDisplayData result;
    for(std::size_t i=0;i<solid.edges().size();++i)
    {
        const auto edge = topology::EdgeId{static_cast<std::uint32_t>(i)};
        if (!isCoplanarSeam(solid, edge))
            result.edges.push_back({edgePoints(solid.edges()[i],false),{feature,body,edge}});
    }
    for(const auto& use:solid.rootShell().uses())
    {
        const auto& face=solid.find(use.face);
        math::Vector3 outward=std::visit([](const auto& support){
            using T=std::decay_t<decltype(support)>;
            if constexpr(std::is_same_v<T,geometry::Plane>) return support.normal();
            else return math::Vector3{1,0,0};
        },face.support());
        if(use.orientation==topology::FaceOrientation::Reversed) outward=-outward;
        const interaction::TopologyFaceTarget target{feature,body,use.face};
        if(std::holds_alternative<geometry::Plane>(face.support())) appendPlanar(solid,face,target,outward,result.triangles);
        else appendCylinder(solid,face,target,use.orientation==topology::FaceOrientation::Reversed,result.triangles);
    }
    return result;
}
}
