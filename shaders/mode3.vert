#version 330 core

layout(location = 0) in vec2 inPos; // Input vertex position
layout(location = 1) in vec3 inCol; // Input vertex color

uniform float uOffsetX; // Horizontal offset from the CPU

out vec3 channelCol; // Output color for the fragment shader

void main() 
{
    // Add offset to the x-coordinate and wrap around if needed
    float newX = inPos.x + uOffsetX;
    if (newX > 1.0) {
        newX -= 2.0; // Wrap to the left if beyond the right edge
    }
    gl_Position = vec4(newX, inPos.y, 0.0, 1.0); // Set transformed position
    channelCol = inCol; // Pass color to the fragment shader
}
