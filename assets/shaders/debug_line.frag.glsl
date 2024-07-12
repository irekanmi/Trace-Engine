#version 450

#include "functions.glsl"

layout(location = 0)out vec4 FragColor;

layout(location = 0) in Data{
    uint color;
};


void main()
{
    float depth = gl_FragCoord.z;
    vec4 out_color = colorFromUint32(color);
    depth = map(depth, 0.998f, 1.0f, 0.00f, 1.0f);
    depth = 1.0f - depth;
    depth = smoothstep(0.01, 0.8, depth);
    out_color.a *= depth;
    FragColor = out_color;
}