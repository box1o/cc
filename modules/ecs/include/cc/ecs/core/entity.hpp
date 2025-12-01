#pragma once

#include <cc/core/types.hpp>

namespace cc::ecs {

using EntityIndex   = cc::u32;
using EntityVersion = cc::u32;

struct Entity {
    EntityIndex   index{0};
    EntityVersion version{0};

    [[nodiscard]] constexpr bool IsNull() const noexcept {
        return index == 0 && version == 0;
    }

    [[nodiscard]] constexpr explicit operator bool() const noexcept {
        return !IsNull();
    }

    [[nodiscard]] friend constexpr bool operator==(Entity a, Entity b) noexcept {
        return a.index == b.index && a.version == b.version;
    }

    [[nodiscard]] friend constexpr bool operator!=(Entity a, Entity b) noexcept {
        return !(a == b);
    }
};

inline constexpr Entity NullEntity{0, 0};

} // namespace cc::ecs
