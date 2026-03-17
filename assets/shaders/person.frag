#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;
in vec3 Color;
in float Distance;

uniform vec3 skyColor;

void main()
{
    // Light direction (sun)
    vec3 lightDir = normalize(vec3(0.8, 1.0, 0.5));
    vec3 norm = normalize(Normal);
    
    // Phong lighting
    float diff = max(dot(norm, lightDir), 0.3);
    vec3 diffuse = diff * Color;
    
    vec3 ambient = Color * 0.5;
    
    vec3 litColor = ambient + diffuse;
    
    // Distance Fog
    float fogDensity = 0.003;
    float fogFactor = exp(-pow(Distance * fogDensity, 2.0));
    fogFactor = clamp(fogFactor, 0.0, 1.0);

    vec3 finalColor = mix(skyColor, litColor, fogFactor);

    FragColor = vec4(finalColor, 1.0);
}
