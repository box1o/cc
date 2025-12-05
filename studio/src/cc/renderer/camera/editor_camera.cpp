#include "editor_camera.hpp"
#include <cc/core/logger.hpp>

namespace cc{

EditorCamera::Builder& EditorCamera::Builder::SetName(std::string_view name) noexcept {
    desc_.name = std::string(name);
    return *this;
}

EditorCamera::Builder& EditorCamera::Builder::SetProjection(const CameraProjection& proj) noexcept {
    desc_.projection = proj;
    return *this;
}

EditorCamera::Builder& EditorCamera::Builder::SetConfig(const EditorCameraConfig& cfg) noexcept {
    desc_.config = cfg;
    return *this;
}

EditorCamera::Builder& EditorCamera::Builder::SetInitialPivot(const cc::vec3f& pivot) noexcept {
    desc_.initialPivot = pivot;
    return *this;
}

EditorCamera::Builder& EditorCamera::Builder::SetEventBus(const ref<cc::gfx::events::EventBus>& bus) noexcept {
    desc_.eventBus = bus;
    return *this;
}

EditorCamera::Builder& EditorCamera::Builder::SetViewportSize(u32 width, u32 height) noexcept {
    desc_.viewportWidth  = width;
    desc_.viewportHeight = height;
    return *this;
}

scope<EditorCamera> EditorCamera::Builder::Build() const {
    auto camera = scope<EditorCamera>(new EditorCamera(desc_));
    return camera;
}

EditorCamera::EditorCamera(const Desc& desc)
    : config_(desc.config)
    , eventBus_(desc.eventBus) {
    if (!eventBus_) {
        log::Warn("EditorCamera created without EventBus");
    }

    CameraProjection proj = desc.projection;
    if (proj.type == CameraProjectionType::Perspective && proj.aspect <= 0.0f) {
        proj.aspect = desc.viewportHeight > 0
            ? static_cast<f32>(desc.viewportWidth) / static_cast<f32>(desc.viewportHeight)
            : 1.0f;
    }

    config_.pivot = desc.initialPivot;

    const f32 cp = std::cos(config_.pitch);
    const f32 sp = std::sin(config_.pitch);
    const f32 cy = std::cos(config_.yaw);
    const f32 sy = std::sin(config_.yaw);

    cc::vec3f offset{
        cy * config_.distance * cp,
        sp * config_.distance,
        sy * config_.distance * cp
    };

    cc::vec3f position = config_.pivot + offset;

    camera_ = Camera::Builder{}
        .SetName(desc.name)
        .SetProjection(proj)
        .SetPosition(position)
        .SetTarget(config_.pivot)
        .SetUp({0.0f, 1.0f, 0.0f})
        .Build();

    if (proj.type == CameraProjectionType::Perspective) {
        camera_.SetAspectRatio(proj.aspect);
    }

    SubscribeEvents();
}

EditorCamera::~EditorCamera() {
    UnsubscribeEvents();
}

void EditorCamera::SetViewportSize(u32 width, u32 height) noexcept {
    if (width == 0 || height == 0) return;

    if (camera_.GetProjection().type == CameraProjectionType::Perspective) {
        camera_.SetAspectRatio(static_cast<f32>(width) / static_cast<f32>(height));
    }
}

void EditorCamera::Tick(f32) noexcept {
    //NOTE: reserved for time-based behavior if needed
}

void EditorCamera::SubscribeEvents() {
    if (!eventBus_) return;

    connMouseMove_ = eventBus_->On<cc::gfx::events::MouseMovedEvent>(
        [this](const cc::gfx::events::MouseMovedEvent& e) {
            OnMouseMoved(e);
            return false;
        }
    );
    connMouseScroll_ = eventBus_->On<cc::gfx::events::MouseScrolledEvent>(
        [this](const cc::gfx::events::MouseScrolledEvent& e) {
            OnMouseScrolled(e);
            return false;
        }
    );
    connMouseButtonPress_ = eventBus_->On<cc::gfx::events::MouseButtonPressedEvent>(
        [this](const cc::gfx::events::MouseButtonPressedEvent& e) {
            OnMouseButtonPressed(e);
            return false;
        }
    );
    connMouseButtonRelease_ = eventBus_->On<cc::gfx::events::MouseButtonReleasedEvent>(
        [this](const cc::gfx::events::MouseButtonReleasedEvent& e) {
            OnMouseButtonReleased(e);
            return false;
        }
    );
    connKeyPress_ = eventBus_->On<cc::gfx::events::KeyPressedEvent>(
        [this](const cc::gfx::events::KeyPressedEvent& e) {
            OnKeyPressed(e);
            return false;
        }
    );
    connKeyRelease_ = eventBus_->On<cc::gfx::events::KeyReleasedEvent>(
        [this](const cc::gfx::events::KeyReleasedEvent& e) {
            OnKeyReleased(e);
            return false;
        }
    );
    connViewportResize_ = eventBus_->On<cc::gfx::events::ViewportResizeEvent>(
        [this](const cc::gfx::events::ViewportResizeEvent& e) {
            OnViewportResize(e);
            return false;
        }
    );
}

void EditorCamera::UnsubscribeEvents() {
    connMouseMove_.Disconnect();
    connMouseScroll_.Disconnect();
    connMouseButtonPress_.Disconnect();
    connMouseButtonRelease_.Disconnect();
    connKeyPress_.Disconnect();
    connKeyRelease_.Disconnect();
    connViewportResize_.Disconnect();
}

void EditorCamera::OnMouseMoved(const cc::gfx::events::MouseMovedEvent& e) {
    cc::vec2f current{e.x, e.y};
    cc::vec2f delta{e.deltaX, e.deltaY};

    lastCursor_ = current;

    const f32 speedFactor = fastMode_ ? 3.0f : 1.0f;

    if (orbiting_) {
        config_.yaw   -= delta.x * config_.rotateSpeed * speedFactor;
        config_.pitch -= delta.y * config_.rotateSpeed * speedFactor;

        if (config_.pitch < config_.minPitch) config_.pitch = config_.minPitch;
        if (config_.pitch > config_.maxPitch) config_.pitch = config_.maxPitch;

        RebuildOrbitPose();
    } else if (panning_) {
        cc::vec3f forward = (camera_.GetTarget() - camera_.GetPosition()).normalize();
        cc::vec3f up      = camera_.GetUp();

        cc::vec3f right{
            forward[1] * up[2] - forward[2] * up[1],
            forward[2] * up[0] - forward[0] * up[2],
            forward[0] * up[1] - forward[1] * up[0]
        };
        right = right.normalize();

        f32 panScale = config_.panSpeed * speedFactor * config_.distance;
        config_.pivot += (-right * delta.x + up * delta.y) * panScale;

        RebuildOrbitPose();
    }
}

void EditorCamera::OnMouseScrolled(const cc::gfx::events::MouseScrolledEvent& e) {
    f32 dir = -e.offsetY;
    f32 speedFactor = fastMode_ ? 3.0f : 1.0f;

    config_.distance += dir * config_.dollySpeed * speedFactor;
    if (config_.distance < config_.minDistance) config_.distance = config_.minDistance;
    if (config_.distance > config_.maxDistance) config_.distance = config_.maxDistance;

    RebuildOrbitPose();
}

void EditorCamera::OnMouseButtonPressed(const cc::gfx::events::MouseButtonPressedEvent& e) {
    if (e.button == config_.orbitButton) {
        orbiting_ = true;
    } else if (e.button == config_.panButton) {
        panning_ = true;
    }
    lastCursor_ = {e.x, e.y};
}

void EditorCamera::OnMouseButtonReleased(const cc::gfx::events::MouseButtonReleasedEvent& e) {
    if (e.button == config_.orbitButton) {
        orbiting_ = false;
    } else if (e.button == config_.panButton) {
        panning_ = false;
    }
}

void EditorCamera::OnKeyPressed(const cc::gfx::events::KeyPressedEvent& e) {
    if (e.key == config_.fastModifier) {
        fastMode_ = true;
    }
}

void EditorCamera::OnKeyReleased(const cc::gfx::events::KeyReleasedEvent& e) {
    if (e.key == config_.fastModifier) {
        fastMode_ = false;
    }
}

void EditorCamera::OnViewportResize(const cc::gfx::events::ViewportResizeEvent& e) {
    SetViewportSize(e.width, e.height);
}

void EditorCamera::RebuildOrbitPose() noexcept {
    const f32 cp = std::cos(config_.pitch);
    const f32 sp = std::sin(config_.pitch);
    const f32 cy = std::cos(config_.yaw);
    const f32 sy = std::sin(config_.yaw);

    cc::vec3f offset{
        cy * config_.distance * cp,
        sp * config_.distance,
        sy * config_.distance * cp
    };

    cc::vec3f position = config_.pivot + offset;
    camera_.SetPose(position, config_.pivot, {0.0f, 1.0f, 0.0f});
}

void EditorCamera::Focus(const cc::vec3f& center, f32 radius) noexcept {
    config_.pivot = center;

    f32 minDist = config_.minDistance;
    f32 maxDist = config_.maxDistance;
    f32 desired = radius > 0.0f ? radius * 2.5f : config_.distance;

    if (desired < minDist) desired = minDist;
    if (desired > maxDist) desired = maxDist;

    config_.distance = desired;
    RebuildOrbitPose();
}

void EditorCamera::SetPivot(const cc::vec3f& pivot) noexcept {
    config_.pivot = pivot;
    RebuildOrbitPose();
}

void EditorCamera::SetDistance(f32 distance) noexcept {
    if (distance < config_.minDistance) distance = config_.minDistance;
    if (distance > config_.maxDistance) distance = config_.maxDistance;
    config_.distance = distance;
    RebuildOrbitPose();
}

void EditorCamera::SetAngles(f32 yaw, f32 pitch) noexcept {
    config_.yaw   = yaw;
    config_.pitch = pitch;

    if (config_.pitch < config_.minPitch) config_.pitch = config_.minPitch;
    if (config_.pitch > config_.maxPitch) config_.pitch = config_.maxPitch;

    RebuildOrbitPose();
}

} // namespace cc
