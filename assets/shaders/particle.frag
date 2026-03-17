#version 330 core
out vec4 FragColor;

void main()
{
    // Map point coordinates to a circle
    vec2 circCoord = 2.0 * gl_PointCoord - 1.0;
    float distance = dot(circCoord, circCoord);
    
    if(distance > 1.0) discard;
    
    // Create soft glowing center (yellowish-green firefly)
    float alpha = 1.0 - distance;
    vec3 glowColor = vec3(0.9, 1.0, 0.1);
    
    FragColor = vec4(glowColor, alpha * 0.8);
}
