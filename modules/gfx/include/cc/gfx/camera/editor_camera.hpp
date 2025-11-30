#pragma once
#include <cc/core/core.hpp>
#include <cc/math/math.hpp>
#include <cc/gfx/camera/camera.hpp>
#include <cc/gfx/events/bus.hpp>
#include <cc/gfx/events/scoped_connection.hpp>
#include <cc/gfx/events/input/events.hpp>
#include <cc/gfx/events/render/events.hpp>
#include <cc/gfx/events/window/events.hpp>

namespace cc::gfx {

struct EditorCameraConfig {
    cc::vec3f pivot{0.0f, 0.0f, 0.0f};

    f32 distance{5.0f};
    f32 minDistance{0.1f};
    f32 maxDistance{500.0f};

    f32 yaw{0.0f};
    f32 pitch{0.0f};
    f32 minPitch{-cc::rad(89.0f)};
    f32 maxPitch{ cc::rad(89.0f)};

    f32 rotateSpeed{0.005f};
    f32 panSpeed{0.005f};
    f32 dollySpeed{0.5f};

    events::MouseButton orbitButton{events::MouseButton::Middle};
    events::MouseButton panButton{events::MouseButton::Right};
    events::KeyCode     fastModifier{events::KeyCode::LeftShift};
};

class EditorCamera {
public:
    struct Desc {
        std::string        name;
        CameraProjection   projection{};
        EditorCameraConfig config{};
        cc::vec3f          initialPivot{0.0f, 0.0f, 0.0f};

        ref<events::EventBus> eventBus{};
        u32                   viewportWidth{1280};
        u32                   viewportHeight{720};
    };

    class Builder {
    public:
        Builder& SetName(std::string_view name) noexcept;
        Builder& SetProjection(const CameraProjection& proj) noexcept;
        Builder& SetConfig(const EditorCameraConfig& cfg) noexcept;
        Builder& SetInitialPivot(const cc::vec3f& pivot) noexcept;

        Builder& SetEventBus(const ref<events::EventBus>& bus) noexcept;
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
    ref<events::EventBus> eventBus_{};

    events::ScopedConnection connMouseMove_;
    events::ScopedConnection connMouseScroll_;
    events::ScopedConnection connMouseButtonPress_;
    events::ScopedConnection connMouseButtonRelease_;
    events::ScopedConnection connKeyPress_;
    events::ScopedConnection connKeyRelease_;
    events::ScopedConnection connViewportResize_;

    bool orbiting_{false};
    bool panning_{false};
    bool fastMode_{false};

    cc::vec2f lastCursor_{0.0f, 0.0f};

    void SubscribeEvents();
    void UnsubscribeEvents();

    void OnMouseMoved(const events::MouseMovedEvent& e);
    void OnMouseScrolled(const events::MouseScrolledEvent& e);
    void OnMouseButtonPressed(const events::MouseButtonPressedEvent& e);
    void OnMouseButtonReleased(const events::MouseButtonReleasedEvent& e);
    void OnKeyPressed(const events::KeyPressedEvent& e);
    void OnKeyReleased(const events::KeyReleasedEvent& e);
    void OnViewportResize(const events::ViewportResizeEvent& e);

    void RebuildOrbitPose() noexcept;
};

} // namespace cc::gfx
