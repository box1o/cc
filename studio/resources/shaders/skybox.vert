#version 460 core

layout(location = 0) in vec3 aPosition;

layout(std140, binding = 0) uniform SkyboxUniforms {
    mat4 viewProjection;
};

out vec3 vTexCoords;

void main() {
    vTexCoords = aPosition;
    vec4 pos = viewProjection * vec4(aPosition, 1.0);
    gl_Position = pos.xyww;
}
