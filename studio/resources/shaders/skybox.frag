#version 460 core

layout(location = 0) in vec3 vTexCoords;
layout(location = 0) out vec4 FragColor;

layout(binding = 0) uniform samplerCube skybox;

void main() {
    vec3 dir   = normalize(vTexCoords);
    vec3 color = texture(skybox, dir).rgb;
    FragColor  = vec4(color, 1.0);
}
