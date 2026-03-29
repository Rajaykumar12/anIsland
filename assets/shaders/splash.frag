#version 330 core
out vec4 FragColor;

in float Alpha;

uniform vec3 skyColor;

void main()
{
    // Create circular splash
    vec2 coord = gl_PointCoord - vec2(0.5);
    float dist = length(coord);

    if (dist > 0.5) discard;

    // White/blue splash color
    vec3 splashColor = vec3(0.85, 0.9, 1.0);

    // Fade edges
    float alpha = (0.5 - dist) * 2.0 * Alpha;

    FragColor = vec4(splashColor, alpha * 0.6);
}
