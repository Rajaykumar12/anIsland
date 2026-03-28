#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 2) in vec3 aOffset; // Instanced offset
layout (location = 3) in vec3 aNormal; // Instanced terrain normal

uniform mat4 view;
uniform mat4 projection;

out vec2  TexCoords;
out vec3  WorldPos;
out vec3  Normal;
out float Distance;

uniform vec3 cameraPos;

void main()
{
    vec3 up = normalize(aNormal);
    vec3 helper = (abs(up.y) > 0.9) ? vec3(1.0, 0.0, 0.0) : vec3(0.0, 1.0, 0.0);
    vec3 tangent = normalize(cross(helper, up));
    vec3 bitangent = normalize(cross(up, tangent));

    vec3 localPos = tangent * aPos.x + up * aPos.y + bitangent * aPos.z;
    vec3 worldPos = aOffset + localPos;
    WorldPos = worldPos;
    Normal = up;

    Distance = length(worldPos - cameraPos);
    TexCoords = vec2(0.0);

    gl_Position = projection * view * vec4(worldPos, 1.0);
}