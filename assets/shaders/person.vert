#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aColor;
layout (location = 3) in vec3 aOffset; // Instanced world position

uniform mat4 view;
uniform mat4 projection;
uniform float u_Time;

out vec3 Normal;
out vec3 FragPos;
out vec3 Color;
out float Distance;

void main()
{
    // Move vertex to its instanced world position
    vec3 worldPos = aPos + aOffset;
    
    // Simple walking animation - gentle bob and sway
    if (aPos.y > 1.0) { // Upper body parts
        float walkAnim = sin(u_Time * 3.0 + aOffset.x * 0.5) * 0.1;
        worldPos.y += walkAnim;
        worldPos.x += walkAnim * 0.5;
    }
    
    FragPos = worldPos;
    
    Normal = normalize(aNormal);
    Color = aColor;

    vec4 viewPos = view * vec4(worldPos, 1.0);
    Distance = length(viewPos.xyz);

    gl_Position = projection * viewPos;
}
