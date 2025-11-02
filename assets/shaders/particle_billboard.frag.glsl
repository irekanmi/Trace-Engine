#version 450

#include "OIT_data.glsl"
#include "bindless.glsl"
#include "functions.glsl"

#define MAX_QUAD_TEXTURE_SLOT 16

// layout(location = 0)out vec4 FragColor;
OUT_OIT_DATA

layout(location = 0)in vec2 in_texCoord;

BINDLESS_COMBINED_SAMPLER2D;


layout(location = 2) in Data{
    vec3 position;
    vec3 color;
    vec3 scale;
    float lifetime;
};

void main()
{
    INSTANCE_TEXTURE_INDEX(u_textures, 0);

    vec4 image_color = texture(GET_BINDLESS_TEXTURE2D(u_textures), in_texCoord);
    float alpha = image_color.a;
    vec4 out_color = image_color * vec4(color, 1.0f);
    out_color.a = alpha;
    //FragColor = out_color;
    PROCESS_COLOR(out_color);
}