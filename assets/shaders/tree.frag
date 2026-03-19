#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 Color;
in float Distance;
in vec3 Normal;
in vec3 FragPos;

uniform vec3 skyColor;
uniform vec3 lightDir;
uniform vec3 lightColor;
uniform vec3 sunsetTint;

void main()
{
    // Dynamic light direction (follows the sun)
    vec3 norm = normalize(Normal);

    // Phong lighting with dynamic sun color
    float diff    = max(dot(norm, lightDir), 0.1);
    vec3 diffuse  = diff * Color * lightColor;
    vec3 ambient  = Color * mix(vec3(0.15), lightColor * 0.5, 0.5);

    // Edge highlighting for depth
    float fresnel = pow(1.0 - abs(dot(norm, vec3(0.0, 1.0, 0.0))), 2.0);
    vec3 litColor = mix(ambient + diffuse, Color * 0.7, fresnel * 0.3);

    // Subtle colour variation
    float detail = sin(FragPos.x * 3.0) * sin(FragPos.z * 3.0) * 0.1;
    litColor += detail * Color;

    // Sunset/sunrise warm tint
    litColor = mix(litColor, litColor * (vec3(1.0) + sunsetTint * 1.2), 0.38);

    // Distance Fog (Exponential)
    float fogDensity = 0.0005;
    float fogFactor  = exp(-pow(Distance * fogDensity, 2.0));
    fogFactor = clamp(fogFactor, 0.0, 1.0);

    vec3 finalColor = mix(skyColor, litColor, fogFactor);
    FragColor = vec4(finalColor, 1.0);
}
