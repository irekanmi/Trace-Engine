#version 450

layout(location = 0)out vec4 FragColor;

layout(location = 0)in vec2 in_texCoord;

layout(set = 0, binding = 0)uniform sampler2D image_dialate;


void main()
{
    vec4 value = texture(image_dialate, in_texCoord);
    FragColor = value;
}