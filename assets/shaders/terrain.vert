#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrix;
uniform sampler2D heightMap; 

out vec2 TexCoords;
out float Height;
out float Distance;
out vec3 Normal;
out vec3 FragPos;
out vec4 FragPosLightSpace;

void main()
{
    TexCoords = aTexCoords;
    
    // Sample height (0 to 1)
    float h = texture(heightMap, aTexCoords).r;
    
    vec3 displacedPos = aPos;
    // VERY IMPORTANT: This multiplier must match TERRAIN_MAX_HEIGHT in main.cpp!
    displacedPos.y = h * 150.0; 
    Height = displacedPos.y;

    // Calculate normal from heightmap slopes
    float texelSize = 1.0 / 512.0;
    float hL = texture(heightMap, aTexCoords - vec2(texelSize, 0.0)).r;
    float hR = texture(heightMap, aTexCoords + vec2(texelSize, 0.0)).r;
    float hD = texture(heightMap, aTexCoords - vec2(0.0, texelSize)).r;
    float hU = texture(heightMap, aTexCoords + vec2(0.0, texelSize)).r;
    
    vec3 normal = normalize(vec3(
        (hL - hR) * 375.0,
        2.0,
        (hD - hU) * 375.0
    ));
    Normal = mat3(transpose(inverse(model))) * normal;

    FragPos = vec3(model * vec4(displacedPos, 1.0));
    FragPosLightSpace = lightSpaceMatrix * vec4(FragPos, 1.0);
    
    vec4 viewPos = view * vec4(FragPos, 1.0);
    Distance = length(viewPos.xyz);

    gl_Position = projection * viewPos;
}