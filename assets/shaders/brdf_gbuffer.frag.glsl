#version 450


#include "globals_data.glsl"
#include "utils.glsl"
#include "bindless.glsl"

OUT_FRAG_DATA
IN_VERTEX_DATA


#include "functions.glsl"

INSTANCE_UNIFORM_BUFFER(InstanceBufferObject, {
    vec4 diffuse_color;
    float metallic;
    float roughness;
});

BINDLESS_COMBINED_SAMPLER2D;


#define GBUFFER_FRAG 1



void main()
{
    INSTANCE_TEXTURE_INDEX(DIFFUSE_MAP, 0);
    INSTANCE_TEXTURE_INDEX(NORMAL_MAP, 1);
    INSTANCE_TEXTURE_INDEX(METALLIC_MAP, 2);
    INSTANCE_TEXTURE_INDEX(ROUGHNESS_MAP, 3);

    vec3 normal;
    SAMPLE_NORMAL_MAP(GET_BINDLESS_TEXTURE2D(NORMAL_MAP), _texCoord, _normal_, _tangent_, normal );

    vec4 color = texture(GET_BINDLESS_TEXTURE2D(DIFFUSE_MAP), _texCoord);
    vec4 diff_color = GET_INSTANCE_PARAM(diffuse_color, InstanceBufferObject);
    diff_color.rgb = pow(diff_color.rgb, vec3(2.2f));
    vec4 final_color = mix(color, diff_color, diff_color.a);

    FRAG_POS = _fragPos;
    FRAG_NORMAL = normal;
    FRAG_NORMAL_W = 1.0f;


    uint color_compressed = vec4ToUint32(final_color);
    FRAG_COLOR_R = color_compressed;

    float metal = texture(GET_BINDLESS_TEXTURE2D(METALLIC_MAP), _texCoord).r;
    float rough = texture(GET_BINDLESS_TEXTURE2D(ROUGHNESS_MAP), _texCoord).r;

    vec4 surface_data = vec4(metal, rough, 0.0f, 0.0f);
    uint surface_data_compressed = vec4ToUint32(surface_data);
    FRAG_COLOR_G = surface_data_compressed;

}
