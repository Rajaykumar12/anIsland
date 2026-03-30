#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aColor;
layout (location = 3) in vec3 aOffset; // Instanced world position

uniform mat4 view;
uniform mat4 projection;
uniform mat4 model;
uniform int uUseModel;
uniform float u_Time;

out vec3 Normal;
out vec3 FragPos;
out vec3 Color;
out float Distance;

void main()
{
    vec3 localPos = aPos;
    vec3 worldPos;
    vec3 worldNormal;
    
    // Simple walking animation for instanced crowd NPCs.
    if (uUseModel == 0 && aPos.y > 1.0) {
        float walkAnim = sin(u_Time * 3.0 + aOffset.x * 0.5) * 0.1;
        localPos.y += walkAnim;
        localPos.x += walkAnim * 0.5;
    }

    if (uUseModel == 1) {
        vec4 wp = model * vec4(localPos, 1.0);
        worldPos = wp.xyz;
        worldNormal = normalize(mat3(transpose(inverse(model))) * aNormal);
    } else {
        worldPos = localPos + aOffset;
        worldNormal = normalize(aNormal);
    }
    
    FragPos = worldPos;
    Normal = worldNormal;
    Color = aColor;

    vec4 viewPos = view * vec4(worldPos, 1.0);
    Distance = length(viewPos.xyz);

    gl_Position = projection * viewPos;
}
