#version 460 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;

layout(std140, binding = 0) uniform ReflectUniforms {
    mat4 model;
    mat4 view;
    mat4 projection;
    vec3 cameraPos;
};

layout(location = 0) out vec3 vReflectDir;

void main() {
    mat4 modelView = view * model;

    vec4 worldPos = model * vec4(aPosition, 1.0);
    vec3 worldNormal = normalize(mat3(model) * aNormal);

    vec3 I = normalize(worldPos.xyz - cameraPos);
    vReflectDir = reflect(I, worldNormal);

    gl_Position = projection * view * vec4(aPosition, 1.0);
}
