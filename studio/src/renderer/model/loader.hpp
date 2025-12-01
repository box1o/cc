#pragma once
#include <cc/core/core.hpp>
#include <cc/gfx/types.hpp>
#include <filesystem>
#include <string_view>
#include <atomic>
#include <thread>

#include "model.hpp"

namespace cc::gfx {

class Device;

enum class LoadMode : u8 {
    Sync,
    Async
};

enum class MeshLoadFlag : u32 {
    None          = 0,
    FlipTexcoordY = 1 << 0
};

constexpr inline MeshLoadFlag operator|(MeshLoadFlag a, MeshLoadFlag b) {
    return static_cast<MeshLoadFlag>(static_cast<u32>(a) | static_cast<u32>(b));
}

constexpr inline bool operator&(MeshLoadFlag a, MeshLoadFlag b) {
    return (static_cast<u32>(a) & static_cast<u32>(b)) != 0;
}

struct MeshLoadOptions {
    LoadMode     mode{LoadMode::Sync};
    MeshLoadFlag flags{MeshLoadFlag::None};
};

class ResourceLoader {
public:
    ~ResourceLoader() = default;

    [[nodiscard]] static scope<ResourceLoader> Create(Device* device) noexcept;

    [[nodiscard]] Device* GetDevice() const noexcept { return device_; }

    [[nodiscard]] ref<Buffer> CreateVertexBuffer(const void* data, u64 size) const;
    [[nodiscard]] ref<Buffer> CreateIndexBuffer(const void* data, u64 size, IndexType indexType) const;

    [[nodiscard]] ref<Texture2D> CreateTexture2DFromMemory(
        const void* data,
        u32 width,
        u32 height,
        TextureFormat format,
        bool srgb
    ) const;

    [[nodiscard]] ref<Texture2D> CreateTexture2DFallback(TextureFormat format, bool srgb) const;

    [[nodiscard]] ref<Sampler> CreateDefaultSampler(bool srgb) const;

private:
    explicit ResourceLoader(Device* device) noexcept;

    Device* device_{nullptr};
};

class MeshLoader {
public:
    ~MeshLoader();

    [[nodiscard]] static scope<MeshLoader> FromFile(
        ResourceLoader* resourceLoader,
        const std::filesystem::path& path,
        const MeshLoadOptions& options = {}
    );

    void Wait();
    [[nodiscard]] bool IsReady() const noexcept { return ready_.load(std::memory_order_acquire); }
    [[nodiscard]] bool HasError() const noexcept { return hasError_.load(std::memory_order_acquire); }
    [[nodiscard]] std::string_view GetErrorMessage() const noexcept { return errorMessage_; }

    [[nodiscard]] const Scene& GetScene() const noexcept { return scene_; }

private:
    MeshLoader(
        ResourceLoader* resourceLoader,
        const std::filesystem::path& path,
        const MeshLoadOptions& options
    ) noexcept;

    void LoadSync();
    void LoadAsync();
    void DoLoad();

    ResourceLoader* resourceLoader_{nullptr};
    std::filesystem::path path_;
    MeshLoadOptions options_{};

    Scene scene_;

    std::atomic<bool> ready_{false};
    std::atomic<bool> hasError_{false};
    std::string errorMessage_;

    std::thread worker_;
};

} // namespace cc::gfx
