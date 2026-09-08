#include "core/math/Transform3.h"

#include "core/math/Transformations.h"

#include <cmath>
#include <stdexcept>

namespace microsw::math {
namespace {

[[nodiscard]] bool isFinite(const Vector3& value) noexcept {
    return std::isfinite(value.x()) && std::isfinite(value.y()) && std::isfinite(value.z());
}

void validateFinite(const Vector3& value, const char* message) {
    if (!isFinite(value)) {
        throw std::invalid_argument(message);
    }
}

void validateScale(const Vector3& value) {
    validateFinite(value, "Transform3 scale components must be finite");
    if (value.x() <= 0.0 || value.y() <= 0.0 || value.z() <= 0.0) {
        throw std::invalid_argument("Transform3 scale components must be positive");
    }
}

} // namespace

Transform3::Transform3(
    const Vector3& translationValue,
    const Vector3& rotationValue,
    const Vector3& scaleValue) {
    validateFinite(translationValue, "Transform3 translation components must be finite");
    validateFinite(rotationValue, "Transform3 rotation components must be finite");
    validateScale(scaleValue);

    translation_ = translationValue;
    rotation_ = rotationValue;
    scale_ = scaleValue;
}

const Vector3& Transform3::translation() const noexcept {
    return translation_;
}

const Vector3& Transform3::rotation() const noexcept {
    return rotation_;
}

const Vector3& Transform3::scale() const noexcept {
    return scale_;
}

void Transform3::setTranslation(const Vector3& value) {
    validateFinite(value, "Transform3 translation components must be finite");
    translation_ = value;
}

void Transform3::setRotation(const Vector3& value) {
    validateFinite(value, "Transform3 rotation components must be finite");
    rotation_ = value;
}

void Transform3::setScale(const Vector3& value) {
    validateScale(value);
    scale_ = value;
}

Matrix4 Transform3::matrix() const {
    const Matrix4 rotationMatrix = rotationZ(rotation_.z()) * rotationY(rotation_.y())
                                 * rotationX(rotation_.x());
    return microsw::math::translation(translation_) * rotationMatrix
         * microsw::math::scaling(scale_);
}

} // namespace microsw::math
