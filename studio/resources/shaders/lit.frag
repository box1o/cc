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
    float isGlass;
    vec2 padding3;
};

layout(binding = 0) uniform sampler2D shadowMap;
layout(binding = 1) uniform samplerCube environmentMap;

out vec4 FragColor;

const float PI = 3.14159265359;

float DistributionGGX(vec3 N, vec3 H, float r) {
    float a  = r * r;
    float a2 = a * a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
    return a2 / max(denom, 0.0001);
}

float GeometrySchlickGGX(float NdotV, float r) {
    float k = ((r + 1.0) * (r + 1.0)) / 8.0;
    return NdotV / (NdotV * (1.0 - k) + k);
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float r) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggxV = GeometrySchlickGGX(NdotV, r);
    float ggxL = GeometrySchlickGGX(NdotL, r);
    return ggxV * ggxL;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float r) {
    return F0 + (max(vec3(1.0 - r), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

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

void main() {
    vec3 N = normalize(vWorldNormal);
    vec3 V = normalize(viewPos - vWorldPos);
    vec3 L = normalize(lightDir);
    vec3 H = normalize(V + L);
    vec3 R = reflect(-V, N);

    vec3 baseColor = vColor * albedo;

    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);

    if (isGlass > 0.5) {
        vec3 F0 = vec3(0.04);
        float ior = 1.5;
        float eta = 1.0 / ior;

        vec3 F = fresnelSchlickRoughness(NdotV, F0, roughness);
        float fresnel = clamp((F.x + F.y + F.z) / 3.0, 0.0, 1.0);

        vec3 envReflect = texture(environmentMap, R).rgb;

        vec3 refractDir = refract(-V, N, eta);
        refractDir = normalize(refractDir);
        vec3 envRefract = texture(environmentMap, refractDir).rgb;

        vec3 glassColor = mix(envRefract, envReflect, fresnel);

        float specPower = mix(64.0, 256.0, 1.0 - roughness);
        float specTerm = pow(max(dot(R, L), 0.0), specPower);
        vec3 specHighlight = specTerm * lightColor * 2.0;

        float rim = pow(1.0 - NdotV, 3.5);
        vec3 rimColor = vec3(0.4, 0.7, 1.0) * rim * 0.6;

        float shadow = calculateShadow(vLightSpacePos, N, L);
        float directFactor = 1.0 - shadow;

        glassColor += (specHighlight + rimColor) * directFactor;

        float alpha = 0.12 + fresnel * 0.7;
        alpha = clamp(alpha, 0.1, 0.95);

        glassColor = glassColor / (glassColor + vec3(1.0));
        glassColor = pow(glassColor, vec3(1.0 / 2.2));

        FragColor = vec4(glassColor, alpha);
    } else {
        vec3 F0 = mix(vec3(0.04), baseColor, metallic);

        float NDF = DistributionGGX(N, H, roughness);
        float G   = GeometrySmith(N, V, L, roughness);
        vec3  F   = fresnelSchlick(max(dot(H, V), 0.0), F0);

        vec3 kS = F;
        vec3 kD = (1.0 - kS) * (1.0 - metallic);

        float denom = 4.0 * NdotV * NdotL + 0.0001;
        vec3 specular = (NDF * G * F) / denom;

        vec3 Lo = (kD * baseColor / PI + specular) * lightColor * NdotL;

        float shadow = calculateShadow(vLightSpacePos, N, L);
        Lo *= (1.0 - shadow * 0.8);

        vec3 envF = fresnelSchlickRoughness(NdotV, F0, roughness);
        vec3 envColor = texture(environmentMap, R).rgb;
        vec3 envSpecular = envF * envColor * (1.0 - roughness) * 0.35;

        vec3 ambient = vec3(0.03) * baseColor * ao;

        vec3 color = ambient + Lo + envSpecular;

        color = color / (color + vec3(1.0));
        color = pow(color, vec3(1.0 / 2.2));

        FragColor = vec4(color, 1.0);
    }
}
