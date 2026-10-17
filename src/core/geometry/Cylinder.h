#pragma once
#include "core/geometry/Point3.h"
namespace microsw::geometry { class Cylinder { public: Cylinder(const Point3& origin,const math::Vector3& axis,math::Scalar radius); [[nodiscard]]const Point3& origin()const noexcept{return origin_;} [[nodiscard]]const math::Vector3& axis()const noexcept{return axis_;} [[nodiscard]]math::Scalar radius()const noexcept{return radius_;} private:Point3 origin_;math::Vector3 axis_;math::Scalar radius_;}; }
