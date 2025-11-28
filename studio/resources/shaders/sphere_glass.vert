#version 460 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec3 aColor;

layout(std140, binding = 0) uniform PBRUniforms {
    mat4 model;
    mat4 view;
    mat4 projection;
    mat4 lightSpaceMatrix;
    vec3 lightDir;
    float padding1;
    vec3 viewPos;
    float padding2;
    vec3 lightColor;
    float metallic;
    vec3 albedo;
    float roughness;
    float ao;
    float isGlass;   // ignored here, sphere is always glass
    vec2 padding3;
};

out vec3 vWorldPos;
out vec3 vWorldNormal;
out vec3 vColor;
out vec4 vLightSpacePos;

void main() {
    vec4 worldPos = model * vec4(aPosition, 1.0);
    vWorldPos = worldPos.xyz;

    vWorldNormal = mat3(transpose(inverse(model))) * aNormal;

    vColor = aColor;
    vLightSpacePos = lightSpaceMatrix * worldPos;

    gl_Position = projection * view * worldPos;
}
