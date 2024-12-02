#version 330 core

out vec4 outCol;
in vec3 channelCol;

uniform float uA;

void main()
{
	outCol = vec4(channelCol.r, uA, channelCol.b, 1.0);
}