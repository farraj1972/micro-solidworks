#include "core/math/Matrix4.h"

namespace microsw::math
{

bool almostEqual(
    const Matrix4& first,
    const Matrix4& second,
    Scalar absoluteTolerance,
    Scalar relativeTolerance)
{
    for (std::size_t row = 0; row < 4; ++row)
    {
        for (std::size_t column = 0; column < 4; ++column)
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
