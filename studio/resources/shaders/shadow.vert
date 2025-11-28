#version 460 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec3 aColor;

layout(std140, binding = 0) uniform ShadowUniforms {
    mat4 model;
    mat4 lightSpaceMatrix;
};

void main() {
    gl_Position = lightSpaceMatrix * model * vec4(aPosition, 1.0);
}
