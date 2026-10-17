#include "viewer/SolidPicker.h"
#include "viewer/PickingRay.h"
#include "viewer/ViewProjection.h"
#include "viewer/GeometryPicker.h"
#include <algorithm>
#include <cmath>
#include <limits>
namespace microsw::viewer {
namespace {
std::optional<math::Scalar> triangleHit(const geometry::Ray3& ray,const presentation::FaceTriangle& triangle)
{
    const auto e1=triangle.points[1]-triangle.points[0]; const auto e2=triangle.points[2]-triangle.points[0];
    const auto p=math::cross(ray.direction(),e2); const auto det=math::dot(e1,p); if(std::abs(det)<1e-12)return{};
    const auto inv=1.0/det; const auto s=ray.origin()-triangle.points[0]; const auto u=math::dot(s,p)*inv; if(u<0||u>1)return{};
    const auto q=math::cross(s,e1); const auto v=math::dot(ray.direction(),q)*inv; if(v<0||u+v>1)return{};
    const auto distance=math::dot(e2,q)*inv; return distance>0?std::optional{distance}:std::nullopt;
}
math::Scalar segmentDistance(math::Scalar px,math::Scalar py,const ScreenPoint&a,const ScreenPoint&b)
{
    const auto dx=b.x-a.x;const auto dy=b.y-a.y;const auto lengthSquared=dx*dx+dy*dy;
    if(lengthSquared==0)return std::hypot(px-a.x,py-a.y);
    const auto u=std::clamp(((px-a.x)*dx+(py-a.y)*dy)/lengthSquared,0.0,1.0);
    return std::hypot(px-(a.x+u*dx),py-(a.y+u*dy));
}
}
std::optional<SolidPickHit> pickSolid(const presentation::SolidDisplayData& display,const PickingContext& context,math::Scalar tolerance)
{
    context.validate();const auto ray=makePickingRay(context);math::Scalar faceDepth=std::numeric_limits<math::Scalar>::infinity();const presentation::FaceTriangle*face{};
    for(const auto& triangle:display.triangles)if(const auto distance=triangleHit(ray,triangle);distance&&*distance<faceDepth){faceDepth=*distance;face=&triangle;}
    math::Scalar edgeScreen=tolerance;math::Scalar edgeDepth=std::numeric_limits<math::Scalar>::infinity();const presentation::PresentedEdge*edge{};
    for(const auto& candidate:display.edges)for(std::size_t i=1;i<candidate.points.size();++i){
        const auto a=projectWorldToScreen(candidate.points[i-1],context);const auto b=projectWorldToScreen(candidate.points[i],context);if(!a||!b)continue;
        const auto screenDistance=segmentDistance(context.mouseX,context.mouseY,*a,*b);
        const geometry::Point3 midpoint{(candidate.points[i-1].x()+candidate.points[i].x())/2,(candidate.points[i-1].y()+candidate.points[i].y())/2,(candidate.points[i-1].z()+candidate.points[i].z())/2};
        const auto depth=(midpoint-ray.origin()).length();if(screenDistance<=edgeScreen&&depth<=faceDepth+1e-6){edgeScreen=screenDistance;edgeDepth=depth;edge=&candidate;}}
    if(edge)return SolidPickHit{edge->target,edgeDepth};if(face)return SolidPickHit{face->target,faceDepth};return{};
}
}
