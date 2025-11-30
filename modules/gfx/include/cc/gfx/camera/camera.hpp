#pragma once
#include <cc/core/core.hpp>
#include <cc/math/math.hpp>
#include <string>
#include <string_view>

namespace cc::gfx {

enum class CameraProjectionType : u8 {
    Perspective,
    Orthographic
};

struct CameraProjection {
    CameraProjectionType type{CameraProjectionType::Perspective};

    f32 yfov{cc::rad(60.0f)};
    f32 aspect{16.0f / 9.0f};
    f32 znear{0.1f};
    f32 zfar{1000.0f};

    f32 xmag{1.0f};
    f32 ymag{1.0f};
};

class Camera {
public:
    struct Desc {
        std::string   name;
        CameraProjection projection{};
        cc::vec3f     position{0.0f, 0.0f, 0.0f};
        cc::vec3f     target{0.0f, 0.0f, -1.0f};
        cc::vec3f     up{0.0f, 1.0f, 0.0f};
    };

    class Builder {
    public:
        Builder& SetName(std::string_view name) noexcept;
        Builder& SetProjection(const CameraProjection& proj) noexcept;
        Builder& SetPosition(const cc::vec3f& pos) noexcept;
        Builder& SetTarget(const cc::vec3f& target) noexcept;
        Builder& SetUp(const cc::vec3f& up) noexcept;

        [[nodiscard]] Camera Build() const;

    private:
        Desc desc_{};
    };

    Camera() = default;
    explicit Camera(const Desc& desc);

    [[nodiscard]] const std::string& GetName() const noexcept { return name_; }
    [[nodiscard]] const CameraProjection& GetProjection() const noexcept { return projDesc_; }

    [[nodiscard]] const cc::vec3f& GetPosition() const noexcept { return position_; }
    [[nodiscard]] const cc::vec3f& GetTarget()   const noexcept { return target_; }
    [[nodiscard]] const cc::vec3f& GetUp()       const noexcept { return up_; }

    [[nodiscard]] const cc::mat4f& GetView()             const noexcept { return view_; }
    [[nodiscard]] const cc::mat4f& GetProjectionMatrix() const noexcept { return proj_; }
    [[nodiscard]] const cc::mat4f& GetViewProjection()   const noexcept { return viewProj_; }

    void SetProjection(const CameraProjection& proj) noexcept;
    void SetPose(const cc::vec3f& position, const cc::vec3f& target, const cc::vec3f& up) noexcept;
    void SetAspectRatio(f32 aspect) noexcept;

    [[nodiscard]] cc::vec3f GetForward() const noexcept {
        return (target_ - position_).norm();
    }

    [[nodiscard]] cc::vec3f GetRight() const noexcept {
        cc::vec3f f = GetForward();
        cc::vec3f u = up_;
        cc::vec3f r{
            f[1] * u[2] - f[2] * u[1],
            f[2] * u[0] - f[0] * u[2],
            f[0] * u[1] - f[1] * u[0]
        };
        return r.norm();
    }

    [[nodiscard]] cc::vec3f GetUpDerived() const noexcept {
        cc::vec3f r = GetRight();
        cc::vec3f f = GetForward();
        cc::vec3f u{
            r[1] * f[2] - r[2] * f[1],
            r[2] * f[0] - r[0] * f[2],
            r[0] * f[1] - r[1] * f[0]
        };
        return u.norm();
    }

private:
    std::string     name_;
    CameraProjection projDesc_{};

    cc::vec3f position_{0.0f, 0.0f, 0.0f};
    cc::vec3f target_{0.0f, 0.0f, -1.0f};
    cc::vec3f up_{0.0f, 1.0f, 0.0f};

    cc::mat4f view_{cc::mat4f::identity()};
    cc::mat4f proj_{cc::mat4f::identity()};
    cc::mat4f viewProj_{cc::mat4f::identity()};

    void RebuildView() noexcept;
    void RebuildProjection() noexcept;
    void RebuildViewProjection() noexcept;
};

} // namespace cc::gfx
