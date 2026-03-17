#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 2) in vec3 aOffset; // Instanced offset

uniform mat4 view;
uniform mat4 projection;
uniform float u_Time;

out vec2 TexCoords;
out vec3 WorldPos;
out vec3 Normal;
out float Distance;

uniform vec3 cameraPos;

void main()
{
    // 1. Move vertex to its instanced world position
    vec3 worldPos = aPos + aOffset;
    WorldPos = worldPos;

    // 2. Wind Animation Math
    float A = 0.25;     // Amplitude (smaller for grass)
    float k = 2.0;      // Wave number
    float omega = 3.0;  // Speed
    
    float displacement = A * sin(k * worldPos.x - omega * u_Time) + 
                        0.15 * cos(k * worldPos.z - omega * u_Time * 0.7);
    
    // Base of grass (Y=0) doesn't move. Top of grass moves fully.
    float windMask = clamp(aPos.y / 0.6, 0.0, 1.0); 
    
    // Apply wind to the X axis
    worldPos.x += displacement * windMask;

    // Normal points upward with slight variation
    Normal = normalize(vec3(sin(displacement * windMask) * 0.2, 1.0, 0.0));

    // Calculate distance from camera
    Distance = length(worldPos - cameraPos);
    
    // Dummy texture coordinate for compatibility
    TexCoords = vec2(0.0);

    gl_Position = projection * view * vec4(worldPos, 1.0);
}
}