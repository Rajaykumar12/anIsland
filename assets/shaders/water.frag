#version 330 core
out vec4 FragColor;

in float Height;
in vec3  FragPos;
in vec3  Normal;
in float Distance;

uniform vec3  lightDir;
uniform vec3  lightColor;
uniform vec3  skyColor;
uniform vec3  cameraPos;
uniform vec3  sunsetTint;

void main()
{
    // Base water colors
    vec3 deepWater    = vec3(0.0, 0.18, 0.48);
    vec3 shallowWater = vec3(0.1, 0.48, 0.78);

    // Wave-height colour blend
    float heightMix = (Height + 0.7) / 1.4;
    vec3  waterColor = mix(deepWater, shallowWater, clamp(heightMix, 0.0, 1.0));

    // Apply sun/night light tint
    waterColor *= (0.4 + 0.6 * lightColor);

    // Sunset warm tint on water
    waterColor = mix(waterColor, waterColor * (vec3(1.0) + sunsetTint * 0.8), 0.3);

    // --- Fresnel + Specular ---
    vec3  norm    = normalize(Normal);
    vec3  viewDir = normalize(cameraPos - FragPos);
    vec3  halfDir = normalize(lightDir + viewDir);

    // Specular highlight
    float spec    = pow(max(dot(norm, halfDir), 0.0), 128.0);
    vec3  specular = lightColor * spec * 0.9;

    // Fresnel: more reflective at glancing angles
    float cosTheta = max(dot(norm, viewDir), 0.0);
    float fresnel  = pow(1.0 - cosTheta, 3.0);
    // Blend water toward sky colour at Fresnel edges
    waterColor = mix(waterColor, skyColor * 0.85, fresnel * 0.45);
    waterColor += specular;

    // Distance Fog (Exponential)
    float fogDensity = 0.0003;
    float fogFactor  = exp(-pow(Distance * fogDensity, 2.0));
    fogFactor = clamp(fogFactor, 0.0, 1.0);

    vec3 finalColor = mix(skyColor, waterColor, fogFactor);
    FragColor = vec4(finalColor, 0.72);
}
