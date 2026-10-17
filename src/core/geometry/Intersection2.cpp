#include "core/geometry/Intersection2.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace microsw::geometry
{
namespace
{
void validateTolerance(math::Scalar t)
{
    if (!std::isfinite(t) || t < 0) throw std::invalid_argument{"intersection tolerance must be finite and nonnegative"};
}
math::Scalar cross(math::Scalar ax, math::Scalar ay, math::Scalar bx, math::Scalar by)
{ return ax * by - ay * bx; }
bool same(const Point2& a, const Point2& b, math::Scalar t)
{ return std::hypot(a.x()-b.x(), a.y()-b.y()) <= t; }
void addUnique(std::vector<Point2>& out, const Point2& p, math::Scalar t)
{ if (std::none_of(out.begin(), out.end(), [&](const auto& q){ return same(p,q,t); })) out.push_back(p); }
IntersectionKind classifyFiltered(std::size_t count, bool tangent, bool endpoint)
{
    if (!count) return IntersectionKind::None;
    if (endpoint) return IntersectionKind::Endpoint;
    return tangent || count == 1 ? IntersectionKind::Tangent : IntersectionKind::Proper;
}
}

Intersection2 intersect(const Segment2& a, const Segment2& b, math::Scalar t)
{
    validateTolerance(t);
    if (a.isDegenerate(t) || b.isDegenerate(t)) return {IntersectionKind::Ambiguous,{}};
    const auto rx=a.b().x()-a.a().x(), ry=a.b().y()-a.a().y();
    const auto sx=b.b().x()-b.a().x(), sy=b.b().y()-b.a().y();
    const auto qx=b.a().x()-a.a().x(), qy=b.a().y()-a.a().y();
    const auto den=cross(rx,ry,sx,sy), qxr=cross(qx,qy,rx,ry);
    const auto scale=std::max({std::abs(rx),std::abs(ry),std::abs(sx),std::abs(sy),1.0});
    const auto eps=t*scale;
    if (std::abs(den)<=eps)
    {
        if (std::abs(qxr)>eps) return {};
        const bool useX=std::abs(rx)>=std::abs(ry);
        const auto av=useX?a.a().x():a.a().y(), bv=useX?a.b().x():a.b().y();
        const auto c=useX?b.a().x():b.a().y(), d=useX?b.b().x():b.b().y();
        const auto lo=std::max(std::min(av,bv),std::min(c,d));
        const auto hi=std::min(std::max(av,bv),std::max(c,d));
        if (hi<lo-t) return {};
        if (std::abs(hi-lo)<=t)
        {
            const auto u=std::abs(bv-av)>t?(lo-av)/(bv-av):0.0;
            return {IntersectionKind::Endpoint,{a.pointAt(std::clamp(u,0.0,1.0))}};
        }
        return {IntersectionKind::Overlap,{}};
    }
    const auto ua=cross(qx,qy,sx,sy)/den, ub=cross(qx,qy,rx,ry)/den;
    if (ua < -t || ua > 1+t || ub < -t || ub > 1+t) return {};
    const auto p=a.pointAt(std::clamp(ua,0.0,1.0));
    const bool endpoint=ua<=t||ua>=1-t||ub<=t||ub>=1-t;
    return {endpoint?IntersectionKind::Endpoint:IntersectionKind::Proper,{p}};
}

Intersection2 intersect(const Segment2& s, const Circle2& c, math::Scalar t)
{
    validateTolerance(t);
    if (s.isDegenerate(t)) return {IntersectionKind::Ambiguous,{}};
    const auto dx=s.b().x()-s.a().x(), dy=s.b().y()-s.a().y();
    const auto fx=s.a().x()-c.center().x(), fy=s.a().y()-c.center().y();
    const auto aa=dx*dx+dy*dy, bb=2*(fx*dx+fy*dy), cc=fx*fx+fy*fy-c.radius()*c.radius();
    auto disc=bb*bb-4*aa*cc;
    const auto eps=t*std::max(aa,1.0);
    if (disc < -eps) return {};
    const bool tangent=std::abs(disc)<=eps;
    disc=std::max(0.0,disc);
    std::vector<Point2> points;
    for (const auto u : {(-bb-std::sqrt(disc))/(2*aa),(-bb+std::sqrt(disc))/(2*aa)})
        if (u>=-t&&u<=1+t) addUnique(points,s.pointAt(std::clamp(u,0.0,1.0)),t);
    bool endpoint=false; for(const auto& p:points) endpoint|=same(p,s.a(),t)||same(p,s.b(),t);
    return {classifyFiltered(points.size(),tangent,endpoint),std::move(points)};
}

Intersection2 intersect(const Circle2& a, const Circle2& b, math::Scalar t)
{
    validateTolerance(t);
    const auto dx=b.center().x()-a.center().x(),dy=b.center().y()-a.center().y();
    const auto d=std::hypot(dx,dy);
    if (d<=t && std::abs(a.radius()-b.radius())<=t) return {IntersectionKind::Coincident,{}};
    if (d<=t || d>a.radius()+b.radius()+t || d<std::abs(a.radius()-b.radius())-t) return {};
    const auto x=(a.radius()*a.radius()-b.radius()*b.radius()+d*d)/(2*d);
    auto h2=a.radius()*a.radius()-x*x;
    const bool tangent=std::abs(h2)<=t*std::max(a.radius()*a.radius(),1.0);
    if(h2<0&&!tangent)return{}; h2=std::max(0.0,h2);
    const auto px=a.center().x()+x*dx/d,py=a.center().y()+x*dy/d,h=std::sqrt(h2);
    std::vector<Point2> points{{px-h*dy/d,py+h*dx/d}};
    if(!tangent)addUnique(points,{px+h*dy/d,py-h*dx/d},t);
    return {tangent?IntersectionKind::Tangent:IntersectionKind::Proper,std::move(points)};
}

Intersection2 intersect(const Segment2& s, const Arc2& a, math::Scalar t)
{
    auto r=intersect(s,Circle2{a.center(),a.radius()},t); if(r.kind==IntersectionKind::None)return r;
    std::vector<Point2> p; bool endpoint=false;
    for(const auto& q:r.points)if(a.contains(q,t)){addUnique(p,q,t);endpoint|=same(q,a.startPoint(),t)||same(q,a.endPoint(),t)||same(q,s.a(),t)||same(q,s.b(),t);}
    r.points=std::move(p); r.kind=classifyFiltered(r.points.size(),r.kind==IntersectionKind::Tangent,endpoint); return r;
}
Intersection2 intersect(const Circle2& c, const Arc2& a, math::Scalar t)
{
    auto r=intersect(c,Circle2{a.center(),a.radius()},t);
    if(r.kind==IntersectionKind::Coincident)return {IntersectionKind::Overlap,{}};
    std::vector<Point2> p; bool endpoint=false; for(const auto&q:r.points)if(a.contains(q,t)){addUnique(p,q,t);endpoint|=same(q,a.startPoint(),t)||same(q,a.endPoint(),t);}
    r.points=std::move(p);r.kind=classifyFiltered(r.points.size(),r.kind==IntersectionKind::Tangent,endpoint);return r;
}
Intersection2 intersect(const Arc2& a, const Arc2& b, math::Scalar t)
{
    auto r=intersect(Circle2{a.center(),a.radius()},Circle2{b.center(),b.radius()},t);
    if(r.kind==IntersectionKind::Coincident)
    {
        const bool touching=a.contains(b.startPoint(),t)||a.contains(b.endPoint(),t)||b.contains(a.startPoint(),t)||b.contains(a.endPoint(),t);
        return {touching?IntersectionKind::Overlap:IntersectionKind::None,{}};
    }
    std::vector<Point2> p;bool endpoint=false;for(const auto&q:r.points)if(a.contains(q,t)&&b.contains(q,t)){addUnique(p,q,t);endpoint|=same(q,a.startPoint(),t)||same(q,a.endPoint(),t)||same(q,b.startPoint(),t)||same(q,b.endPoint(),t);}
    r.points=std::move(p);r.kind=classifyFiltered(r.points.size(),r.kind==IntersectionKind::Tangent,endpoint);return r;
}
}
