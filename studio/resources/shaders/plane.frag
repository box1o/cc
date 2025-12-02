#version 450 core

layout(std140, binding = 0) uniform UBO {
    mat4 model;
    mat4 view;
    mat4 projection;
    vec4 cameraPos;
    vec4 utils;
} ubo;

layout(location = 0) in vec3 vNormal;
layout(location = 1) in vec2 vUV;
layout(location = 2) in vec3 vWorldPos;

layout(location = 0) out vec4 outColor;

void main() {
    vec3 N = normalize(vNormal);
    vec3 L = normalize(vec3(0.5, 1.0, 0.3));
    vec3 V = normalize(ubo.cameraPos.xyz - vWorldPos);

    float NdotL = max(dot(N, L), 0.0);
    float diffuse = NdotL;

    vec3 baseColor = vec3(vUV, 1.0); // simple UV-based color
    vec3 color = baseColor * (0.2 + 0.8 * diffuse);

    outColor = vec4(color, 1.0);
}
