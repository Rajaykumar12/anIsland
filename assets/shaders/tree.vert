#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in vec3 aColor;
layout (location = 4) in vec3 aOffset; // Instanced world position

uniform mat4 view;
uniform mat4 projection;

out vec2 TexCoords;
out vec3 Color;
out float Distance;
out vec3 Normal;
out vec3 FragPos;

void main()
{
    TexCoords = aTexCoords;
    Color = aColor;

    // Move vertex to its instanced world position
    vec3 worldPos = aPos + aOffset;
    FragPos = worldPos;
    
    // Simple normal transformation
    Normal = normalize(aNormal);

    vec4 viewPos = view * vec4(worldPos, 1.0);
    Distance = length(viewPos.xyz); // For fog

    gl_Position = projection * viewPos;
}
