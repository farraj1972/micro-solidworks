#include "core/geometry/CircularArc3.h"
#include "core/math/Vector3.h"
#include <cmath>
#include <numbers>
#include <stdexcept>
namespace microsw::geometry {
CircularArc3::CircularArc3(const Point3& c,const math::Vector3& n,const math::Vector3& x,math::Scalar r,math::Scalar s,math::Scalar w):center_{c},normal_{n.normalized()},xAxis_{x.normalized()},yAxis_{math::cross(normal_,xAxis_).normalized()},radius_{r},start_{s},sweep_{w}{
 if(!std::isfinite(r)||r<=0||!std::isfinite(s)||!std::isfinite(w)||std::abs(w)<=0||std::abs(w)>2*std::numbers::pi)throw std::invalid_argument{"invalid CircularArc3"};
 if(std::abs(math::dot(normal_,xAxis_))>1e-9)throw std::invalid_argument{"CircularArc3 axes must be perpendicular"};}
Point3 CircularArc3::pointAt(math::Scalar t)const{if(!std::isfinite(t)||t<0||t>1)throw std::invalid_argument{"CircularArc3 parameter must be in [0,1]"};const auto a=start_+sweep_*t;return center_+xAxis_*(radius_*std::cos(a))+yAxis_*(radius_*std::sin(a));}
}
