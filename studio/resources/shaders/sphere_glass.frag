#version 460 core

in vec3 vWorldPos;
in vec3 vWorldNormal;
in vec3 vColor;
in vec4 vLightSpacePos;

layout(std140, binding = 0) uniform PBRUniforms {
    mat4 model;
    mat4 view;
    mat4 projection;
    mat4 lightSpaceMatrix;
    vec3 lightDir;
    float padding1;
    vec3 viewPos;
    float padding2;
    vec3 lightColor;
    float metallic;
    vec3 albedo;
    float roughness;
    float ao;
    float isGlass;   // ignored, sphere is always glass here
    vec2 padding3;
};

layout(binding = 0) uniform sampler2D shadowMap;
layout(binding = 1) uniform samplerCube environmentMap;

out vec4 FragColor;

const float PI = 3.14159265359;

float calculateShadow(vec4 lightSpacePos, vec3 normal, vec3 lightDirection) {
    vec3 projCoords = lightSpacePos.xyz / lightSpacePos.w;
    projCoords = projCoords * 0.5 + 0.5;

    if (projCoords.z > 1.0) return 0.0;
    if (projCoords.x < 0.0 || projCoords.x > 1.0 ||
        projCoords.y < 0.0 || projCoords.y > 1.0) return 0.0;

    float currentDepth = projCoords.z;

    float ndl = max(dot(normal, lightDirection), 0.0);
    float bias = max(0.0005 * (1.0 - ndl), 0.0003);

    float shadow = 0.0;
    vec2 texelSize = 1.0 / vec2(textureSize(shadowMap, 0));
    int radius = 2;

    for (int x = -radius; x <= radius; ++x) {
        for (int y = -radius; y <= radius; ++y) {
            vec2 offset = vec2(x, y) * texelSize;
            float pcfDepth = texture(shadowMap, projCoords.xy + offset).r;
            shadow += (currentDepth - bias > pcfDepth) ? 1.0 : 0.0;
        }
    }

    shadow /= float((radius * 2 + 1) * (radius * 2 + 1));
    return shadow;
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float r) {
    return F0 + (max(vec3(1.0 - r), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

void main() {
    vec3 N = normalize(vWorldNormal);
    vec3 V = normalize(viewPos - vWorldPos);
    vec3 L = normalize(lightDir);
    vec3 R = reflect(-V, N);

    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);

    //NOTE: Glass parameters
    float ior = 1.5;
    float eta = 1.0 / ior;
    vec3 F0 = vec3(0.04);

    vec3 F = fresnelSchlickRoughness(NdotV, F0, roughness);
    float fresnel = clamp((F.x + F.y + F.z) / 3.0, 0.0, 1.0);

    //NOTE: Environment reflection and refraction
    vec3 envReflect = texture(environmentMap, R).rgb;

    vec3 refractDir = refract(-V, N, eta);
    refractDir = normalize(refractDir);
    vec3 envRefract = texture(environmentMap, refractDir).rgb;

    vec3 glassBase = mix(envRefract, envReflect, fresnel);

    //NOTE: Direct lighting on the sphere (specular highlight)
    float specPower = mix(64.0, 256.0, clamp(1.0 - roughness, 0.0, 1.0));
    float specTerm = pow(max(dot(R, L), 0.0), specPower);
    vec3 specular = specTerm * lightColor * 2.0;

    //NOTE: Rim light to emphasize sphere curvature
    float rim = pow(1.0 - NdotV, 3.0);
    vec3 rimColor = vec3(0.4, 0.7, 1.0) * rim * 0.5;

    float shadow = calculateShadow(vLightSpacePos, N, L);
    float directFactor = 1.0 - shadow;

    vec3 color = glassBase;
    color += (specular + rimColor) * directFactor;

    float alpha = 0.18 + fresnel * 0.65;
    alpha = clamp(alpha, 0.15, 0.95);

    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0 / 2.2));

    FragColor = vec4(color, alpha);
}
