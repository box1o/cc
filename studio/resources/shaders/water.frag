#version 460 core

in vec3 vWorldPos;
in vec3 vWorldNormal;
in vec3 vViewPos;
in vec2 vUV;

layout(std140, binding = 0) uniform WaterUniforms {
    mat4 model;
    mat4 view;
    mat4 projection;
    vec3 viewPos;
    float time;
    vec3 waterColor;
    float waveStrength;
};

layout(binding = 0) uniform samplerCube environmentMap;

out vec4 FragColor;

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

void main() {
    vec3 N = normalize(vWorldNormal);
    vec3 V = normalize(viewPos - vWorldPos);

    // Simple procedural normal perturbation to simulate small waves
    float wave1 = sin(vUV.x * 20.0 + time * 1.5);
    float wave2 = cos(vUV.y * 17.0 - time * 1.0);
    float wave3 = sin((vUV.x + vUV.y) * 30.0 + time * 2.3);

    float wave = (wave1 + wave2 + wave3) / 3.0;
    vec3 perturb = normalize(vec3(
        dFdx(wave),     // approximate slope
        1.0,
        dFdy(wave)
    ));

    N = normalize(mix(N, perturb, waveStrength));

    float NdotV = max(dot(N, V), 0.0);

    float ior = 1.33; // water
    float eta = 1.0 / ior;

    vec3 F0 = vec3(0.02, 0.03, 0.05);
    vec3 F  = fresnelSchlick(NdotV, F0);
    float fresnel = clamp((F.x + F.y + F.z) / 3.0, 0.0, 1.0);

    vec3 R = reflect(-V, N);
    vec3 reflectColor = texture(environmentMap, R).rgb;

    vec3 refractDir = refract(-V, N, eta);
    refractDir = normalize(refractDir);
    vec3 refractColor = texture(environmentMap, refractDir).rgb;

    vec3 base = mix(refractColor, reflectColor, fresnel);

    // Depth / tint approximation: stronger tint when looking down
    float depthFactor = pow(1.0 - NdotV, 0.7);
    vec3 tinted = mix(base, base * waterColor, depthFactor * 0.6);

    // Slight darkening
    tinted *= 0.9;

    tinted = tinted / (tinted + vec3(1.0));
    tinted = pow(tinted, vec3(1.0 / 2.2));

    float alpha = 0.85;
    FragColor = vec4(tinted, alpha);
}
