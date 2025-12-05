#version 460 core

layout(location = 0) in vec2 vUv;
layout(location = 1) in vec3 vRayOrigin;
layout(location = 2) in vec3 vRayDir;

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

layout(binding = 2) uniform sampler2D uSimTex;

layout(location = 0) out vec4 FragColor;

const float IOR_AIR = 1.0;
const float IOR_WATER = 1.333;

vec3 skyColor(vec3 dir) {
    float t = clamp(dir.y * 0.5 + 0.5, 0.0, 1.0);
    vec3 top = vec3(0.1, 0.4, 0.8);
    vec3 bottom = vec3(0.0, 0.0, 0.1);
    return mix(bottom, top, t);
}

void main() {
    vec2 coord = vUv;
    vec4 info = texture(uSimTex, coord);

    for (int i = 0; i < 5; ++i) {
        coord += info.ba * 0.01;
        info = texture(uSimTex, coord);
    }

    vec2 nXZ = info.ba;
    float ny = sqrt(max(0.0, 1.0 - dot(nXZ, nXZ)));
    vec3 normal = normalize(vec3(nXZ.x, ny, nXZ.y));

    vec3 origin = vRayOrigin;
    vec3 dir = normalize(vRayDir);

    float tPlane = -origin.y / dir.y;
    vec3 hit = origin + dir * tPlane;
    hit.y += info.r;

    vec3 toEye = normalize(origin - hit);

    vec3 refl = reflect(-toEye, normal);
    vec3 refr = refract(-toEye, normal, IOR_AIR / IOR_WATER);

    float fresnel = mix(0.02, 1.0, pow(1.0 - max(0.0, dot(normal, toEye)), 3.0));

    vec3 reflColor = skyColor(refl);
    vec3 refrColor = skyColor(refr) * vec3(0.8, 1.0, 1.1);

    vec3 color = mix(refrColor, reflColor, fresnel);

    FragColor = vec4(color, 1.0);
}
