#version 450


#include "globals_data.glsl"
#include "utils.glsl"
#include "bindless.glsl"
#include "functions.glsl"

OUT_FRAG_DATA
IN_VERTEX_DATA



INSTANCE_UNIFORM_BUFFER(InstanceBufferObject, {
    vec4 diffuse_color;
    vec4 emissive_color;
    vec2 tilling;
    vec2 metallic;
    vec2 roughness;
    float height_scale;
});

BINDLESS_COMBINED_SAMPLER2D;


#define GBUFFER_FRAG 1

layout(std140, set = 0, binding = 0)uniform SceneBufferObject{
    mat4 _projection;
    mat4 _view;
    vec3 _view_position;
};


void main()
{
    INSTANCE_TEXTURE_INDEX(DIFFUSE_MAP, 0);
    INSTANCE_TEXTURE_INDEX(NORMAL_MAP, 1);
    INSTANCE_TEXTURE_INDEX(METALLIC_MAP, 2);
    INSTANCE_TEXTURE_INDEX(ROUGHNESS_MAP, 3);
    INSTANCE_TEXTURE_INDEX(HEIGHT_MAP, 4);
    INSTANCE_TEXTURE_INDEX(OCCLUSION_MAP, 5);


    vec3 obj_norm = normalize(_normal_); 
    vec3 _tangent = normalize( _tangent_.xyz - (dot(_tangent_.xyz, obj_norm) * obj_norm) ); 
    vec3 _bitangent = cross(obj_norm, _tangent) * _tangent_.w; 
    mat3 TBN = mat3(_tangent, _bitangent, obj_norm); 
    mat3 world_TBN = mat3(transpose(_view)) * TBN;
    mat3 inv_TBN = transpose(world_TBN);

    vec3 actual_world_position = world_position;
    vec3 actual_world_view_dir = _view_position - world_position;

    vec3 tangent_space_pos = inv_TBN * actual_world_position;
    vec3 tangent_space_view_pos = inv_TBN * _view_position;
    vec3 tangent_space_view_dir = normalize( (tangent_space_view_pos - tangent_space_pos));

    
    vec2 parallax_tex_coord = _texCoord * GET_INSTANCE_PARAM(tilling, InstanceBufferObject);
    parallax_tex_coord = ParallaxMapping(GET_BINDLESS_TEXTURE2D(HEIGHT_MAP), GET_INSTANCE_PARAM(height_scale, InstanceBufferObject), parallax_tex_coord, tangent_space_view_dir);

    vec3 normal;
    vec3 _n = texture(GET_BINDLESS_TEXTURE2D(NORMAL_MAP), parallax_tex_coord).rgb;  
    _n = _n * 2.0f - 1.0f;
    normal = normalize(TBN * _n);


    vec4 color = texture(GET_BINDLESS_TEXTURE2D(DIFFUSE_MAP), parallax_tex_coord);
    vec4 diff_color = GET_INSTANCE_PARAM(diffuse_color, InstanceBufferObject);
    //diff_color.rgb = pow(diff_color.rgb, vec3(2.2f));
    vec4 final_color = mix(color, diff_color, diff_color.a);

    FRAG_POS = _fragPos;
    FRAG_NORMAL = normal;

    FRAG_NORMAL_W = texture(GET_BINDLESS_TEXTURE2D(OCCLUSION_MAP), parallax_tex_coord).r;

    uint color_compressed = vec4ToUint32(final_color);
    FRAG_COLOR_R = color_compressed;

    float metal = texture(GET_BINDLESS_TEXTURE2D(METALLIC_MAP), parallax_tex_coord).r;
    float rough = texture(GET_BINDLESS_TEXTURE2D(ROUGHNESS_MAP), parallax_tex_coord).r;

    vec2 metallic = GET_INSTANCE_PARAM(metallic, InstanceBufferObject);
    vec2 roughness = GET_INSTANCE_PARAM(roughness, InstanceBufferObject);

    metal = mix(metal, metallic.x, metallic.y);
    rough = mix(rough, roughness.x, roughness.y);

    vec4 surface_data = vec4(metal, rough, 0.0f, 0.0f);
    uint surface_data_compressed = vec4ToUint32(surface_data);
    FRAG_COLOR_G = surface_data_compressed;

}
