#version 460 core

layout(location = 0) in vec3 aPosition;
layout(location = 2) in vec2 aUv;

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

layout(location = 0) out vec2 vCoord;

void main() {
    vec2 pos = aPosition.xy;
    vCoord = pos * 0.5 + 0.5;
    gl_Position = vec4(pos, 0.0, 1.0);
}
