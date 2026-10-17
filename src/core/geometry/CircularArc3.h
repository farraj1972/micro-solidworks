#pragma once
#include "core/geometry/Point3.h"
namespace microsw::geometry {
class CircularArc3 {
public:
 CircularArc3(const Point3& center,const math::Vector3& normal,const math::Vector3& xAxis,math::Scalar radius,math::Scalar startAngle,math::Scalar sweepAngle);
 [[nodiscard]] const Point3& center()const noexcept{return center_;} [[nodiscard]] const math::Vector3& normal()const noexcept{return normal_;}
 [[nodiscard]] const math::Vector3& xAxis()const noexcept{return xAxis_;} [[nodiscard]] math::Scalar radius()const noexcept{return radius_;}
 [[nodiscard]] math::Scalar startAngle()const noexcept{return start_;} [[nodiscard]] math::Scalar sweepAngle()const noexcept{return sweep_;}
 [[nodiscard]] Point3 pointAt(math::Scalar t)const;
private: Point3 center_;math::Vector3 normal_,xAxis_,yAxis_;math::Scalar radius_,start_,sweep_;
}; }
