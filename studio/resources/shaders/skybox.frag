#version 460 core

in vec3 vTexCoords;

layout(binding = 0) uniform samplerCube skybox;

out vec4 FragColor;

void main() {
    FragColor = texture(skybox, vTexCoords);
}
