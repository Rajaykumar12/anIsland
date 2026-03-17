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
    float diff = max(dot(norm, lightDir), 0.2);
    vec3 diffuse = diff * Color * 0.9;
    
    vec3 ambient = Color * 0.4;
    
    // Specular highlights on windows
    vec3 viewDir = normalize(-FragPos); // Simplified: assume camera at origin
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    vec3 specular = spec * vec3(1.0, 1.0, 0.8) * 0.3;
    
    vec3 litColor = ambient + diffuse + specular;
    
    // Distance Fog
    float fogDensity = 0.003;
    float fogFactor = exp(-pow(Distance * fogDensity, 2.0));
    fogFactor = clamp(fogFactor, 0.0, 1.0);

    vec3 finalColor = mix(skyColor, litColor, fogFactor);

    FragColor = vec4(finalColor, 1.0);
}
