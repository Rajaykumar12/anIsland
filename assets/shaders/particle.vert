#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 projection;
uniform mat4 view;

void main()
{
    gl_Position = projection * view * vec4(aPos, 1.0);
    // Make points larger if closer to camera, smaller if far
    gl_PointSize = max(2.0, 100.0 / gl_Position.w);
}
