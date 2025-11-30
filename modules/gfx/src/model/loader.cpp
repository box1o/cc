#include <cc/gfx/model/loader.hpp>
#include <cc/gfx/buffer/buffer.hpp>
#include <cc/gfx/texture/texture.hpp>
#include <cc/gfx/sampler/sampler.hpp>
#include <cc/gfx/device/device.hpp>
#include <cc/core/logger.hpp>

#define TINYGLTF_IMPLEMENTATION
#include <tiny_gltf.h>
#include <stdexcept>
#include <cstring>

namespace cc::gfx {

namespace {

[[nodiscard]] inline PrimitiveTopology ToPrimitiveTopology(int gltfMode) noexcept {
    switch (gltfMode) {
        case TINYGLTF_MODE_POINTS:          return PrimitiveTopology::PointList;
        case TINYGLTF_MODE_LINE:            return PrimitiveTopology::LineList;
        case TINYGLTF_MODE_LINE_LOOP:       return PrimitiveTopology::LineStrip;
        case TINYGLTF_MODE_TRIANGLES:       return PrimitiveTopology::TriangleList;
        case TINYGLTF_MODE_TRIANGLE_STRIP:  return PrimitiveTopology::TriangleStrip;
        case TINYGLTF_MODE_TRIANGLE_FAN:    return PrimitiveTopology::TriangleList;
        default:                            return PrimitiveTopology::TriangleList;
    }
}

[[nodiscard]] inline TextureFormat ToTextureFormatRGBA8(bool srgb) noexcept {
    return srgb ? TextureFormat::SRGB8_A8 : TextureFormat::RGBA8;
}

[[nodiscard]] inline AlphaMode ToAlphaMode(const std::string& mode) noexcept {
    if (mode == "MASK")  return AlphaMode::Mask;
    if (mode == "BLEND") return AlphaMode::Blend;
    return AlphaMode::Opaque;
}

[[nodiscard]] inline InterpolationType ToInterpolationType(const std::string& s) noexcept {
    if (s == "STEP")         return InterpolationType::Step;
    if (s == "CUBICSPLINE")  return InterpolationType::CubicSpline;
    return InterpolationType::Linear;
}

[[nodiscard]] inline LightType ToLightType(const std::string& s) noexcept {
    if (s == "directional") return LightType::Directional;
    if (s == "spot")        return LightType::Spot;
    return LightType::Point;
}

[[nodiscard]] IndexType ToIndexType(int componentType) {
    switch (componentType) {
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: return IndexType::U16;
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:   return IndexType::U32;
        default:
            throw std::runtime_error("Unsupported index component type");
    }
}

[[nodiscard]] inline u32 ComponentTypeSize(int componentType) {
    switch (componentType) {
        case TINYGLTF_COMPONENT_TYPE_BYTE:           return 1;
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:  return 1;
        case TINYGLTF_COMPONENT_TYPE_SHORT:          return 2;
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: return 2;
        case TINYGLTF_COMPONENT_TYPE_INT:            return 4;
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:   return 4;
        case TINYGLTF_COMPONENT_TYPE_FLOAT:          return 4;
        case TINYGLTF_COMPONENT_TYPE_DOUBLE:         return 8;
        default:                                     return 0;
    }
}

[[nodiscard]] inline u32 TypeComponentCount(int type) {
    switch (type) {
        case TINYGLTF_TYPE_SCALAR: return 1;
        case TINYGLTF_TYPE_VEC2:   return 2;
        case TINYGLTF_TYPE_VEC3:   return 3;
        case TINYGLTF_TYPE_VEC4:   return 4;
        case TINYGLTF_TYPE_MAT2:   return 4;
        case TINYGLTF_TYPE_MAT3:   return 9;
        case TINYGLTF_TYPE_MAT4:   return 16;
        default:                   return 0;
    }
}

[[nodiscard]] inline cc::vec4f ReadVec4(const tinygltf::Accessor& accessor,
                                        const tinygltf::BufferView& view,
                                        const tinygltf::Buffer& buffer,
                                        size_t index) {
    const u32 compSize  = ComponentTypeSize(accessor.componentType);
    const u32 compCount = TypeComponentCount(accessor.type);
    const size_t stride = accessor.ByteStride(view) ? accessor.ByteStride(view)
                                                    : compSize * compCount;

    const size_t offset = accessor.byteOffset + view.byteOffset + index * stride;
    const unsigned char* data = buffer.data.data() + offset;

    cc::vec4f out{0.0f, 0.0f, 0.0f, 0.0f};

    if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT) {
        const float* v = reinterpret_cast<const float*>(data);
        for (u32 i = 0; i < compCount && i < 4; ++i) {
            out[i] = v[i];
        }
    }
    return out;
}

[[nodiscard]] inline cc::vec3f Vec3FromAccessor(const tinygltf::Accessor& accessor,
                                                const tinygltf::BufferView& view,
                                                const tinygltf::Buffer& buffer,
                                                size_t index) {
    const cc::vec4f v = ReadVec4(accessor, view, buffer, index);
    return cc::vec3f{v[0], v[1], v[2]};
}

} // anonymous namespace

ResourceLoader::ResourceLoader(Device* device) noexcept
    : device_(device) {}

scope<ResourceLoader> ResourceLoader::Create(Device* device) noexcept {
    return scope<ResourceLoader>(new ResourceLoader(device));
}

ref<Buffer> ResourceLoader::CreateVertexBuffer(const void* data, u64 size) const {
    return Buffer::Create(device_, BufferType::Vertex, size, BufferUsage::Static, data);
}

ref<Buffer> ResourceLoader::CreateIndexBuffer(const void* data, u64 size, IndexType indexType) const {
    (void)indexType;
    return Buffer::Create(device_, BufferType::Index, size, BufferUsage::Static, data);
}

ref<Texture2D> ResourceLoader::CreateTexture2DFromMemory(
    const void* data,
    u32 width,
    u32 height,
    TextureFormat format,
    bool /*srgb*/
) const {
    return Texture2D::Create(device_, width, height, format, data);
}

ref<Texture2D> ResourceLoader::CreateTexture2DFallback(TextureFormat format, bool /*srgb*/) const {
    const u32 w = 1;
    const u32 h = 1;
    u32 pixel = 0xffffffffu;
    return Texture2D::Create(device_, w, h, format, &pixel);
}

ref<Sampler> ResourceLoader::CreateDefaultSampler(bool /*srgb*/) const {
    SamplerConfig cfg = SamplerConfig::LinearRepeat();
    return Sampler::Create(device_, cfg);
}

MeshLoader::MeshLoader(
    ResourceLoader* resourceLoader,
    const std::filesystem::path& path,
    const MeshLoadOptions& options
) noexcept
    : resourceLoader_(resourceLoader)
    , path_(path)
    , options_(options) {}

MeshLoader::~MeshLoader() {
    if (worker_.joinable()) {
        worker_.join();
    }
}

scope<MeshLoader> MeshLoader::FromFile(
    ResourceLoader* resourceLoader,
    const std::filesystem::path& path,
    const MeshLoadOptions& options
) {
    if (resourceLoader == nullptr) {
        log::Critical("ResourceLoader is required to create MeshLoader");
        throw std::runtime_error("ResourceLoader is null");
    }

    auto loader = scope<MeshLoader>(new MeshLoader(resourceLoader, path, options));

    if (options.mode == LoadMode::Async) {
        loader->LoadAsync();
    } else {
        loader->LoadSync();
    }

    return loader;
}

void MeshLoader::Wait() {
    if (worker_.joinable()) {
        worker_.join();
    }
}

void MeshLoader::LoadSync() {
    DoLoad();
}

void MeshLoader::LoadAsync() {
    worker_ = std::thread([this]() {
        try {
            DoLoad();
        } catch (...) {
            hasError_.store(true, std::memory_order_release);
            errorMessage_ = "Unhandled exception during async load";
            ready_.store(true, std::memory_order_release);
        }
    });
}

void MeshLoader::DoLoad() {
    tinygltf::Model model;
    tinygltf::TinyGLTF loader;
    std::string err;
    std::string warn;

    const std::string pathStr = path_.string();
    const bool isGlb = path_.extension() == ".glb";

    bool ok = false;
    if (isGlb) {
        ok = loader.LoadBinaryFromFile(&model, &err, &warn, pathStr);
    } else {
        ok = loader.LoadASCIIFromFile(&model, &err, &warn, pathStr);
    }

    if (!warn.empty()) {
        log::Warn("tinygltf: {}", warn);
    }

    if (!ok || !err.empty()) {
        log::Error("Failed to load glTF '{}': {}", pathStr, err);
        hasError_.store(true, std::memory_order_release);
        errorMessage_ = err.empty() ? "Failed to load glTF file" : err;
        ready_.store(true, std::memory_order_release);
        return;
    }

    try {
        //NOTE: images metadata
        scene_.textures_.clear();
        scene_.textureObjects_.clear();
        scene_.samplerObjects_.clear();

        scene_.textures_.reserve(model.images.size());
        for (size_t i = 0; i < model.images.size(); ++i) {
            const tinygltf::Image& img = model.images[i];
            TextureInfo info{};
            info.name           = img.name;
            info.uri            = img.uri;
            info.width          = img.width  > 0 ? static_cast<u32>(img.width)  : 0u;
            info.height         = img.height > 0 ? static_cast<u32>(img.height) : 0u;
            info.componentCount = img.component > 0 ? static_cast<u32>(img.component) : 0u;
            info.imageIndex     = static_cast<u32>(i);
            scene_.textures_.push_back(std::move(info));
        }

        //NOTE: textures (GPU) + samplers
        const size_t textureCount = model.textures.size();
        std::vector<TextureRef> textureRefs(textureCount);

        scene_.textureObjects_.resize(textureCount);
        scene_.samplerObjects_.resize(textureCount);

        for (size_t ti = 0; ti < textureCount; ++ti) {
            const tinygltf::Texture& tex = model.textures[ti];
            if (tex.source < 0 || tex.source >= static_cast<int>(model.images.size())) {
                continue;
            }

            const tinygltf::Image& img = model.images[static_cast<size_t>(tex.source)];
            if (img.width <= 0 || img.height <= 0 || img.image.empty()) {
                continue;
            }

            bool srgb = true;
            TextureFormat fmt = ToTextureFormatRGBA8(srgb);
            ref<Texture2D> texture = resourceLoader_->CreateTexture2DFromMemory(
                img.image.data(),
                static_cast<u32>(img.width),
                static_cast<u32>(img.height),
                fmt,
                srgb
            );

            ref<Sampler> sampler;
            if (tex.sampler >= 0 && tex.sampler < static_cast<int>(model.samplers.size())) {
                const tinygltf::Sampler& s = model.samplers[static_cast<size_t>(tex.sampler)];
                SamplerConfig cfg = SamplerConfig::LinearRepeat();

                auto ToFilter = [](int f) {
                    if (f == TINYGLTF_TEXTURE_FILTER_NEAREST ||
                        f == TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_NEAREST ||
                        f == TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_LINEAR) {
                        return TextureFilter::Nearest;
                    }
                    return TextureFilter::Linear;
                };

                auto ToWrap = [](int w) {
                    switch (w) {
                        case TINYGLTF_TEXTURE_WRAP_CLAMP_TO_EDGE:   return TextureWrap::ClampToEdge;
                        case TINYGLTF_TEXTURE_WRAP_MIRRORED_REPEAT: return TextureWrap::MirrorRepeat;
                        case TINYGLTF_TEXTURE_WRAP_REPEAT:
                        default:                                     return TextureWrap::Repeat;
                    }
                };

                if (s.minFilter != -1) cfg.minFilter = ToFilter(s.minFilter);
                if (s.magFilter != -1) cfg.magFilter = ToFilter(s.magFilter);
                if (s.wrapS != -1)     cfg.wrapU    = ToWrap(s.wrapS);
                if (s.wrapT != -1)     cfg.wrapV    = ToWrap(s.wrapT);

                sampler = Sampler::Create(resourceLoader_->GetDevice(), cfg);
            } else {
                sampler = resourceLoader_->CreateDefaultSampler(srgb);
            }

            scene_.textureObjects_[ti] = texture;
            scene_.samplerObjects_[ti] = sampler;

            textureRefs[ti] = TextureRef{
                .texture = texture.get(),
                .sampler = sampler.get(),
                .srgb    = srgb
            };
        }

        //NOTE: materials
        scene_.materials_.reserve(model.materials.size());
        for (const auto& m : model.materials) {
            Material mat{};
            mat.name = m.name;

            if (m.values.contains("baseColorFactor")) {
                const auto& f = m.values.at("baseColorFactor").ColorFactor();
                mat.baseColorFactor = cc::vec4f{
                    static_cast<f32>(f[0]),
                    static_cast<f32>(f[1]),
                    static_cast<f32>(f[2]),
                    static_cast<f32>(f[3])
                };
            }

            if (m.values.contains("metallicFactor")) {
                mat.metallicFactor = static_cast<f32>(m.values.at("metallicFactor").Factor());
            }
            if (m.values.contains("roughnessFactor")) {
                mat.roughnessFactor = static_cast<f32>(m.values.at("roughnessFactor").Factor());
            }

            if (m.additionalValues.contains("emissiveFactor")) {
                const auto& f = m.additionalValues.at("emissiveFactor").ColorFactor();
                mat.emissiveFactor = cc::vec3f{
                    static_cast<f32>(f[0]),
                    static_cast<f32>(f[1]),
                    static_cast<f32>(f[2])
                };
            }

            if (m.values.contains("baseColorTexture")) {
                const int idx = m.values.at("baseColorTexture").TextureIndex();
                if (idx >= 0 && idx < static_cast<int>(textureRefs.size())) {
                    mat.baseColorTexture = textureRefs[static_cast<size_t>(idx)];
                }
            }
            if (m.values.contains("metallicRoughnessTexture")) {
                const int idx = m.values.at("metallicRoughnessTexture").TextureIndex();
                if (idx >= 0 && idx < static_cast<int>(textureRefs.size())) {
                    mat.metallicRoughnessTexture = textureRefs[static_cast<size_t>(idx)];
                }
            }
            if (m.additionalValues.contains("normalTexture")) {
                const int idx = m.additionalValues.at("normalTexture").TextureIndex();
                if (idx >= 0 && idx < static_cast<int>(textureRefs.size())) {
                    mat.normalTexture = textureRefs[static_cast<size_t>(idx)];
                    mat.normalTexture.srgb = false;
                }
            }
            if (m.additionalValues.contains("occlusionTexture")) {
                const int idx = m.additionalValues.at("occlusionTexture").TextureIndex();
                if (idx >= 0 && idx < static_cast<int>(textureRefs.size())) {
                    mat.occlusionTexture = textureRefs[static_cast<size_t>(idx)];
                    mat.occlusionTexture.srgb = false;
                }
            }
            if (m.additionalValues.contains("emissiveTexture")) {
                const int idx = m.additionalValues.at("emissiveTexture").TextureIndex();
                if (idx >= 0 && idx < static_cast<int>(textureRefs.size())) {
                    mat.emissiveTexture = textureRefs[static_cast<size_t>(idx)];
                }
            }

            mat.alphaMode   = ToAlphaMode(m.alphaMode);
            mat.alphaCutoff = static_cast<f32>(m.alphaCutoff);
            mat.doubleSided = m.doubleSided;

            scene_.materials_.push_back(std::move(mat));
        }

        //NOTE: meshes
        scene_.meshes_.reserve(model.meshes.size());
        for (const auto& gm : model.meshes) {
            Mesh mesh{};
            mesh.name = gm.name;

            for (const auto& prim : gm.primitives) {
                MeshPrimitive mp{};

                const auto posIt = prim.attributes.find("POSITION");
                if (posIt == prim.attributes.end()) {
                    log::Warn("Primitive without POSITION attribute skipped");
                    continue;
                }

                const int posAccessorIndex = posIt->second;
                const tinygltf::Accessor& posAccessor = model.accessors[static_cast<size_t>(posAccessorIndex)];
                const tinygltf::BufferView& posView = model.bufferViews[static_cast<size_t>(posAccessor.bufferView)];
                const tinygltf::Buffer& posBuffer = model.buffers[static_cast<size_t>(posView.buffer)];

                const u32 vertexCount = static_cast<u32>(posAccessor.count);
                const bool flipY = (options_.flags & MeshLoadFlag::FlipTexcoordY) != 0;

                struct Vertex {
                    cc::vec3f position;
                    cc::vec3f normal;
                    cc::vec4f tangent;
                    cc::vec2f texcoord0;
                    cc::vec4f color0;
                };

                std::vector<Vertex> vertices(vertexCount);

                // positions
                {
                    const u32 compSize  = ComponentTypeSize(posAccessor.componentType);
                    const u32 compCount = TypeComponentCount(posAccessor.type);
                    const size_t stride = posAccessor.ByteStride(posView)
                                          ? posAccessor.ByteStride(posView)
                                          : compSize * compCount;

                    for (u32 i = 0; i < vertexCount; ++i) {
                        const size_t offset = posAccessor.byteOffset + posView.byteOffset + i * stride;
                        const float* v = reinterpret_cast<const float*>(posBuffer.data.data() + offset);
                        vertices[i].position = cc::vec3f{v[0], v[1], v[2]};
                    }
                }

                // normals
                if (auto it = prim.attributes.find("NORMAL"); it != prim.attributes.end()) {
                    const tinygltf::Accessor& acc = model.accessors[static_cast<size_t>(it->second)];
                    const tinygltf::BufferView& view = model.bufferViews[static_cast<size_t>(acc.bufferView)];
                    const tinygltf::Buffer& buf = model.buffers[static_cast<size_t>(view.buffer)];

                    const u32 compSize  = ComponentTypeSize(acc.componentType);
                    const u32 compCount = TypeComponentCount(acc.type);
                    const size_t stride = acc.ByteStride(view)
                                          ? acc.ByteStride(view)
                                          : compSize * compCount;

                    for (u32 i = 0; i < vertexCount; ++i) {
                        const size_t offset = acc.byteOffset + view.byteOffset + i * stride;
                        const float* v = reinterpret_cast<const float*>(buf.data.data() + offset);
                        vertices[i].normal = cc::vec3f{v[0], v[1], v[2]};
                    }
                } else {
                    for (u32 i = 0; i < vertexCount; ++i) {
                        vertices[i].normal = cc::vec3f{0.0f, 1.0f, 0.0f};
                    }
                }

                // tangents
                if (auto it = prim.attributes.find("TANGENT"); it != prim.attributes.end()) {
                    const tinygltf::Accessor& acc = model.accessors[static_cast<size_t>(it->second)];
                    const tinygltf::BufferView& view = model.bufferViews[static_cast<size_t>(acc.bufferView)];
                    const tinygltf::Buffer& buf = model.buffers[static_cast<size_t>(view.buffer)];

                    const u32 compSize  = ComponentTypeSize(acc.componentType);
                    const u32 compCount = TypeComponentCount(acc.type);
                    const size_t stride = acc.ByteStride(view)
                                          ? acc.ByteStride(view)
                                          : compSize * compCount;

                    for (u32 i = 0; i < vertexCount; ++i) {
                        const size_t offset = acc.byteOffset + view.byteOffset + i * stride;
                        const float* v = reinterpret_cast<const float*>(buf.data.data() + offset);
                        vertices[i].tangent = cc::vec4f{v[0], v[1], v[2], v[3]};
                    }
                } else {
                    for (u32 i = 0; i < vertexCount; ++i) {
                        vertices[i].tangent = cc::vec4f{1.0f, 0.0f, 0.0f, 1.0f};
                    }
                }

                // texcoord0
                if (auto it = prim.attributes.find("TEXCOORD_0"); it != prim.attributes.end()) {
                    const tinygltf::Accessor& acc = model.accessors[static_cast<size_t>(it->second)];
                    const tinygltf::BufferView& view = model.bufferViews[static_cast<size_t>(acc.bufferView)];
                    const tinygltf::Buffer& buf = model.buffers[static_cast<size_t>(view.buffer)];

                    const u32 compSize  = ComponentTypeSize(acc.componentType);
                    const u32 compCount = TypeComponentCount(acc.type);
                    const size_t stride = acc.ByteStride(view)
                                          ? acc.ByteStride(view)
                                          : compSize * compCount;

                    for (u32 i = 0; i < vertexCount; ++i) {
                        const size_t offset = acc.byteOffset + view.byteOffset + i * stride;
                        const float* v = reinterpret_cast<const float*>(buf.data.data() + offset);
                        f32 u = v[0];
                        f32 vv = v[1];
                        if (flipY) vv = 1.0f - vv;
                        vertices[i].texcoord0 = cc::vec2f{u, vv};
                    }
                } else {
                    for (u32 i = 0; i < vertexCount; ++i) {
                        vertices[i].texcoord0 = cc::vec2f{0.0f, 0.0f};
                    }
                }

                // color0
                if (auto it = prim.attributes.find("COLOR_0"); it != prim.attributes.end()) {
                    const tinygltf::Accessor& acc = model.accessors[static_cast<size_t>(it->second)];
                    const tinygltf::BufferView& view = model.bufferViews[static_cast<size_t>(acc.bufferView)];
                    const tinygltf::Buffer& buf = model.buffers[static_cast<size_t>(view.buffer)];

                    const u32 compSize  = ComponentTypeSize(acc.componentType);
                    const u32 compCount = TypeComponentCount(acc.type);
                    const size_t stride = acc.ByteStride(view)
                                          ? acc.ByteStride(view)
                                          : compSize * compCount;

                    for (u32 i = 0; i < vertexCount; ++i) {
                        const size_t offset = acc.byteOffset + view.byteOffset + i * stride;
                        cc::vec4f c{1.0f, 1.0f, 1.0f, 1.0f};

                        if (acc.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT) {
                            const float* v = reinterpret_cast<const float*>(buf.data.data() + offset);
                            for (u32 j = 0; j < compCount && j < 4; ++j) c[j] = v[j];
                        } else if (acc.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) {
                            const unsigned char* v = buf.data.data() + offset;
                            for (u32 j = 0; j < compCount && j < 4; ++j)
                                c[j] = static_cast<f32>(v[j]) / 255.0f;
                        }

                        vertices[i].color0 = c;
                    }
                } else {
                    for (u32 i = 0; i < vertexCount; ++i) {
                        vertices[i].color0 = cc::vec4f{1.0f, 1.0f, 1.0f, 1.0f};
                    }
                }

                const u64 vbSize = static_cast<u64>(vertices.size() * sizeof(Vertex));
                mp.vertexBuffer = resourceLoader_->CreateVertexBuffer(vertices.data(), vbSize);

                // indices
                if (prim.indices >= 0) {
                    const tinygltf::Accessor& idxAcc = model.accessors[static_cast<size_t>(prim.indices)];
                    const tinygltf::BufferView& idxView = model.bufferViews[static_cast<size_t>(idxAcc.bufferView)];
                    const tinygltf::Buffer& idxBuf = model.buffers[static_cast<size_t>(idxView.buffer)];

                    mp.indexType = ToIndexType(idxAcc.componentType);
                    const u32 compSize = ComponentTypeSize(idxAcc.componentType);
                    const u32 count = static_cast<u32>(idxAcc.count);
                    const u64 ibSize = static_cast<u64>(count) * compSize;

                    const u8* src = idxBuf.data.data() + idxView.byteOffset + idxAcc.byteOffset;

                    std::vector<u8> ibData(ibSize);
                    std::memcpy(ibData.data(), src, ibSize);

                    mp.indexBuffer = resourceLoader_->CreateIndexBuffer(ibData.data(), ibSize, mp.indexType);
                    mp.indexCount  = count;
                    mp.firstIndex  = 0;
                } else {
                    mp.indexType  = IndexType::U32;
                    mp.indexCount = vertexCount;
                    std::vector<u32> indices(vertexCount);
                    for (u32 i = 0; i < vertexCount; ++i) indices[i] = i;
                    const u64 ibSize = static_cast<u64>(indices.size() * sizeof(u32));
                    mp.indexBuffer = resourceLoader_->CreateIndexBuffer(indices.data(), ibSize, mp.indexType);
                    mp.firstIndex  = 0;
                }

                if (prim.material >= 0 && prim.material < static_cast<int>(scene_.materials_.size())) {
                    mp.material = static_cast<MaterialID>(prim.material);
                }

                auto layoutBuilder = VertexLayout::Create()
                    .Binding(0, sizeof(Vertex), VertexInputRate::PerVertex)
                    .Attribute(0, 0, VertexFormat::Float3, offsetof(Vertex, position))
                    .Attribute(1, 0, VertexFormat::Float3, offsetof(Vertex, normal))
                    .Attribute(2, 0, VertexFormat::Float4, offsetof(Vertex, tangent))
                    .Attribute(3, 0, VertexFormat::Float2, offsetof(Vertex, texcoord0))
                    .Attribute(4, 0, VertexFormat::Float4, offsetof(Vertex, color0));

                mp.vertexLayout = layoutBuilder.Build().release();

                mesh.primitives.push_back(std::move(mp));
            }

            scene_.meshes_.push_back(std::move(mesh));
        }

        //NOTE: skins
        scene_.skins_.reserve(model.skins.size());
        for (const auto& s : model.skins) {
            Skin skin{};
            skin.name = s.name;

            if (s.skeleton >= 0) skin.skeletonRoot = static_cast<NodeID>(s.skeleton);

            for (int j : s.joints) {
                if (j >= 0) skin.joints.push_back(static_cast<NodeID>(j));
            }

            if (s.inverseBindMatrices >= 0) {
                const tinygltf::Accessor& acc = model.accessors[static_cast<size_t>(s.inverseBindMatrices)];
                const tinygltf::BufferView& view = model.bufferViews[static_cast<size_t>(acc.bufferView)];
                const tinygltf::Buffer& buf = model.buffers[static_cast<size_t>(view.buffer)];

                const size_t count = acc.count;
                skin.inverseBindMatrices.resize(count);

                const size_t stride = acc.ByteStride(view)
                                      ? acc.ByteStride(view)
                                      : ComponentTypeSize(acc.componentType) * 16;

                for (size_t i = 0; i < count; ++i) {
                    const size_t offset = acc.byteOffset + view.byteOffset + i * stride;
                    const float* m = reinterpret_cast<const float*>(buf.data.data() + offset);

                    cc::mat4f mat{};
                    for (int r = 0; r < 4; ++r)
                        for (int c = 0; c < 4; ++c)
                            mat(r, c) = m[r * 4 + c];

                    skin.inverseBindMatrices[i] = mat;
                }
            }

            scene_.skins_.push_back(std::move(skin));
        }

        //NOTE: nodes
        scene_.nodes_.reserve(model.nodes.size());
        for (size_t i = 0; i < model.nodes.size(); ++i) {
            const tinygltf::Node& gn = model.nodes[i];

            Node node{};
            node.name   = gn.name;
            node.parent = INVALID_NODE_ID;

            if (!gn.matrix.empty()) {
                node.hasLocalMatrix = true;
                cc::mat4f m{};
                for (int r = 0; r < 4; ++r)
                    for (int c = 0; c < 4; ++c)
                        m(r, c) = static_cast<f32>(gn.matrix[r * 4 + c]);
                node.localMatrix = m;
            } else {
                if (!gn.translation.empty()) {
                    node.local.translation = cc::vec3f{
                        static_cast<f32>(gn.translation[0]),
                        static_cast<f32>(gn.translation[1]),
                        static_cast<f32>(gn.translation[2])
                    };
                }
                if (!gn.rotation.empty()) {
                    node.local.rotation = cc::quatf{
                        static_cast<f32>(gn.rotation[0]),
                        static_cast<f32>(gn.rotation[1]),
                        static_cast<f32>(gn.rotation[2]),
                        static_cast<f32>(gn.rotation[3])
                    };
                }
                if (!gn.scale.empty()) {
                    node.local.scale = cc::vec3f{
                        static_cast<f32>(gn.scale[0]),
                        static_cast<f32>(gn.scale[1]),
                        static_cast<f32>(gn.scale[2])
                    };
                }
            }

            if (gn.mesh >= 0) {
                node.type = NodeType::Mesh;
                node.mesh = static_cast<MeshID>(gn.mesh);
            } else {
                node.type = NodeType::Empty;
            }

            if (gn.camera >= 0) node.camera = static_cast<CameraID>(gn.camera);
            if (gn.skin   >= 0) node.skin   = static_cast<SkinID>(gn.skin);

            scene_.nodes_.push_back(std::move(node));
        }

        for (size_t i = 0; i < model.nodes.size(); ++i) {
            const tinygltf::Node& gn = model.nodes[i];
            Node& node = scene_.nodes_[i];
            for (int child : gn.children) {
                if (child >= 0) {
                    node.children.push_back(static_cast<NodeID>(child));
                    scene_.nodes_[static_cast<size_t>(child)].parent = static_cast<NodeID>(i);
                }
            }
        }

        //NOTE: cameras
        scene_.cameras_.reserve(model.cameras.size());
        for (size_t i = 0; i < model.cameras.size(); ++i) {
            const tinygltf::Camera& gc = model.cameras[i];

            SceneCamera sc{};
            sc.name = gc.name;

            CameraProjection proj{};
            if (gc.type == "perspective") {
                proj.type   = CameraProjectionType::Perspective;
                proj.yfov   = static_cast<f32>(gc.perspective.yfov);
                proj.znear  = static_cast<f32>(gc.perspective.znear);
                proj.zfar   = static_cast<f32>(gc.perspective.zfar <= 0.0 ? 1000.0 : gc.perspective.zfar);
                proj.aspect = static_cast<f32>(gc.perspective.aspectRatio <= 0.0 ? 1.0 : gc.perspective.aspectRatio);
            } else if (gc.type == "orthographic") {
                proj.type   = CameraProjectionType::Orthographic;
                proj.xmag   = static_cast<f32>(gc.orthographic.xmag);
                proj.ymag   = static_cast<f32>(gc.orthographic.ymag);
                proj.znear  = static_cast<f32>(gc.orthographic.znear);
                proj.zfar   = static_cast<f32>(gc.orthographic.zfar);
            }

            Camera::Desc desc{};
            desc.name       = sc.name;
            desc.projection = proj;
            desc.position   = cc::vec3f{0.0f, 0.0f, 0.0f};
            desc.target     = cc::vec3f{0.0f, 0.0f, -1.0f};
            desc.up         = cc::vec3f{0.0f, 1.0f, 0.0f};

            sc.camera = Camera(desc);

            sc.node = INVALID_NODE_ID;
            for (size_t ni = 0; ni < scene_.nodes_.size(); ++ni) {
                const Node& n = scene_.nodes_[ni];
                if (n.camera && *n.camera == static_cast<CameraID>(i)) {
                    sc.node = static_cast<NodeID>(ni);
                    break;
                }
            }

            scene_.cameras_.push_back(std::move(sc));
        }

        //NOTE: lights (KHR_lights_punctual)
        const auto itExtLights = model.extensions.find("KHR_lights_punctual");
        if (itExtLights != model.extensions.end()) {
            const tinygltf::Value& lightsExt = itExtLights->second;
            const auto& lightsArray = lightsExt.Get("lights").Get<tinygltf::Value::Array>();

            scene_.lights_.reserve(lightsArray.size());

            for (size_t li = 0; li < lightsArray.size(); ++li) {
                const tinygltf::Value& l = lightsArray[li];
                Light light{};

                if (l.Has("name")) {
                    light.name = l.Get("name").Get<std::string>();
                }

                if (l.Has("type")) {
                    light.type = ToLightType(l.Get("type").Get<std::string>());
                }

                if (l.Has("color")) {
                    const auto& arr = l.Get("color").Get<tinygltf::Value::Array>();
                    light.color = cc::vec3f{
                        static_cast<f32>(arr[0].Get<double>()),
                        static_cast<f32>(arr[1].Get<double>()),
                        static_cast<f32>(arr[2].Get<double>())
                    };
                }

                if (l.Has("intensity")) {
                    light.intensity = static_cast<f32>(l.Get("intensity").Get<double>());
                }

                if (l.Has("range")) {
                    light.range = static_cast<f32>(l.Get("range").Get<double>());
                }

                if (light.type == LightType::Spot && l.Has("spot")) {
                    const auto& spot = l.Get("spot");
                    if (spot.Has("innerConeAngle")) {
                        light.innerConeAngle = static_cast<f32>(spot.Get("innerConeAngle").Get<double>());
                    }
                    if (spot.Has("outerConeAngle")) {
                        light.outerConeAngle = static_cast<f32>(spot.Get("outerConeAngle").Get<double>());
                    }
                }

                light.node = INVALID_NODE_ID;
                for (size_t ni = 0; ni < model.nodes.size(); ++ni) {
                    const tinygltf::Node& n = model.nodes[ni];
                    const auto itn = n.extensions.find("KHR_lights_punctual");
                    if (itn != n.extensions.end()) {
                        int idx = itn->second.Get("light").Get<int>();
                        if (idx == static_cast<int>(li)) {
                            light.node = static_cast<NodeID>(ni);
                            scene_.nodes_[ni].light = static_cast<LightID>(scene_.lights_.size());
                            scene_.nodes_[ni].type  = NodeType::Light;
                            break;
                        }
                    }
                }

                scene_.lights_.push_back(std::move(light));
            }
        }

        //NOTE: animations
        scene_.animations_.reserve(model.animations.size());
        for (const auto& ga : model.animations) {
            Animation anim{};
            anim.name = ga.name;

            anim.samplers.reserve(ga.samplers.size());
            for (const auto& s : ga.samplers) {
                AnimationSampler sampler{};

                const tinygltf::Accessor& inAcc = model.accessors[static_cast<size_t>(s.input)];
                const tinygltf::BufferView& inView = model.bufferViews[static_cast<size_t>(inAcc.bufferView)];
                const tinygltf::Buffer& inBuf = model.buffers[static_cast<size_t>(inView.buffer)];

                const size_t inCount = inAcc.count;
                sampler.input.resize(inCount);

                const size_t inStride = inAcc.ByteStride(inView)
                                        ? inAcc.ByteStride(inView)
                                        : ComponentTypeSize(inAcc.componentType) * TypeComponentCount(inAcc.type);

                for (size_t i = 0; i < inCount; ++i) {
                    const size_t offset = inAcc.byteOffset + inView.byteOffset + i * inStride;
                    const float* t = reinterpret_cast<const float*>(inBuf.data.data() + offset);
                    sampler.input[i] = *t;
                    if (sampler.input[i] > anim.durationSeconds) {
                        anim.durationSeconds = sampler.input[i];
                    }
                }

                const tinygltf::Accessor& outAcc = model.accessors[static_cast<size_t>(s.output)];
                const tinygltf::BufferView& outView = model.bufferViews[static_cast<size_t>(outAcc.bufferView)];
                const tinygltf::Buffer& outBuf = model.buffers[static_cast<size_t>(outView.buffer)];

                const size_t outCount = outAcc.count;
                sampler.output.resize(outCount);

                const size_t outStride = outAcc.ByteStride(outView)
                                         ? outAcc.ByteStride(outView)
                                         : ComponentTypeSize(outAcc.componentType) * TypeComponentCount(outAcc.type);

                for (size_t i = 0; i < outCount; ++i) {
                    const size_t offset = outAcc.byteOffset + outView.byteOffset + i * outStride;
                    cc::vec4f v{0.0f, 0.0f, 0.0f, 0.0f};

                    if (outAcc.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT) {
                        const float* p = reinterpret_cast<const float*>(outBuf.data.data() + offset);
                        const u32 compCount = TypeComponentCount(outAcc.type);
                        for (u32 c = 0; c < compCount && c < 4; ++c) {
                            v[c] = p[c];
                        }
                    }

                    sampler.output[i] = v;
                }

                sampler.interpolation = ToInterpolationType(s.interpolation);

                anim.samplers.push_back(std::move(sampler));
            }

            anim.channels.reserve(ga.channels.size());
            for (const auto& c : ga.channels) {
                if (c.target_node < 0 || c.sampler < 0) continue;

                AnimationChannel ch{};
                ch.targetNode = static_cast<NodeID>(c.target_node);

                if      (c.target_path == "translation") ch.path = AnimationPath::Translation;
                else if (c.target_path == "rotation")    ch.path = AnimationPath::Rotation;
                else if (c.target_path == "scale")       ch.path = AnimationPath::Scale;
                else if (c.target_path == "weights")     ch.path = AnimationPath::Weights;

                ch.samplerIndex = static_cast<u32>(c.sampler);
                anim.channels.push_back(std::move(ch));
            }

            scene_.animations_.push_back(std::move(anim));
        }

        //NOTE: roots
        if (!model.scenes.empty()) {
            int sceneIndex = model.defaultScene >= 0 ? model.defaultScene : 0;
            const tinygltf::Scene& gs = model.scenes[static_cast<size_t>(sceneIndex)];
            for (int n : gs.nodes) {
                if (n >= 0) scene_.roots_.push_back(static_cast<NodeID>(n));
            }
        } else {
            for (size_t i = 0; i < scene_.nodes_.size(); ++i) {
                if (scene_.nodes_[i].parent == INVALID_NODE_ID) {
                    scene_.roots_.push_back(static_cast<NodeID>(i));
                }
            }
        }

    } catch (const std::exception& e) {
        log::Error("MeshLoader::DoLoad failed: {}", e.what());
        hasError_.store(true, std::memory_order_release);
        errorMessage_ = e.what();
        ready_.store(true, std::memory_order_release);
        return;
    }

    ready_.store(true, std::memory_order_release);
}

} // namespace cc::gfx
