#pragma once
#include <cc/math/math.hpp>
#include <cc/core/core.hpp>
#include <string>
#include <vector>
#include <unordered_map>
#include <span>
#include <algorithm>

namespace cc::io {

struct intrinsics {
    std::string name;
    f32 fx{0.0f};
    f32 fy{0.0f};
    f32 cx{0.0f};
    f32 cy{0.0f};

    constexpr intrinsics() noexcept = default;
    constexpr intrinsics(std::string n, f32 fx_, f32 fy_, f32 cx_, f32 cy_) noexcept
    : name(std::move(n)), fx(fx_), fy(fy_), cx(cx_), cy(cy_) {}
};

struct uv {
    std::string marker_name;
    f32 u{0.0f};
    f32 v{0.0f};

    constexpr uv() noexcept = default;
    constexpr uv(std::string name, f32 u_, f32 v_) noexcept
    : marker_name(std::move(name)), u(u_), v(v_) {}

    [[nodiscard]] constexpr bool is_visible() const noexcept {
        return u >= 0.0f && v >= 0.0f;
    }
};

class scene {
    std::vector<intrinsics> cameras_;
    std::unordered_map<std::string, std::vector<uv>> camera_uvs_;

public:
    [[nodiscard]] auto& cameras() noexcept { return cameras_; }
    [[nodiscard]] const auto& cameras() const noexcept { return cameras_; }

    [[nodiscard]] auto& uvs() noexcept { return camera_uvs_; }
    [[nodiscard]] const auto& uvs() const noexcept { return camera_uvs_; }

    [[nodiscard]] const intrinsics* find_camera(std::string_view name) const noexcept {
        auto it = std::ranges::find(cameras_, name, &intrinsics::name);
        return it != cameras_.end() ? &(*it) : nullptr;
    }

    [[nodiscard]] std::span<const uv> uvs_for_camera(std::string_view name) const noexcept {
        auto it = camera_uvs_.find(std::string(name));
        return it != camera_uvs_.end() ? std::span{it->second} : std::span<const uv>{};
    }

    void clear() noexcept {
        cameras_.clear();
        camera_uvs_.clear();
    }
};

} // namespace cc::io
