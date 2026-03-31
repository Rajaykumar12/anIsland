#version 330 core
out vec4 FragColor;

uniform vec3 fadeColor;
uniform float fadeAlpha;

void main() {
    FragColor = vec4(fadeColor, clamp(fadeAlpha, 0.0, 1.0));
}
