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

    float centerHeight = info.r;

    vec3 dx = vec3(
            uDelta.x,
            texture(uSimTex, vec2(vCoord.x + uDelta.x, vCoord.y)).r - centerHeight,
            0.0
        );

    vec3 dy = vec3(
            0.0,
            texture(uSimTex, vec2(vCoord.x, vCoord.y + uDelta.y)).r - centerHeight,
            uDelta.y
        );

    vec3 n = normalize(cross(dy, dx));
    info.b = n.x;
    info.a = n.z;

    FragColor = info;
}
