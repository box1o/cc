#version 460 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec3 aUv;

layout(std140, binding = 0) uniform UBO {
    mat4 model;
    mat4 view;
    mat4 projection;
};

layout(location = 0) out vec3 vNormal;
layout(location = 1) out vec3 vUv;

void main()
{
    vNormal = aNormal;
    vUv = aUv;
    gl_Position = projection * view * model * vec4(aPosition, 1.0);
}
