#version 450

#include "bindless.glsl"

#define MAX_QUAD_TEXTURE_SLOT 16

layout(location = 0)out vec4 FragColor;

layout(location = 0)in vec2 in_texCoord;
layout(location = 1)in float in_texIndex;

//layout(set = 0, binding = 1)uniform sampler2D u_textures[MAX_QUAD_TEXTURE_SLOT];
BINDLESS_COMBINED_SAMPLER2D;

void main()
{
    INSTANCE_TEXTURE_INDEX(u_textures, 0);

    vec4 color = texture(GET_BINDLESS_TEXTURE2D(u_textures), in_texCoord);
    FragColor = color;
}