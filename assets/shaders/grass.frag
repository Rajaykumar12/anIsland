#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

void main()
{
    // A nice, vibrant grass green
    FragColor = vec4(0.2, 0.8, 0.2, 1.0);
    
    // If you had a grass texture with transparency, you would do:
    // vec4 texColor = texture(grassTexture, TexCoords);
    // if(texColor.a < 0.1) discard;
    // FragColor = texColor;
}