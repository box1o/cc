#include "camera.hpp"

namespace cc{

Camera::Builder& Camera::Builder::SetName(std::string_view name) noexcept {
    desc_.name = std::string(name);
    return *this;
}

Camera::Builder& Camera::Builder::SetProjection(const CameraProjection& proj) noexcept {
    desc_.projection = proj;
    return *this;
}

Camera::Builder& Camera::Builder::SetPosition(const cc::vec3f& pos) noexcept {
    desc_.position = pos;
    return *this;
}

Camera::Builder& Camera::Builder::SetTarget(const cc::vec3f& target) noexcept {
    desc_.target = target;
    return *this;
}

Camera::Builder& Camera::Builder::SetUp(const cc::vec3f& up) noexcept {
    desc_.up = up;
    return *this;
}

Camera Camera::Builder::Build() const {
    return Camera(desc_);
}

Camera::Camera(const Desc& desc)
    : name_(desc.name)
    , projDesc_(desc.projection)
    , position_(desc.position)
    , target_(desc.target)
    , up_(desc.up) {
    RebuildView();
    RebuildProjection();
    RebuildViewProjection();
}

void Camera::SetProjection(const CameraProjection& proj) noexcept {
    projDesc_ = proj;
    RebuildProjection();
    RebuildViewProjection();
}

void Camera::SetPose(const cc::vec3f& position, const cc::vec3f& target, const cc::vec3f& up) noexcept {
    position_ = position;
    target_   = target;
    up_       = up;
    RebuildView();
    RebuildViewProjection();
}

void Camera::SetAspectRatio(f32 aspect) noexcept {
    projDesc_.aspect = aspect;
    RebuildProjection();
    RebuildViewProjection();
}

void Camera::RebuildView() noexcept {
    view_ = cc::look_at(position_, target_, up_);
}

void Camera::RebuildProjection() noexcept {
    if (projDesc_.type == CameraProjectionType::Perspective) {
        proj_ = cc::perspective(
            projDesc_.yfov,
            projDesc_.aspect > 0.0f ? projDesc_.aspect : 1.0f,
            projDesc_.znear,
            projDesc_.zfar
        );
    } else {
        const f32 x = projDesc_.xmag;
        const f32 y = projDesc_.ymag;
        const f32 n = projDesc_.znear;
        const f32 f = projDesc_.zfar;
        proj_ = cc::ortho(-x, x, -y, y, n, f);
    }
}

void Camera::RebuildViewProjection() noexcept {
    viewProj_ = proj_ * view_;
}

} // namespace cc
