#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform sampler2D heightMap; // Your Perlin noise texture

out vec2 TexCoords;
out float Height; // Pass height to fragment shader for coloring

void main()
{
    TexCoords = aTexCoords;
    
    // Sample height from texture (assuming grayscale, so .r is fine)
    float h = texture(heightMap, aTexCoords).r;
    
    vec3 displacedPos = aPos;
    displacedPos.y = h * 20.0; // Scale the displacement
    Height = displacedPos.y;

    gl_Position = projection * view * model * vec4(displacedPos, 1.0);
}