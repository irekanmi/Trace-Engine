#version 450

#include "functions.glsl"

layout(location = 0)out vec4 FragColor;

layout(location = 0)in vec2 in_texCoord;

layout(set = 0, binding = 0)uniform sampler2D u_srcTexture;
layout(set = 0, binding = 3)uniform SampleData{
    float filterRadius;
};

void main()
{
    FragColor = sampleTent_3x3(u_srcTexture, in_texCoord, filterRadius);
    FragColor.a = 1.0f;

}

