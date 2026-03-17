#version 330 core
out vec4 FragColor;

in float Height;
in vec3 FragPos;
in float Distance;

uniform vec3 lightColor;
uniform vec3 skyColor;

void main()
{
    // Base water colors
    vec3 deepWater = vec3(0.0, 0.2, 0.5);
    vec3 shallowWater = vec3(0.1, 0.5, 0.8);
    
    // Normalize height roughly between 0.0 and 1.0
    float heightMix = (Height + 0.7) / 1.4;
    
    // Mix colors based on wave height
    vec3 waterColor = mix(deepWater, shallowWater, clamp(heightMix, 0.0, 1.0));
    
    // Apply light color influence
    waterColor *= (0.5 + 0.5 * lightColor);
    
    // Distance Fog (Exponential)
    float fogDensity = 0.0003;
    float fogFactor = exp(-pow(Distance * fogDensity, 2.0));
    fogFactor = clamp(fogFactor, 0.0, 1.0);

    // Mix water color with sky color based on fog factor
    vec3 finalColor = mix(skyColor, waterColor, fogFactor);
    
    // Output with transparency
    FragColor = vec4(finalColor, 0.6);
}
