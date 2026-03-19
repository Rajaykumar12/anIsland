#version 330 core
out vec4 FragColor;
in vec2 TexCoords;
in vec3 WorldPos;
in vec3 Normal;
in float Distance;

uniform vec3 skyColor;
uniform vec3 lightDir;
uniform vec3 lightColor;
uniform vec3 sunsetTint;

void main()
{
    // Create pseudo-random color variation based on position
    float rand = fract(sin(WorldPos.x * 12.9898 + WorldPos.z * 78.233) * 43758.5453);
    
    // Base grass green with variation
    vec3 baseGreen  = vec3(0.3, 0.9, 0.3);
    vec3 darkGreen  = vec3(0.25, 0.75, 0.2);
    vec3 lightGreen = vec3(0.4, 1.0, 0.35);
    
    vec3 grassColor = mix(darkGreen, lightGreen, rand);
    
    // Dynamic lighting
    vec3 norm = normalize(Normal);
    float diff = max(dot(norm, lightDir), 0.1);
    
    // AO based on position
    float ao = 0.7 + 0.3 * sin(WorldPos.x * 2.0) * sin(WorldPos.z * 2.0);
    
    vec3 litColor = grassColor * lightColor * diff * ao;

    // Sunset/sunrise warm tint
    litColor = mix(litColor, litColor * (vec3(1.0) + sunsetTint * 1.3), 0.35);

    // Distance Fog (Exponential)
    float fogDensity = 0.0005;
    float fogFactor  = exp(-pow(Distance * fogDensity, 2.0));
    fogFactor = clamp(fogFactor, 0.0, 1.0);

    vec3 finalColor = mix(skyColor, litColor, fogFactor);
    FragColor = vec4(finalColor, 1.0);
}