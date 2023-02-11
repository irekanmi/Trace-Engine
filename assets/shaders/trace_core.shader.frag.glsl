#version 450

layout(location = 0)out vec4 FragColor;

layout(location = 0)in vec2 out_tex;

layout(set = 0, binding = 1)uniform sampler2D testing;

void main()
{
    FragColor = texture(testing, out_tex);
}