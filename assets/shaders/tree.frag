#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 Color;
in float Distance;
in vec3 Normal;
in vec3 FragPos;

uniform vec3 skyColor;

void main()
{
    // Light direction (sun)
    vec3 lightDir = normalize(vec3(0.8, 1.0, 0.5));
    vec3 norm = normalize(Normal);
    
    // Phong lighting
    float diff = max(dot(norm, lightDir), 0.2);
    vec3 diffuse = diff * Color;
    vec3 ambient = Color * 0.5;
    
    // Add edge highlighting for depth
    float fresnel = pow(1.0 - abs(dot(norm, vec3(0.0, 1.0, 0.0))), 2.0);
    vec3 litColor = mix(ambient + diffuse, Color * 0.7, fresnel * 0.3);
    
    // Add subtle color variation
    float detail = sin(FragPos.x * 3.0) * sin(FragPos.z * 3.0) * 0.1;
    litColor += detail * Color;
    
    // Distance Fog (Exponential)
    float fogDensity = 0.0005;
    float fogFactor = exp(-pow(Distance * fogDensity, 2.0));
    fogFactor = clamp(fogFactor, 0.0, 1.0);

    // Mix tree color with sky color based on fog factor
    vec3 finalColor = mix(skyColor, litColor, fogFactor);

    FragColor = vec4(finalColor, 1.0);
}
