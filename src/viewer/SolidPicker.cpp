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
struct ClosestScreenPoint { math::Scalar distance,x,y,u; };
ClosestScreenPoint closestOnSegment(math::Scalar px,math::Scalar py,const ScreenPoint&a,const ScreenPoint&b)
{
    const auto dx=b.x-a.x;const auto dy=b.y-a.y;const auto lengthSquared=dx*dx+dy*dy;
    const auto u=lengthSquared==0?0.0:std::clamp(((px-a.x)*dx+(py-a.y)*dy)/lengthSquared,0.0,1.0);
    const auto x=a.x+u*dx,y=a.y+u*dy;
    return {std::hypot(px-x,py-y),x,y,u};
}
}
std::optional<SolidPickHit> pickSolid(const presentation::SolidDisplayData& display,const PickingContext& context,math::Scalar tolerance)
{
    context.validate();const auto ray=makePickingRay(context);math::Scalar faceDepth=std::numeric_limits<math::Scalar>::infinity();const presentation::FaceTriangle*face{};
    for(const auto& triangle:display.triangles)if(const auto distance=triangleHit(ray,triangle);distance&&*distance<faceDepth){faceDepth=*distance;face=&triangle;}
    math::Scalar edgeScreen=tolerance;math::Scalar edgeDepth=std::numeric_limits<math::Scalar>::infinity();const presentation::PresentedEdge*edge{};
    for(const auto& candidate:display.edges)for(std::size_t i=1;i<candidate.points.size();++i){
        const auto a=projectWorldToScreen(candidate.points[i-1],context);const auto b=projectWorldToScreen(candidate.points[i],context);if(!a||!b)continue;
        const auto closest=closestOnSegment(context.mouseX,context.mouseY,*a,*b);
        if(closest.distance>edgeScreen)continue;
        auto edgeContext=context;edgeContext.mouseX=closest.x;edgeContext.mouseY=closest.y;
        const auto edgeRay=makePickingRay(edgeContext);
        const auto& first=candidate.points[i-1];const auto& second=candidate.points[i];
        auto along=closest.u;
        if(context.projection.mode()==ProjectionMode::Perspective){
            const auto firstDepth=math::dot(first-edgeRay.origin(),context.camera.forward());
            const auto secondDepth=math::dot(second-edgeRay.origin(),context.camera.forward());
            const auto denominator=(1-closest.u)*secondDepth+closest.u*firstDepth;
            if(denominator<=0)continue;
            along=closest.u*firstDepth/denominator;
        }
        const auto edgePoint=first+(second-first)*along;
        const auto depth=(edgePoint-edgeRay.origin()).length();
        math::Scalar occluderDepth=std::numeric_limits<math::Scalar>::infinity();
        for(const auto& triangle:display.triangles)
            if(const auto hit=triangleHit(edgeRay,triangle);hit&&*hit<occluderDepth)
                occluderDepth=*hit;
        if(depth<=occluderDepth+1e-6){edgeScreen=closest.distance;edgeDepth=depth;edge=&candidate;}}
    if(edge)return SolidPickHit{edge->target,edgeDepth};if(face)return SolidPickHit{face->target,faceDepth};return{};
}
}
