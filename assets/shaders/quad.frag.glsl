#version 450

#include "bindless.glsl"
#include "functions.glsl"

#define MAX_QUAD_TEXTURE_SLOT 16

layout(location = 0)out vec4 FragColor;

layout(location = 0)in vec2 in_texCoord;

BINDLESS_COMBINED_SAMPLER2D;


layout(location = 2) in Data{
    uint color;
};

void main()
{
    INSTANCE_TEXTURE_INDEX(u_textures, 0);

    vec4 image_color = texture(GET_BINDLESS_TEXTURE2D(u_textures), in_texCoord);
    vec4 base_color = colorFromUint32(color);
    float alpha = base_color.a;
    vec4 out_color = mix(image_color, base_color, alpha);
    FragColor = out_color;
}