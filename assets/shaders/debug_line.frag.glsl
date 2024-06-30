#version 450

#include "functions.glsl"

layout(location = 0)out vec4 FragColor;

layout(location = 0) in Data{
    uint color;
};


void main()
{
    vec4 out_color = colorFromUint32(color);
    FragColor = out_color;
}