#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 2) in vec3 aOffset; // Instanced offset

uniform mat4  view;
uniform mat4  projection;
uniform float u_Time;
uniform float windStrength;   // 0.0–1.0, driven by slow sine from CPU

out vec2  TexCoords;
out vec3  WorldPos;
out vec3  Normal;
out float Distance;

uniform vec3 cameraPos;

void main()
{
    // Move vertex to its instanced world position
    vec3 worldPos = aPos + aOffset;
    WorldPos = worldPos;

    // Dynamic wind amplitude and speed based on windStrength
    float A     = 0.08 + windStrength * 0.35;   // amplitude: calm 0.08, gusty 0.43
    float k     = 2.0;
    float omega = 2.0 + windStrength * 3.0;     // speed: slow 2, fast 5

    float displacement = A * sin(k * worldPos.x - omega * u_Time + aOffset.z * 0.4)
                       + (A * 0.4) * cos(k * worldPos.z - omega * u_Time * 0.7 + aOffset.x * 0.3);

    // Base of grass (Y=0) stays fixed; top sways fully
    float windMask = clamp(aPos.y / 0.6, 0.0, 1.0);

    worldPos.x += displacement * windMask;

    Normal = normalize(vec3(sin(displacement * windMask) * 0.2, 1.0, 0.0));

    Distance = length(worldPos - cameraPos);
    TexCoords = vec2(0.0);

    gl_Position = projection * view * vec4(worldPos, 1.0);
}