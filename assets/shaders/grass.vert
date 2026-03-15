#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;
layout (location = 2) in vec3 aOffset; // Instanced offset!

uniform mat4 view;
uniform mat4 projection;
uniform float u_Time;

out vec2 TexCoords;

void main()
{
    TexCoords = aTexCoords;

    // 1. Move vertex to its instanced world position
    vec3 worldPos = aPos + aOffset;

    // 2. Wind Animation Math
    float A = 0.5;      // Amplitude
    float k = 1.0;      // Wave number
    float omega = 2.0;  // Speed
    
    float displacement = A * sin(k * worldPos.x - omega * u_Time);
    
    // Base of grass (Y=0) doesn't move. Top of grass moves fully.
    float windMask = clamp(aPos.y, 0.0, 1.0); 
    
    // Apply wind to the X axis
    worldPos.x += displacement * windMask;

    gl_Position = projection * view * vec4(worldPos, 1.0);
}