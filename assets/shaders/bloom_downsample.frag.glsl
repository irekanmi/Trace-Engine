#version 450

#include "functions.glsl"

layout(location = 0)out vec4 FragColor;

layout(location = 0)in vec2 in_texCoord;

layout(set = 0, binding = 0)uniform sampler2D u_srcTexture;


void main()
{
    FragColor = sampleTexture13Tap(u_srcTexture, in_texCoord);
}
