#version 460 core

in vec2 vUV;

layout(binding = 0) uniform sampler2D diffuseTex;

out vec4 FragColor;

void main() {
    FragColor = texture(diffuseTex, vUV);
}
