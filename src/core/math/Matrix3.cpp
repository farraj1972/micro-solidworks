#include "core/math/Matrix3.h"

namespace microsw::math
{

bool almostEqual(
    const Matrix3& first,
    const Matrix3& second,
    Scalar absoluteTolerance,
    Scalar relativeTolerance)
{
    for (std::size_t row = 0; row < 3; ++row)
    {
        for (std::size_t column = 0; column < 3; ++column)
        {
            if (!almostEqual(
                    first(row, column),
                    second(row, column),
                    absoluteTolerance,
                    relativeTolerance))
            {
                return false;
            }
        }
    }

    return true;
}

}
