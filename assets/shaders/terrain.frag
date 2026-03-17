#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in float Height;
in float Distance;
in vec3 Normal;
in vec3 FragPos;
in vec4 FragPosLightSpace;

uniform vec3 skyColor;
uniform sampler2D shadowMap;

float ShadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir)
{
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    
    if(projCoords.z > 1.0) return 0.0;

    float currentDepth = projCoords.z;
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);

    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;
    
    return shadow;
}

void main()
{
    // Define vibrant terrain colors based on height
    vec3 grassColor = vec3(0.25, 0.65, 0.15);      // Vibrant bright green
    vec3 dirtColor  = vec3(0.55, 0.42, 0.25);      // Warm brown
    vec3 rockColor  = vec3(0.5, 0.48, 0.45);       // Warm dark gray
    vec3 snowColor  = vec3(0.95, 0.96, 1.0);       // Bright white snow

    vec3 terrainColor;

    // Smooth blending between bands with adjusted thresholds
    if (Height < 25.0) {
        terrainColor = grassColor;
    } else if (Height < 50.0) {
        float t = smoothstep(25.0, 50.0, Height);
        terrainColor = mix(grassColor, dirtColor, t);
    } else if (Height < 85.0) {
        float t = smoothstep(50.0, 85.0, Height);
        terrainColor = mix(dirtColor, rockColor, t);
    } else if (Height < 120.0) {
        float t = smoothstep(85.0, 120.0, Height);
        terrainColor = mix(rockColor, snowColor, t);
    } else {
        terrainColor = snowColor;
    }

    // Add detail with pseudo-random variation
    vec2 detailCoord = TexCoords * 10.0;
    float detail = sin(detailCoord.x * 12.9898 + detailCoord.y * 78.233) * 0.5 + 0.5;
    terrainColor += detail * 0.05;

    // Light direction (sun in sky)
    vec3 lightDir = normalize(vec3(0.8, 1.0, 0.5));
    vec3 norm = normalize(Normal);
    
    // Calculate shadow
    float shadow = ShadowCalculation(FragPosLightSpace, norm, lightDir);
    
    // Phong lighting
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * terrainColor * 0.8;
    
    vec3 ambient = terrainColor * 0.4;
    
    // Slope shadowing for more drama
    float slope = 1.0 - dot(norm, vec3(0.0, 1.0, 0.0));
    vec3 litColor = mix(ambient + diffuse, ambient, slope * 0.3);
    
    // Apply shadow to diffuse component
    litColor = ambient + (1.0 - shadow) * (diffuse + slope * 0.3 * terrainColor * 0.2);

    // Distance Fog (Exponential)
    float fogDensity = 0.0002;
    float fogFactor = exp(-pow(Distance * fogDensity, 2.0));
    fogFactor = clamp(fogFactor, 0.0, 1.0);

    // Mix terrain color with sky color based on fog factor
    vec3 finalColor = mix(skyColor, litColor, fogFactor);

    FragColor = vec4(finalColor, 1.0);
}