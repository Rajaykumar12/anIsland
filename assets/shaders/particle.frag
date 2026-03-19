#version 330 core
out vec4 FragColor;

uniform float dayIntensity;   // 0 = full night, 1 = full day

void main()
{
    // Map point coordinates to a circle
    vec2  circCoord = 2.0 * gl_PointCoord - 1.0;
    float dist      = dot(circCoord, circCoord);

    if(dist > 1.0) discard;

    // Soft glowing core (yellowish-green firefly)
    float alpha    = 1.0 - dist;

    // Fireflies only visible at night – fade out during the day
    float nightFactor = 1.0 - smoothstep(0.15, 0.55, dayIntensity);

    // Slight colour flicker based on gl_PointCoord so each firefly looks unique
    float rg = 0.85 + 0.15 * gl_PointCoord.x;
    vec3  glowColor = vec3(rg, 1.0, 0.05);

    FragColor = vec4(glowColor, alpha * 0.85 * nightFactor);
}
