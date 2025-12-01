#version 460 core

layout(location = 0) in vec3 aPosition;

layout(std140, binding = 0) uniform SkyboxUniforms {
    mat4 view;
    mat4 projection;
};

layout(location = 0) out vec3 vTexCoords;

void main()
{
    vTexCoords = aPosition;

    mat4 rotView = mat4(mat3(view));

    vec4 pos = projection * rotView * vec4(aPosition, 1.0);

    gl_Position = pos.xyww;
}
