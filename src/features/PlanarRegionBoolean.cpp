#include "features/PlanarRegionBoolean.h"

#include "core/geometry/GeometricTolerance.h"

#include <algorithm>
#include <cmath>
#include <numbers>
#include <stdexcept>
#include <type_traits>

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
    result.erase(std::unique(result.begin(),result.end(),[](auto a,auto b){return std::abs(a-b)<=geometry::defaultGeometricTolerance;}),result.end());
    return result;
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
