#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float u_Time;

out float Height;
out vec3  FragPos;
out vec3  Normal;
out float Distance;

uniform vec3 cameraPos;

void main()
{
    vec3 pos = aPos;

    // Wave 1: Slow, large waves traveling diagonally
    float wave1 = 0.4  * sin(pos.x * 0.3  + pos.z * 0.3  + u_Time * 1.0);
    // Wave 2: Fast, small ripples traveling along X
    float wave2 = 0.15 * sin(pos.x * 1.5  + u_Time * 2.5);
    // Wave 3: Medium ripples along Z
    float wave3 = 0.15 * sin(pos.z * 1.5  - u_Time * 2.0);

    pos.y = wave1 + wave2 + wave3;
    Height = pos.y;

    // Approximate surface normal from wave gradients
    float dx = 0.4  * 0.3  * cos(pos.x * 0.3  + pos.z * 0.3  + u_Time * 1.0)
             + 1.5  * 0.15 * cos(pos.x * 1.5  + u_Time * 2.5);
    float dz = 0.4  * 0.3  * cos(pos.x * 0.3  + pos.z * 0.3  + u_Time * 1.0)
             - 1.5  * 0.15 * cos(pos.z * 1.5  - u_Time * 2.0);
    Normal = normalize(vec3(-dx, 1.0, -dz));

    FragPos  = vec3(model * vec4(pos, 1.0));
    Distance = length(FragPos - cameraPos);

    gl_Position = projection * view * model * vec4(pos, 1.0);
}
