#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in float aSize;

uniform mat4 view;
uniform mat4 projection;

out float Alpha;

void main()
{
    gl_Position = projection * view * vec4(aPos, 1.0);
    gl_PointSize = aSize * 10.0; // Scale for visibility
    Alpha = 1.0;
}
