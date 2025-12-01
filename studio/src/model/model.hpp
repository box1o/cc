#pragma once
#include "camera/camera.hpp"
#include "common/functions.hpp"
#include "pipeline/vertex_layout.hpp"
#include <cc/core/core.hpp>
#include <cc/math/math.hpp>
#include <cc/gfx/types.hpp>
#include <string>
#include <string_view>
#include <vector>
#include <optional>
#include <limits>

namespace cc::gfx {

class Buffer;
class Texture2D;
class Sampler;

//NOTE: Handle/ID types
using MeshID      = u32;
using NodeID      = u32;
using CameraID    = u32;
using LightID     = u32;
using MaterialID  = u32;
using SkinID      = u32;
using AnimationID = u32;

constexpr inline MeshID      INVALID_MESH_ID      = std::numeric_limits<MeshID>::max();
constexpr inline NodeID      INVALID_NODE_ID      = std::numeric_limits<NodeID>::max();
constexpr inline CameraID    INVALID_CAMERA_ID    = std::numeric_limits<CameraID>::max();
constexpr inline LightID     INVALID_LIGHT_ID     = std::numeric_limits<LightID>::max();
constexpr inline MaterialID  INVALID_MATERIAL_ID  = std::numeric_limits<MaterialID>::max();
constexpr inline SkinID      INVALID_SKIN_ID      = std::numeric_limits<SkinID>::max();
constexpr inline AnimationID INVALID_ANIMATION_ID = std::numeric_limits<AnimationID>::max();

//NOTE: Texture reference used by materials (GPU resource)
struct TextureRef {
    Texture2D* texture{nullptr};
    Sampler*   sampler{nullptr};
    bool       srgb{false};
};

//NOTE: Source texture metadata (from glTF images)
struct TextureInfo {
    std::string name;
    std::string uri;
    u32         width{0};
    u32         height{0};
    u32         componentCount{0};
    u32         imageIndex{0};
};

//NOTE: Material model (PBR metallic-roughness)
enum class AlphaMode : u8 {
    Opaque,
    Mask,
    Blend
};

struct Material {
    std::string name;

    cc::vec4f baseColorFactor{1.0f, 1.0f, 1.0f, 1.0f};
    f32 metallicFactor{1.0f};
    f32 roughnessFactor{1.0f};
    cc::vec3f emissiveFactor{0.0f, 0.0f, 0.0f};

    TextureRef baseColorTexture;
    TextureRef metallicRoughnessTexture;
    TextureRef normalTexture;
    TextureRef occlusionTexture;
    TextureRef emissiveTexture;

    AlphaMode alphaMode{AlphaMode::Opaque};
    f32       alphaCutoff{0.5f};
    bool      doubleSided{false};
};

//NOTE: Mesh primitive / submesh
struct MeshPrimitive {
    ref<Buffer> vertexBuffer;
    ref<Buffer> indexBuffer;

    VertexLayout* vertexLayout{nullptr};

    IndexType indexType{IndexType::U32};
    u32       indexCount{0};
    u32       firstIndex{0};
    i32       vertexOffset{0};

    std::optional<MaterialID> material;
};

//NOTE: Mesh (collection of primitives)
struct Mesh {
    std::string name;
    std::vector<MeshPrimitive> primitives;
};

//NOTE: Skinning data
struct Skin {
    std::string name;
    std::vector<NodeID> joints;
    std::vector<cc::mat4f> inverseBindMatrices;
    std::optional<NodeID> skeletonRoot;
};

//NOTE: Node transform
struct NodeTransform {
    cc::vec3f translation{0.0f, 0.0f, 0.0f};
    cc::quatf rotation{};
    cc::vec3f scale{1.0f, 1.0f, 1.0f};
};

enum class NodeType : u8 {
    Empty,
    Mesh,
    Camera,
    Light
};

struct Node {
    std::string name;

    NodeID parent{INVALID_NODE_ID};
    std::vector<NodeID> children;

    NodeTransform local;
    cc::mat4f     localMatrix{cc::mat4f::identity()};
    bool          hasLocalMatrix{false};

    NodeType type{NodeType::Empty};

    std::optional<MeshID>   mesh;
    std::optional<CameraID> camera;
    std::optional<LightID>  light;
    std::optional<SkinID>   skin;
};

//NOTE: Scene camera (Camera + node binding)
struct SceneCamera {
    std::string name;
    Camera      camera;
    NodeID      node{INVALID_NODE_ID};
};

//NOTE: Lights (KHR_lights_punctual)
enum class LightType : u8 {
    Directional,
    Point,
    Spot
};

struct Light {
    std::string name;
    LightType   type{LightType::Point};

    cc::vec3f color{1.0f, 1.0f, 1.0f};
    f32       intensity{1.0f};
    std::optional<f32> range;

    f32 innerConeAngle{0.0f};
    f32 outerConeAngle{cc::radians(45.0f)};

    NodeID node{INVALID_NODE_ID};
};

//NOTE: Animation representation (node transforms)
enum class AnimationPath : u8 {
    Translation,
    Rotation,
    Scale,
    Weights
};

enum class InterpolationType : u8 {
    Linear,
    Step,
    CubicSpline
};

struct AnimationSampler {
    std::vector<f32>       input;
    std::vector<cc::vec4f> output;
    InterpolationType      interpolation{InterpolationType::Linear};
};

struct AnimationChannel {
    NodeID        targetNode{INVALID_NODE_ID};
    AnimationPath path{AnimationPath::Translation};
    u32           samplerIndex{0};
};

struct Animation {
    std::string name;
    std::vector<AnimationSampler> samplers;
    std::vector<AnimationChannel> channels;
    f32 durationSeconds{0.0f};
};

//NOTE: Scene: all loaded entities
class Scene {
public:
    Scene() = default;

    [[nodiscard]] const std::vector<Node>&        GetNodes() const noexcept { return nodes_; }
    [[nodiscard]] const std::vector<Mesh>&        GetMeshes() const noexcept { return meshes_; }
    [[nodiscard]] const std::vector<Material>&    GetMaterials() const noexcept { return materials_; }
    [[nodiscard]] const std::vector<SceneCamera>& GetCameras() const noexcept { return cameras_; }
    [[nodiscard]] const std::vector<Light>&       GetLights() const noexcept { return lights_; }
    [[nodiscard]] const std::vector<Skin>&        GetSkins() const noexcept { return skins_; }
    [[nodiscard]] const std::vector<Animation>&   GetAnimations() const noexcept { return animations_; }
    [[nodiscard]] const std::vector<TextureInfo>& GetTextureInfos() const noexcept { return textures_; }

    //NOTE: optional access to GPU resources if needed
    [[nodiscard]] const std::vector<ref<Texture2D>>& GetTextureObjects() const noexcept { return textureObjects_; }
    [[nodiscard]] const std::vector<ref<Sampler>>&   GetSamplerObjects() const noexcept { return samplerObjects_; }

    [[nodiscard]] const Mesh* GetMesh(MeshID id) const noexcept {
        return (id < meshes_.size()) ? &meshes_[id] : nullptr;
    }

    [[nodiscard]] const Node* GetNode(NodeID id) const noexcept {
        return (id < nodes_.size()) ? &nodes_[id] : nullptr;
    }

    [[nodiscard]] const SceneCamera* GetCamera(CameraID id) const noexcept {
        return (id < cameras_.size()) ? &cameras_[id] : nullptr;
    }

    [[nodiscard]] const Light* GetLight(LightID id) const noexcept {
        return (id < lights_.size()) ? &lights_[id] : nullptr;
    }

    [[nodiscard]] const Material* GetMaterial(MaterialID id) const noexcept {
        return (id < materials_.size()) ? &materials_[id] : nullptr;
    }

    [[nodiscard]] const Skin* GetSkin(SkinID id) const noexcept {
        return (id < skins_.size()) ? &skins_[id] : nullptr;
    }

    [[nodiscard]] const std::vector<NodeID>& GetRootNodes() const noexcept { return roots_; }

private:
    std::vector<Node>        nodes_;
    std::vector<Mesh>        meshes_;
    std::vector<Material>    materials_;
    std::vector<SceneCamera> cameras_;
    std::vector<Light>       lights_;
    std::vector<Skin>        skins_;
    std::vector<Animation>   animations_;
    std::vector<NodeID>      roots_;
    std::vector<TextureInfo> textures_;

    //NOTE: own textures/samplers so TextureRef pointers stay valid
    std::vector<ref<Texture2D>> textureObjects_;
    std::vector<ref<Sampler>>   samplerObjects_;

    friend class MeshLoader;
};

} // namespace cc::gfx
