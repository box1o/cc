#version 460 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec3 aColor; // unused here, but keeps same layout

layout(std140, binding = 0) uniform WaterUniforms {
    mat4 model;
    mat4 view;
    mat4 projection;
    vec3 viewPos;
    float time;
    vec3 waterColor;
    float waveStrength;
};

out vec3 vWorldPos;
out vec3 vWorldNormal;
out vec3 vViewPos;
out vec2 vUV;

void main() {
    vec4 worldPos = model * vec4(aPosition, 1.0);
    vWorldPos = worldPos.xyz;
    vViewPos  = viewPos;

    // Base normal
    vWorldNormal = mat3(transpose(inverse(model))) * aNormal;

    // Simple UV from world XZ
    vUV = worldPos.xz * 0.1;

    gl_Position = projection * view * worldPos;
}
