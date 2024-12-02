#version 330 core

layout(location = 0) in vec2 inPos; // Input vertex position
layout(location = 1) in vec3 inCol; // Input vertex color

uniform float uTime; // Time value for smooth movement

out vec3 channelCol; // Output color for the fragment shader
out float alpha;     // Output alpha for transparency

uniform float offsetX;

void main() 
{
    // Compute the x-offset using cosine for smooth back-and-forth movement
    //float offsetX = cos(uTime) * 0.8; 

    // Set the new position of the vertex
    gl_Position = vec4(inPos.x + offsetX, inPos.y, 0.0, 1.0);

    // Calculate transparency based on distance from the center
    float edgeProximity = abs(gl_Position.x); // Closer to the edge increases proximity
    alpha = 1.0 - edgeProximity;             // Alpha decreases as proximity increases

    // Ensure alpha stays within [0.2, 1.0] range for visibility
    alpha = clamp(alpha, 0.05, 1.0);

    channelCol = inCol; // Pass color to the fragment shader
}
