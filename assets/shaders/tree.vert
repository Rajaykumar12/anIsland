#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in vec3 aColor;
layout (location = 4) in vec3 aOffset; // Instanced world position

uniform mat4  view;
uniform mat4  projection;
uniform float u_Time;
uniform float windStrength;   // 0.0–1.0 from CPU

out vec2  TexCoords;
out vec3  Color;
out float Distance;
out vec3  Normal;
out vec3  FragPos;

void main()
{
    TexCoords = aTexCoords;
    Color     = aColor;

    // Move vertex to its instanced world position
    vec3 worldPos = aPos + aOffset;

    // Canopy sway: only affect the top part of the tree (y > some threshold above offset)
    // aPos.y is local; canopy starts at y ~= 1.5 (trunk top)
    float canopyMask = clamp((aPos.y - 1.5) / 3.0, 0.0, 1.0);

    float A     = 0.06 + windStrength * 0.22;
    float omega = 1.2  + windStrength * 1.5;

    // Unique phase per tree instance to break synchronisation
    float phase = aOffset.x * 0.13 + aOffset.z * 0.17;
    float swayX = A * sin(omega * u_Time + phase);
    float swayZ = A * 0.6 * cos(omega * 0.7 * u_Time + phase + 1.0);

    worldPos.x += swayX * canopyMask;
    worldPos.z += swayZ * canopyMask;

    FragPos  = worldPos;
    Normal   = normalize(aNormal);

    vec4 viewPos = view * vec4(worldPos, 1.0);
    Distance = length(viewPos.xyz);

    gl_Position = projection * viewPos;
}
