#version 460 core

layout(location = 0) in vec3 vNormal;
layout(location = 1) in vec2 vUv;
layout(location = 2) in vec3 vWorldPos;
layout(location = 3) in vec3 vCameraPos;

layout(binding = 1) uniform samplerCube envMap;
layout(binding = 2) uniform sampler2D albedoMap;

layout(location = 0) out vec4 FragColor;

void main()
{
    vec3 N = normalize(vNormal);
    vec3 V = normalize(vCameraPos - vWorldPos);
    vec3 R = reflect(-V, N);

    vec3 reflection = texture(envMap, R).rgb;
    vec3 baseColor = texture(albedoMap, vUv).rgb;

    vec3 finalColor = mix(baseColor, reflection, .4);

    FragColor = vec4(finalColor, 1.0);
}
