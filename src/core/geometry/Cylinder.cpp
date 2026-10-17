#include "core/geometry/Cylinder.h"
#include <cmath>
#include <stdexcept>
namespace microsw::geometry { Cylinder::Cylinder(const Point3&o,const math::Vector3&a,math::Scalar r):origin_{o},axis_{a.normalized()},radius_{r}{if(!std::isfinite(r)||r<=0)throw std::invalid_argument{"Cylinder radius must be finite and positive"};} }
