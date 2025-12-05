#version 460 core

layout(location = 0) in vec2 vCoord;

layout(std140, binding = 0) uniform UBO {
    float iTime;
    float iTimeDelta;
    float iFrame;
    float pad0;

    vec3 iResolution;
    float pad1;

    vec4 iMouse;

    vec4 camPos;
    vec4 camDir;
    vec4 camUp;
    vec4 camRight;

    mat4 view;
    mat4 projection;
};

layout(std140, binding = 1) uniform SimParams {
    vec2 uCenter;
    float uRadius;
    float uStrength;
    float _pad0;
};

layout(binding = 2) uniform sampler2D uSimTex;

layout(location = 0) out vec4 FragColor;

const float PI = 3.14159265358979323846;

void main() {
    vec4 info = texture(uSimTex, vCoord);

    float dist = length(uCenter * 0.5 + 0.5 - vCoord);
    float drop = max(0.0, 1.0 - dist / uRadius);
    drop = 0.5 - cos(drop * PI) * 0.5;

    info.r += drop * uStrength;

    FragColor = info;
}
