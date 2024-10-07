#version 450



#define G_POSITION 0
#define G_NORMAL 1
#define G_EMISSION 2
#define MAX_LIGHT_COUNT 15
#define MAX_SPECULAR_VALUE 256.0f


layout(location = 0)out vec4 FragColor;

layout(location = 0)in vec2 in_texCoord;

layout(std140, set = 0, binding = 0)uniform SceneBufferObject{
    mat4 _view;
    mat4 _inv_view;
    int _ssao_dat;
};

layout(set = 0, binding = 1)uniform sampler2D g_bufferData[3];
layout(set = 0, binding = 2)uniform usampler2D g_bufferColor;
layout(set = 0, binding = 3)uniform sampler2D ssao_blur;


#include "functions.glsl"


struct LightData
{
    vec4 position;
    vec4 direction;
    vec4 color;
    vec4 params1;
    vec4 params2;
};

layout(set = 0, binding = 4)uniform ShadowdedLights {

    uint num_shadowed_sun_lights;
	uint num_non_shadowed_sun_lights;
    LightData sun_lights[MAX_LIGHT_COUNT];

    uint num_shadowed_spot_lights;
	uint num_non_shadowed_spot_lights;
	LightData spot_lights[MAX_LIGHT_COUNT];

    uint num_shadowed_point_lights;
	uint num_non_shadowed_point_lights;
	LightData point_lights[MAX_LIGHT_COUNT];
};

layout(set = 0, binding = 5)uniform sampler2D sun_shadow_maps[MAX_SHADOW_SUN_LIGHTS];
layout(set = 0, binding = 6)uniform sampler2D spot_shadow_maps[MAX_SHADOW_SPOT_LIGHTS];

layout(set = 0, binding = 7)uniform LightViewProj {
    mat4 sun_shadow_view_proj[MAX_SHADOW_SUN_LIGHTS];
    mat4 spot_shadow_view_proj[MAX_SHADOW_SPOT_LIGHTS];
};


const float ambeint_factor = 0.05f;

vec4 calculate_directional_light(vec3 normal, vec3 view_direction, vec3 albedo, LightData light_data, float metallic, float roughness,float ssao);
vec4 calculate_point_light(vec3 normal, vec3 view_direction, vec3 albedo, vec3 position,LightData light_data, float metallic, float roughness, float ssao);
vec4 calculate_spot_light(vec3 normal, vec3 view_direction, vec3 albedo, vec3 position, LightData light_data, float metallic, float roughness, float ssao);

void main()
{
    vec4 pData = texture(g_bufferData[G_POSITION], in_texCoord);
    vec3 frag_pos = pData.xyz;
    vec4 nData = texture(g_bufferData[G_NORMAL], in_texCoord);
    vec4 emission_Data = texture(g_bufferData[G_EMISSION], in_texCoord);

    vec3 normal = nData.xyz;
    uvec4 cData = texture(g_bufferColor, in_texCoord);

    uint color_compressed = cData.r;
    uint surface_data_compressed = cData.g;

    vec4 real_color = colorFromUint32(color_compressed);
    vec4 surface_data = colorFromUint32(surface_data_compressed);

    vec3 view_dir = normalize(-frag_pos);
    vec3 albedo = real_color.rgb;
    vec4 final_color = vec4(0.0f);
    float ambeint_occlusion = nData.w;
    emission_Data.w = 0.0f;

    vec4 world_position = _inv_view * vec4(frag_pos, 1.0f);


    float ssao_res = 1.0f;
    if(_ssao_dat == 1)
    {
        ssao_res = texture(ssao_blur, in_texCoord).r;
    }

    float metal = surface_data.r;
    float rough = surface_data.g;

    mat3 view_mat = mat3(_view);

    for(uint i = 0; i < num_shadowed_sun_lights; i++)
    {
        LightData light = sun_lights[i];
        float N_dot_L = max(dot(normal, -(view_mat * light.direction.xyz)), 0.0f);
        float bias = mix(SHADOW_BIAS, 0.0f, N_dot_L);
        vec4 dir_c = calculate_directional_light(normal, view_dir, albedo, light, metal, rough, ssao_res);

        vec4 final_light_position = sun_shadow_view_proj[i] * world_position;
        final_light_position.xyz /= final_light_position.w;

        vec2 tex_pos = final_light_position.xy * 0.5 + 0.5;

        dir_c *= ShadowPCF(sun_shadow_maps[i], tex_pos, 2, final_light_position.z, bias);

        
        final_color += dir_c;
    }

    for(uint i = num_shadowed_sun_lights; i < num_non_shadowed_sun_lights; i++)
    {
        LightData light = sun_lights[i];
        vec4 dir_c = calculate_directional_light(normal, view_dir, albedo, light, metal, rough, ssao_res);
        final_color += dir_c;
    }

    for(uint i = 0; i < num_shadowed_point_lights; i++)
    {
        LightData light = point_lights[i];
        vec4 p_c = calculate_point_light(normal, view_dir, albedo, frag_pos, light, metal, rough, ssao_res);
        final_color += p_c;
    }

    for(uint i = num_shadowed_point_lights; i < num_non_shadowed_point_lights; i++)
    {
        LightData light = point_lights[i];
        vec4 p_c = calculate_point_light(normal, view_dir, albedo, frag_pos, light, metal, rough, ssao_res);
        final_color += p_c;
    }

    for(uint i = 0; i < num_shadowed_spot_lights; i++)
    {
        LightData light = spot_lights[i];
        float N_dot_L = max(dot(normal, -(view_mat * light.direction.xyz)), 0.0f);
        float bias = mix(SHADOW_BIAS, 0.0f, N_dot_L);
        vec4 s_c = calculate_spot_light(normal, view_dir, albedo, frag_pos, light, metal, rough, ssao_res);

        vec4 final_light_position = spot_shadow_view_proj[i] * world_position;
        final_light_position.xyz /= final_light_position.w;

        vec2 tex_pos = final_light_position.xy * 0.5 + 0.5;

        s_c *= (ShadowPCF(spot_shadow_maps[i], tex_pos, 2, final_light_position.z, bias));

        final_color += s_c;
    }

    for(uint i = num_shadowed_spot_lights; i < num_non_shadowed_spot_lights; i++)
    {
        LightData light = spot_lights[i];
        vec4 s_c = calculate_spot_light(normal, view_dir, albedo, frag_pos, light, metal, rough, ssao_res);
        final_color += s_c;
    }

    vec4 ambient = (vec4( (ambeint_factor * albedo).rgb, 1.0f ) * ambeint_occlusion) * ssao_res;
    final_color += ambient;

    FragColor = emission_Data + final_color;

}


vec4 calculate_directional_light(vec3 normal, vec3 view_direction, vec3 albedo, LightData light_data, float metallic, float roughness, float ssao)
{
    mat3 view_mat = mat3(_view);
    vec3 light_direction = normalize(view_mat * light_data.direction.xyz);
    float _lgt_intensity = light_data.params2.y;
    vec4 _lgt_color = light_data.color * _lgt_intensity;
    


    vec3 radiance = _lgt_color.rgb;

    vec3 out_color = CookTorrance_BRDF(normal, (-light_direction), view_direction, radiance, albedo, metallic, roughness);

    return vec4(out_color, 1.0f);//( (ambient + diffuse) * _lgt_color + specular);

}

vec4 calculate_point_light(vec3 normal, vec3 view_direction, vec3 albedo, vec3 position, LightData light_data, float metallic, float roughness, float ssao)
{
    vec3 light_pos =  ( _view * vec4(light_data.position.xyz, 1.0f)).xyz;
    vec3 light_dir;
    float _lgt_intensity = light_data.params2.y;
    vec4 _lgt_color = light_data.color * _lgt_intensity;
    float _lgt_constant = light_data.params1.x;
    float _lgt_linear = light_data.params1.y;
    float _lgt_quadratic = light_data.params1.z;

    float distance = length(light_pos - position);
    float attenuation = 1 / ( _lgt_constant + (_lgt_linear * distance) + (_lgt_quadratic * (distance * distance)) );
    light_dir = normalize(light_pos - position);

    vec3 radiance = vec4(_lgt_color * attenuation).rgb;

    vec3 out_color = CookTorrance_BRDF(normal, (light_dir), view_direction, radiance, albedo, metallic, roughness);

    return vec4(out_color, 1.0f);
}

vec4 calculate_spot_light(vec3 normal, vec3 view_direction, vec3 albedo, vec3 position, LightData light_data, float metallic, float roughness, float ssao)
{
    vec3 light_pos =  ( _view * vec4(light_data.position.xyz, 1.0f)).xyz;
    mat3 view_mat = mat3(_view);
    vec3 light_direction = normalize(view_mat * light_data.direction.xyz);
    vec3 light_dir = light_pos - position;
    float _lgt_intensity = light_data.params2.y;
    vec4 _lgt_color = light_data.color * _lgt_intensity;
    float _lgt_innerCutOff = light_data.params1.w;
    float _lgt_outerCutOff = light_data.params2.x;
    light_dir = normalize(light_dir);
    float theta = max(dot(light_dir, -light_direction), 0.0f);
    float NdotL = max(dot(normal, -light_direction), 0.0f);
    
    if(theta > _lgt_outerCutOff)
    {
        float epslion = _lgt_innerCutOff - _lgt_outerCutOff;
        float intensity = clamp(( theta - _lgt_outerCutOff ) / epslion, 0.0, 1.0);
    

        vec3 radiance = vec4(_lgt_color * intensity).rgb;

        vec3 out_color = CookTorrance_BRDF(normal, (light_dir), view_direction, radiance, albedo, metallic, roughness);
        
        return vec4(out_color, 1.0f);//( (ambient + diffuse) * _lgt_color + specular) ;

    }

    return vec4(0.0, 0.0, 0.0, 0.0);

}