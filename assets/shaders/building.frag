#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;
in vec3 Color;
in float Distance;

uniform vec3 skyColor;
uniform vec3 lightDir;
uniform vec3 lightColor;
uniform vec3 sunsetTint;
uniform vec3 cameraPos;

void main()
{
    // Dynamic light direction (follows the sun)
    vec3 norm = normalize(Normal);

    // Phong lighting with dynamic sun color
    float diff    = max(dot(norm, lightDir), 0.2);
    vec3 diffuse  = diff * Color * 0.9 * lightColor;
    vec3 ambient  = Color * mix(vec3(0.2), lightColor * 0.45, 0.5);

    // Specular highlights
    vec3 viewDir    = normalize(cameraPos - FragPos);
    vec3 halfDir    = normalize(lightDir + viewDir);
    float spec      = pow(max(dot(norm, halfDir), 0.0), 32.0);
    vec3 specular   = spec * lightColor * 0.25;

    vec3 litColor = ambient + diffuse + specular;

    // Sunset/sunrise warm tint
    litColor = mix(litColor, litColor * (vec3(1.0) + sunsetTint * 1.1), 0.35);

    // Distance Fog
    float fogDensity = 0.003;
    float fogFactor  = exp(-pow(Distance * fogDensity, 2.0));
    fogFactor = clamp(fogFactor, 0.0, 1.0);

    vec3 finalColor = mix(skyColor, litColor, fogFactor);
    FragColor = vec4(finalColor, 1.0);
}
