#pragma once

#include "core/math/Matrix4.h"
#include "core/math/Vector3.h"

namespace microsw::math {

class Transform3 {
public:
    Transform3() = default;
    Transform3(const Vector3& translation, const Vector3& rotation, const Vector3& scale);

    [[nodiscard]] const Vector3& translation() const noexcept;
    [[nodiscard]] const Vector3& rotation() const noexcept;
    [[nodiscard]] const Vector3& scale() const noexcept;

    void setTranslation(const Vector3& translation);
    void setRotation(const Vector3& rotation);
    void setScale(const Vector3& scale);

    [[nodiscard]] Matrix4 matrix() const;

private:
    Vector3 translation_{};
    Vector3 rotation_{};
    Vector3 scale_{1.0, 1.0, 1.0};
};

} // namespace microsw::math
