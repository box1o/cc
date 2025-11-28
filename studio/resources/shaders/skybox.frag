#version 460 core

in vec3 vTexCoords;

layout(binding = 0) uniform samplerCube skybox;

out vec4 FragColor;

void main() {
    vec3 color = texture(skybox, vTexCoords).rgb;
    FragColor = vec4(color, 1.0);
}
