#version 460 core

layout(location = 0) in vec3 vWorldPos;
layout(location = 1) in vec3 vWorldNormal;
layout(location = 2) in vec3 vWorldTangent;
layout(location = 3) in vec3 vWorldBitangent;
layout(location = 4) in vec2 vTexCoord;
layout(location = 5) in vec4 vColor;

layout(location = 0) out vec4 FragColor;

struct DirectionalLightGPU {
    vec3 direction;
    float _pad0;
    vec3 color;
    float intensity;
};

layout(std140, binding = 1) uniform SceneLighting {
    DirectionalLightGPU uDirLight;
    vec3 uAmbientColor;
    float uAmbientIntensity;
};

layout(std140, binding = 2) uniform MaterialParams {
    vec4  uBaseColorFactor;
    vec3  uEmissiveFactor;
    float uMetallicFactor;
    float uRoughnessFactor;
    int   uHasBaseColorTex;
    int   uHasMetallicRoughnessTex;
    int   uHasNormalTex;
    int   _pad;
};

layout(binding = 0) uniform sampler2D uBaseColorTex;
layout(binding = 1) uniform sampler2D uMetallicRoughnessTex;
layout(binding = 2) uniform sampler2D uNormalTex;

vec3 GetNormal()
{
    vec3 N = normalize(vWorldNormal);
    if (uHasNormalTex == 0) {
        return N;
    }

    vec3 T = normalize(vWorldTangent);
    vec3 B = normalize(vWorldBitangent);

    mat3 TBN = mat3(T, B, N);

    vec3 n = texture(uNormalTex, vTexCoord).xyz * 2.0 - 1.0;
    return normalize(TBN * n);
}

void main()
{
    vec4 baseColor = uBaseColorFactor;
    if (uHasBaseColorTex == 1) {
        baseColor *= texture(uBaseColorTex, vTexCoord);
    }
    baseColor *= vColor;

    float metallic  = uMetallicFactor;
    float roughness = uRoughnessFactor;

    if (uHasMetallicRoughnessTex == 1) {
        vec4 mr = texture(uMetallicRoughnessTex, vTexCoord);
        metallic  *= mr.b;
        roughness *= mr.g;
    }

    vec3 N = GetNormal();

    vec3 L = normalize(-uDirLight.direction);
    vec3 V = normalize(uDirLight.direction); //NOTE: cheap stand-in if you don't pass cameraPos here
    vec3 H = normalize(L + V);

    float NdotL = max(dot(N, L), 0.0);
    float NdotV = max(dot(N, V), 0.0);
    float NdotH = max(dot(N, H), 0.0);
    float VdotH = max(dot(V, H), 0.0);

    vec3 F0 = mix(vec3(0.04), baseColor.rgb, metallic);

    float a      = roughness * roughness;
    float a2     = a * a;
    float denomD = (NdotH * NdotH) * (a2 - 1.0) + 1.0;
    denomD       = 3.14159265 * denomD * denomD;
    float D      = a2 / max(denomD, 1e-4);

    float k = (roughness + 1.0);
    k = (k * k) / 8.0;

    float Gv = NdotV / max(NdotV * (1.0 - k) + k, 1e-4);
    float Gl = NdotL / max(NdotL * (1.0 - k) + k, 1e-4);
    float G  = Gv * Gl;

    vec3  F  = F0 + (1.0 - F0) * pow(1.0 - VdotH, 5.0);
    vec3  specular = (D * G * F) / max(4.0 * NdotV * NdotL, 1e-4);

    vec3 kd = (1.0 - F) * (1.0 - metallic);
    vec3 diffuse = kd * baseColor.rgb / 3.14159265;

    vec3 radiance = uDirLight.color * uDirLight.intensity;

    vec3 Lo = (diffuse + specular) * radiance * NdotL;
    vec3 ambient = uAmbientColor * uAmbientIntensity * baseColor.rgb;

    vec3 emissive = uEmissiveFactor;

    vec3 color = ambient + Lo + emissive;

    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0 / 2.2));

    FragColor = vec4(color, baseColor.a);
}
