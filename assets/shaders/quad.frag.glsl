#version 450

#define MAX_QUAD_TEXTURE_SLOT 16

layout(location = 0)out vec4 FragColor;

layout(location = 0)in vec2 in_texCoord;
layout(location = 1)in float in_texIndex;

layout(set = 0, binding = 1)uniform sampler2D u_textures[MAX_QUAD_TEXTURE_SLOT];

void main()
{
    vec4 color = texture(u_textures[int(in_texIndex)], in_texCoord);
    FragColor = color;
}