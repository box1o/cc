#version 460 core

layout(location = 0) in vec3 vNormal;
layout(location = 1) in vec3 vUv;

layout(location = 0) out vec4 FragColor;

void main()
{
    FragColor = vec4(vNormal, 1.0);
}
