#version 450

#include "functions.glsl"


layout(location = 0)out vec4 FragColor;

layout(location = 0)in vec2 in_texCoord;

layout(set = 0, binding = 0)uniform sampler2D u_srcTexture;
layout(set = 0, binding = 1)uniform FilterData{
    float threshold;
};

void main()
{
    FragColor = prefilter_color(sampleTexture13Tap(u_srcTexture, in_texCoord), threshold);
    FragColor.a = 1.0f;
}



