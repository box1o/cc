#version 450 core

layout(std140, binding = 0) uniform UBO {
    mat4 model;
    mat4 view;
    mat4 projection;
    vec4 cameraPos;
    vec4 utils;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;

layout(location = 0) out vec3 vNormal;
layout(location = 1) out vec2 vUV;
layout(location = 2) out vec3 vWorldPos;

void main() {
    vec4 worldPos = ubo.model * vec4(inPosition, 1.0);
    vWorldPos = worldPos.xyz;
    vNormal = mat3(ubo.model) * inNormal;
    vUV = inUV;

    gl_Position = ubo.projection * ubo.view * worldPos;
}
