#version 450

layout(location = 0)out vec4 FragColor;

layout(location = 0)in vec3 out_color;

void main()
{
    FragColor = vec4(out_color, 1.0f);
}