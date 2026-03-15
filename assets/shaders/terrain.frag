#version 330 core
out vec4 FragColor;
in float Height;

void main()
{
    // Simple coloring based on height (brown for dirt, gray for peaks)
    vec3 color = mix(vec3(0.4, 0.3, 0.1), vec3(0.8, 0.8, 0.8), Height / 20.0);
    FragColor = vec4(color, 1.0);
}