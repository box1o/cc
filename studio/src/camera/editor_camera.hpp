#pragma once
#include <cc/core/core.hpp>
#include <cc/math/math.hpp>
#include <cc/gfx/gfx.hpp>
#include "camera.hpp"

namespace cc{

struct EditorCameraConfig {
    cc::vec3f pivot{0.0f, 0.0f, 0.0f};

    f32 distance{5.0f};
    f32 minDistance{0.1f};
    f32 maxDistance{500.0f};

    f32 yaw{0.0f};
    f32 pitch{0.0f};
    f32 minPitch{-cc::radians(89.0f)};
    f32 maxPitch{ cc::radians(89.0f)};

    f32 rotateSpeed{0.005f};
    f32 panSpeed{0.005f};
    f32 dollySpeed{0.5f};

    cc::gfx::events::MouseButton orbitButton{cc::gfx::events::MouseButton::Middle};
    cc::gfx::events::MouseButton panButton{cc::gfx::events::MouseButton::Right};
    cc::gfx::events::KeyCode     fastModifier{cc::gfx::events::KeyCode::LeftShift};
};

class EditorCamera {
public:
    struct Desc {
        std::string        name;
        CameraProjection   projection{};
        EditorCameraConfig config{};
        cc::vec3f          initialPivot{0.0f, 0.0f, 0.0f};

        ref<cc::gfx::events::EventBus> eventBus{};
        u32                   viewportWidth{1280};
        u32                   viewportHeight{720};
    };

    class Builder {
    public:
        Builder& SetName(std::string_view name) noexcept;
        Builder& SetProjection(const CameraProjection& proj) noexcept;
        Builder& SetConfig(const EditorCameraConfig& cfg) noexcept;
        Builder& SetInitialPivot(const cc::vec3f& pivot) noexcept;

        Builder& SetEventBus(const ref<cc::gfx::events::EventBus>& bus) noexcept;
        Builder& SetViewportSize(u32 width, u32 height) noexcept;

        [[nodiscard]] EditorCamera Build() const;

    private:
        Desc desc_{};
    };

    EditorCamera() = default;
    explicit EditorCamera(const Desc& desc);
    ~EditorCamera();

    EditorCamera(const EditorCamera&) = delete;
    EditorCamera& operator=(const EditorCamera&) = delete;
    EditorCamera(EditorCamera&&) noexcept = default;
    EditorCamera& operator=(EditorCamera&&) noexcept = default;

    [[nodiscard]] Camera&       GetCamera() noexcept { return camera_; }
    [[nodiscard]] const Camera& GetCamera() const noexcept { return camera_; }

    [[nodiscard]] EditorCameraConfig&       GetConfig() noexcept { return config_; }
    [[nodiscard]] const EditorCameraConfig& GetConfig() const noexcept { return config_; }

    void SetViewportSize(u32 width, u32 height) noexcept;
    void Tick(f32 deltaTime) noexcept;

    void Focus(const cc::vec3f& center, f32 radius) noexcept;
    void SetPivot(const cc::vec3f& pivot) noexcept;
    void SetDistance(f32 distance) noexcept;
    void SetAngles(f32 yaw, f32 pitch) noexcept;

private:
    Camera                camera_;
    EditorCameraConfig    config_{};
    ref<cc::gfx::events::EventBus> eventBus_{};

    cc::gfx::events::ScopedConnection connMouseMove_;
    cc::gfx::events::ScopedConnection connMouseScroll_;
    cc::gfx::events::ScopedConnection connMouseButtonPress_;
    cc::gfx::events::ScopedConnection connMouseButtonRelease_;
    cc::gfx::events::ScopedConnection connKeyPress_;
    cc::gfx::events::ScopedConnection connKeyRelease_;
    cc::gfx::events::ScopedConnection connViewportResize_;

    bool orbiting_{false};
    bool panning_{false};
    bool fastMode_{false};

    cc::vec2f lastCursor_{0.0f, 0.0f};

    void SubscribeEvents();
    void UnsubscribeEvents();

    void OnMouseMoved(const cc::gfx::events::MouseMovedEvent& e);
    void OnMouseScrolled(const cc::gfx::events::MouseScrolledEvent& e);
    void OnMouseButtonPressed(const cc::gfx::events::MouseButtonPressedEvent& e);
    void OnMouseButtonReleased(const cc::gfx::events::MouseButtonReleasedEvent& e);
    void OnKeyPressed(const cc::gfx::events::KeyPressedEvent& e);
    void OnKeyReleased(const cc::gfx::events::KeyReleasedEvent& e);
    void OnViewportResize(const cc::gfx::events::ViewportResizeEvent& e);

    void RebuildOrbitPose() noexcept;
};

} // namespace cc
