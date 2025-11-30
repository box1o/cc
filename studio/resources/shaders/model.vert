#version 460 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 3) in vec2 aTexcoord0; // matches loader: TEXCOORD_0 at attribute 3

layout(std140, binding = 0) uniform ModelUniforms {
    mat4 model;
    mat4 view;
    mat4 projection;
    vec3 cameraPos;
};

layout(location = 0) out vec3 vWorldPos;
layout(location = 1) out vec3 vWorldNormal;
layout(location = 2) out vec3 vViewPos;
layout(location = 3) out vec2 vUV;

void main() {
    vec4 worldPos    = model * vec4(aPosition, 1.0);
    vWorldPos        = worldPos.xyz;
    vWorldNormal     = mat3(model) * aNormal;
    vViewPos         = cameraPos;
    vUV              = aTexcoord0;

    gl_Position = projection * view * worldPos;
}
