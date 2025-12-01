#version 460 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec4 aTangent;
layout(location = 3) in vec2 aTexCoord;
layout(location = 4) in vec4 aColor;

layout(std140, binding = 0) uniform ModelUniforms {
    mat4 uModel;
    mat4 uView;
    mat4 uProjection;
    vec3 uCameraPos;
    float _pad0;
};

layout(location = 0) out vec3 vWorldPos;
layout(location = 1) out vec3 vWorldNormal;
layout(location = 2) out vec3 vWorldTangent;
layout(location = 3) out vec3 vWorldBitangent;
layout(location = 4) out vec2 vTexCoord;
layout(location = 5) out vec4 vColor;

void main()
{
    vec4 worldPos = uModel * vec4(aPosition, 1.0);
    vWorldPos = worldPos.xyz;

    mat3 normalMat = mat3(transpose(inverse(uModel)));
    vWorldNormal   = normalize(normalMat * aNormal);

    vec3 T = normalize(normalMat * aTangent.xyz);
    vec3 B = normalize(cross(vWorldNormal, T) * aTangent.w);
    vWorldTangent   = T;
    vWorldBitangent = B;

    vTexCoord = aTexCoord;
    vColor    = aColor;

    gl_Position = uProjection * uView * worldPos;
}
