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
    vec2 uDelta;
    vec2 _pad0;
};

layout(binding = 2) uniform sampler2D uSimTex;

layout(location = 0) out vec4 FragColor;

void main() {
    vec4 info = texture(uSimTex, vCoord);

    vec2 dx = vec2(uDelta.x, 0.0);
    vec2 dy = vec2(0.0, uDelta.y);

    float average = (
        texture(uSimTex, vCoord - dx).r +
            texture(uSimTex, vCoord - dy).r +
            texture(uSimTex, vCoord + dx).r +
            texture(uSimTex, vCoord + dy).r
        ) * 0.25;

    info.g += (average - info.r) * 2.0;
    info.g *= 0.995;
    info.r += info.g;

    FragColor = info;
}
