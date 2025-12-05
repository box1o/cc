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

layout(location = 0) out vec2 vUv;
layout(location = 1) out vec3 vRayOrigin;
layout(location = 2) out vec3 vRayDir;

void main() {
    vUv = aUv;

    vec2 ndc = aPosition.xy;
    mat4 invVP = inverse(projection * view);

    vec4 worldNear = invVP * vec4(ndc, 0.0, 1.0);
    vec4 worldFar = invVP * vec4(ndc, 1.0, 1.0);
    worldNear /= worldNear.w;
    worldFar /= worldFar.w;

    vec3 origin = camPos.xyz;
    vec3 dir = normalize(worldFar.xyz - origin);

    vRayOrigin = origin;
    vRayDir = dir;

    gl_Position = vec4(aPosition.xy, 0.0, 1.0);
}
