#pragma once
#include <cc/core/core.hpp>
#include <cc/gfx/types.hpp>
#include <cc/gfx/fwd.hpp>

namespace cc::gfx {

struct ResourceHandle {
    ResourceType type{ResourceType::None};

    union {
        Texture* texture;
        Buffer* buffer;
        void* ptr;
    };

    ResourceHandle() noexcept
        : type(ResourceType::None)
        , ptr(nullptr) {}

    explicit ResourceHandle(Texture* tex) noexcept
        : type(ResourceType::Texture)
        , texture(tex) {}

    explicit ResourceHandle(Buffer* buf) noexcept
        : type(ResourceType::Buffer)
        , buffer(buf) {}

    [[nodiscard]] static ResourceHandle Backbuffer() noexcept {
        ResourceHandle handle;
        handle.type = ResourceType::Backbuffer;
        handle.ptr = nullptr;
        return handle;
    }

    [[nodiscard]] bool IsValid() const noexcept { return type != ResourceType::None; }
    [[nodiscard]] bool IsBackbuffer() const noexcept { return type == ResourceType::Backbuffer; }
    [[nodiscard]] bool IsTexture() const noexcept { return type == ResourceType::Texture; }
    [[nodiscard]] bool IsBuffer() const noexcept { return type == ResourceType::Buffer; }

    [[nodiscard]] Texture* GetTexture() const noexcept { return IsTexture() ? texture : nullptr; }
    [[nodiscard]] Buffer* GetBuffer() const noexcept { return IsBuffer() ? buffer : nullptr; }

    [[nodiscard]] bool operator==(const ResourceHandle& other) const noexcept {
        if (type != other.type) {
            return false;
        }
        return ptr == other.ptr;
    }

    [[nodiscard]] bool operator!=(const ResourceHandle& other) const noexcept {
        return !(*this == other);
    }
};

struct ResourceAccess {
    ResourceHandle handle{};
    ResourceState state{ResourceState::Undefined};
};

} // namespace cc::gfx
