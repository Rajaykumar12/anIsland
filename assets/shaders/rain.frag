#version 330 core
out vec4 FragColor;

void main()
{
    // Elongated raindrop shape: bright core, transparent edges
    vec2  coord = 2.0 * gl_PointCoord - 1.0;
    // Make drops feel elongated vertically
    float dist  = length(vec2(coord.x * 3.0, coord.y));
    if(dist > 1.0) discard;

    float alpha = (1.0 - dist) * 0.55;
    // Blue-white rain colour
    vec3  rainColor = vec3(0.72, 0.82, 1.0);
    FragColor = vec4(rainColor, alpha);
}
