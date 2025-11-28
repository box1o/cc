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

    ResourceHandle() : type(ResourceType::None), ptr(nullptr) {}

    explicit ResourceHandle(Texture* tex) 
    : type(ResourceType::Texture), texture(tex) {}

    explicit ResourceHandle(Buffer* buf) 
    : type(ResourceType::Buffer), buffer(buf) {}

    static ResourceHandle Backbuffer() {
        ResourceHandle handle;
        handle. type = ResourceType::Backbuffer;
        handle.ptr = nullptr;
        return handle;
    }

    bool IsValid() const { return type != ResourceType::None; }
    bool IsBackbuffer() const { return type == ResourceType::Backbuffer; }
    bool IsTexture() const { return type == ResourceType::Texture; }
    bool IsBuffer() const { return type == ResourceType::Buffer; }

    Texture* GetTexture() const { return IsTexture() ? texture : nullptr; }
    Buffer* GetBuffer() const { return IsBuffer() ? buffer : nullptr; }

    bool operator==(const ResourceHandle& other) const {
        if (type != other. type) return false;
        return ptr == other.ptr;
    }

    bool operator!=(const ResourceHandle& other) const {
        return !(*this == other);
    }
};

struct ResourceAccess {
    ResourceHandle handle;
    ResourceState state{ResourceState::Undefined};
};

} // namespace cc::gfx
