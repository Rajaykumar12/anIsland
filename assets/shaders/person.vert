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
    
    // Enhanced walking animation - realistic limb movement
    if (aPos.y > 1.5) { 
        // Head and upper body - gentle bob
        float walkAnim = sin(u_Time * 2.5) * 0.05;
        worldPos.y += walkAnim;
    } else if (aPos.y < 1.0) {  
        // Legs - swing forward and back
        float legSwing = sin(u_Time * 2.5 + 0.3) * 0.15;
        worldPos.z += legSwing;
        // Slight vertical variation with stride
        worldPos.y += abs(cos(u_Time * 2.5)) * 0.08;
    } else {
        // Arms - counter-rotate with body
        float armSwing = sin(u_Time * 2.5 + 1.57) * 0.12;  // 90° phase offset
        worldPos.z += armSwing;
        worldPos.y += sin(u_Time * 2.5) * 0.06;
    }
    
    FragPos = worldPos;
    
    Normal = normalize(aNormal);
    Color = aColor;

    vec4 viewPos = view * vec4(worldPos, 1.0);
    Distance = length(viewPos.xyz);

    gl_Position = projection * viewPos;
}
