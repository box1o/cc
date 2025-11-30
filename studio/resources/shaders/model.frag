#version 460 core

layout(location = 0) in vec3 vWorldPos;
layout(location = 1) in vec3 vWorldNormal;
layout(location = 2) in vec3 vViewPos;
layout(location = 3) in vec2 vUV;

layout(location = 0) out vec4 FragColor;

struct DirectionalLight {
    vec3 direction;
    vec3 color;
    float intensity;
};

layout(std140, binding = 1) uniform SceneLighting {
    DirectionalLight dirLight;
    vec3 ambientColor;
    float ambientIntensity;
};

// Material params (per-draw)
layout(std140, binding = 2) uniform MaterialParams {
    vec4 baseColorFactor;
    vec3 emissiveFactor;
    float metallicFactor;
    float roughnessFactor;
    int  hasBaseColorTex;
    int  hasMetallicRoughnessTex;
    int  hasNormalTex;
    int  _pad;
};

layout(binding = 0) uniform sampler2D uBaseColorTex;
layout(binding = 1) uniform sampler2D uMetallicRoughnessTex;
layout(binding = 2) uniform sampler2D uNormalTex;

vec3 getNormal() {
    vec3 N = normalize(vWorldNormal);
    if (hasNormalTex == 0) {
        return N;
    }
    // Tangent space not provided yet, so just perturb along world normal slightly
    vec3 map = texture(uNormalTex, vUV).xyz * 2.0 - 1.0;
    return normalize(N + 0.35 * map);
}

void main() {
    vec3  albedo = baseColorFactor.rgb;
    float metallic  = metallicFactor;
    float roughness = roughnessFactor;

    if (hasBaseColorTex == 1) {
        vec4 texCol = texture(uBaseColorTex, vUV);
        albedo *= texCol.rgb;
    }

    if (hasMetallicRoughnessTex == 1) {
        vec4 mr = texture(uMetallicRoughnessTex, vUV);
        // glTF MR: B = metal, G = roughness
        metallic  *= mr.b;
        roughness *= mr.g;
    }

    vec3 N = getNormal();
    vec3 V = normalize(vViewPos - vWorldPos);
    vec3 L = normalize(-dirLight.direction);

    float NdotL = max(dot(N, L), 0.0);
    vec3 diffuse = albedo * dirLight.color * dirLight.intensity * NdotL;

    float specPower = mix(8.0, 64.0, 1.0 - roughness);
    vec3 R = reflect(-L, N);
    float spec = pow(max(dot(R, V), 0.0), specPower);
    vec3 specular = dirLight.color * dirLight.intensity * spec * mix(0.04, 1.0, metallic);

    vec3 ambient = ambientColor * ambientIntensity * albedo;

    vec3 color = ambient + diffuse + specular + emissiveFactor;
    FragColor = vec4(color, baseColorFactor.a);
}
