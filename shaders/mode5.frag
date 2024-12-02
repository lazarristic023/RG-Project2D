#version 330 core

in vec3 channelCol; // Input color from vertex shader
in float alpha;     // Input transparency from vertex shader

out vec4 outCol; // Final output color

void main() 
{
    outCol = vec4(channelCol, alpha); // Set color with transparency
}
