#include "presentation/SolidDisplay.h"

#include <algorithm>
#include <cmath>
#include <numbers>
#include <stdexcept>
#include <type_traits>

namespace microsw::presentation
{
namespace
{
constexpr int curveSegments = 24;

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

void appendPlanar(const topology::Solid& solid, const topology::Face& face,
    interaction::TopologyFaceTarget target, const math::Vector3& outward, std::vector<FaceTriangle>& out)
{
    const auto outer = wirePoints(solid, face.outerWire());
    if (outer.size() < 3) return;
    // Fan triangulation is exact for the convex MVP faces and deterministic.
    // General holed caps are filtered by rejecting triangles whose centroid lies
    // inside an inner wire; curved boundaries are already sampled consistently.
    const auto basisNormal = outward;
    const auto& plane = std::get<geometry::Plane>(face.support());
    const auto axisX = std::abs(plane.normal().x()) < 0.9
        ? math::cross(plane.normal(), {1,0,0}).normalized()
        : math::cross(plane.normal(), {0,1,0}).normalized();
    const auto axisY = math::cross(plane.normal(), axisX);
    const auto inside = [&](const geometry::Point3& point, topology::WireId wire)
    {
        const auto polygon = wirePoints(solid, wire);
        const auto px=math::dot(point-plane.origin(),axisX), py=math::dot(point-plane.origin(),axisY);
        bool value=false;
        for(std::size_t i=0,j=polygon.size()-1;i<polygon.size();j=i++){
            const auto xi=math::dot(polygon[i]-plane.origin(),axisX), yi=math::dot(polygon[i]-plane.origin(),axisY);
            const auto xj=math::dot(polygon[j]-plane.origin(),axisX), yj=math::dot(polygon[j]-plane.origin(),axisY);
            if(((yi>py)!=(yj>py)) && px < (xj-xi)*(py-yi)/(yj-yi)+xi) value=!value;
        }
        return value;
    };
    for (std::size_t i=1;i+1<outer.size();++i)
    {
        const geometry::Point3 centroid{(outer[0].x()+outer[i].x()+outer[i+1].x())/3,
            (outer[0].y()+outer[i].y()+outer[i+1].y())/3,
            (outer[0].z()+outer[i].z()+outer[i+1].z())/3};
        bool inHole=false; for(auto hole:face.innerWires()) if(inside(centroid,hole)){inHole=true;break;}
        if(!inHole) out.push_back({{outer[0],outer[i],outer[i+1]},
            triangleNormal(outer[0],outer[i],outer[i+1],basisNormal),target});
    }
}

void appendCylinder(const topology::Solid& solid, const topology::Face& face,
    interaction::TopologyFaceTarget target, const math::Vector3& outward, std::vector<FaceTriangle>& out)
{
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
        out.push_back({{a,b,c},triangleNormal(a,b,c,outward),target});
        out.push_back({{a,c,d},triangleNormal(a,c,d,outward),target});
    }
}
}

SolidDisplayData deriveSolidDisplay(const topology::Solid& solid, features::FeatureId feature,
    interaction::BodyInstanceContext body)
{
    if(!solid.isValid()||!feature.isValid()) throw std::invalid_argument{"Solid display requires valid inputs"};
    SolidDisplayData result;
    for(std::size_t i=0;i<solid.edges().size();++i)
        result.edges.push_back({edgePoints(solid.edges()[i],false),{feature,body,topology::EdgeId{static_cast<std::uint32_t>(i)}}});
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
        else appendCylinder(solid,face,target,outward,result.triangles);
    }
    return result;
}
}
