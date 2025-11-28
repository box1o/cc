#version 460 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec2 aUV;

layout(std140, binding = 0) uniform CubeUniforms {
    mat4 model;
    mat4 view;
    mat4 projection;
};

out vec2 vUV;

void main() {
    vUV = aUV;
    gl_Position = projection * view * model * vec4(aPosition, 1.0);
}
