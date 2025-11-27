#version 460 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aColor;

layout(std140, binding = 0) uniform Uniforms {
    mat4 model;
    mat4 view;
    mat4 projection;
};

out vec3 vColor;

void main() {
    gl_Position = projection * view * model * vec4(aPosition, 1.0);
    vColor = aColor;
}
