#include "viewer/ReferenceAxes.h"

namespace microsw::viewer
{

std::array<math::Vector3, 2> ReferenceAxes::xAxis() const noexcept
{
    return {math::Vector3{}, math::Vector3{length(), 0.0, 0.0}};
}

std::array<math::Vector3, 2> ReferenceAxes::yAxis() const noexcept
{
    return {math::Vector3{}, math::Vector3{0.0, length(), 0.0}};
}

std::array<math::Vector3, 2> ReferenceAxes::zAxis() const noexcept
{
    return {math::Vector3{}, math::Vector3{0.0, 0.0, length()}};
}

}
