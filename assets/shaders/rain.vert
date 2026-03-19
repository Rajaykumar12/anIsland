#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 projection;
uniform mat4 view;

void main()
{
    gl_Position = projection * view * vec4(aPos, 1.0);
    // Fixed large point size for rain streaks
    gl_PointSize = max(1.5, 40.0 / gl_Position.w);
}
