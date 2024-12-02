#version 330 core

in vec3 channelCol;
out vec4 FragColor;
uniform float transparency; // Alpha value controlled by temperature

void main() {
    FragColor = vec4(channelCol, transparency); // Blue color with varying alpha
}