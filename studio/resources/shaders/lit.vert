#version 460 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aUv;

layout(std140, binding = 0) uniform UBO {
    mat4 model;
    mat4 view;
    mat4 projection;
    vec4 cameraPos;
    vec4 utils;
};

layout(location = 0) out vec3 vNormal;
layout(location = 1) out vec2 vUv;
layout(location = 2) out vec3 vWorldPos;
layout(location = 3) out vec3 vCameraPos;

void main()
{
    vec4 worldPos = model * vec4(aPosition, 1.0);

    mat3 normalMatrix = transpose(inverse(mat3(model)));
    vNormal    = normalize(normalMatrix * aNormal);

    vUv        = aUv;
    vWorldPos  = worldPos.xyz;
    vCameraPos = cameraPos.xyz;

    gl_Position = projection * view * worldPos;
}
