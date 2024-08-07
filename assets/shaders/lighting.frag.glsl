#version 450

#include "functions.glsl"

#define G_POSITION 0
#define G_NORMAL 1
#define G_COLOR 2
#define MAX_LIGHT_COUNT 15
#define MAX_SPECULAR_VALUE 256.0f


layout(location = 0)out vec4 FragColor;

layout(location = 0)in vec2 in_texCoord;

layout(std140, set = 0, binding = 0)uniform SceneBufferObject{
    mat4 _view;
    ivec4 _light_data;
    int _ssao_dat;
};

layout(set = 0, binding = 1)uniform sampler2D g_bufferData[2];
layout(set = 0, binding = 6)uniform usampler2D g_bufferColor;

layout(set = 0, binding = 3)uniform Lights {
    vec4 light_positions[MAX_LIGHT_COUNT];
    vec4 light_directions[MAX_LIGHT_COUNT];
    vec4 light_colors[MAX_LIGHT_COUNT];
    vec4 light_params1s[MAX_LIGHT_COUNT];
    vec4 light_params2s[MAX_LIGHT_COUNT];
};

layout(set = 0, binding = 5)uniform sampler2D ssao_blur;



const float ambeint_factor = 0.005f;

vec4 calculate_directional_light(vec3 normal, vec3 view_direction, vec3 albedo, int index, float metallic, float roughness,float ssao);
vec4 calculate_point_light(vec3 normal, vec3 view_direction, vec3 albedo, vec3 position,int index, float metallic, float roughness, float ssao);
vec4 calculate_spot_light(vec3 normal, vec3 view_direction, vec3 albedo, vec3 position, int index, float metallic, float roughness, float ssao);

void main()
{
    vec4 pData = texture(g_bufferData[G_POSITION], in_texCoord);
    vec3 frag_pos = pData.xyz;
    vec4 nData = texture(g_bufferData[G_NORMAL], in_texCoord);
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


    float ssao_res = 1.0f;
    if(_ssao_dat == 1)
    {
        ssao_res = texture(ssao_blur, in_texCoord).r;
    }

    float metal = surface_data.r;
    float rough = surface_data.g;

    for(int i = 0; i < _light_data.x; i++)
    {
        vec4 dir_c = calculate_directional_light(normal, view_dir, albedo, i, metal, rough, ssao_res);
        final_color += dir_c;
    }

    int num = _light_data.x + _light_data.y;
    for(int i = _light_data.x; i < num; i++)
    {
        vec4 p_c = calculate_point_light(normal, view_dir, albedo, frag_pos, i, metal, rough, ssao_res);
        final_color += p_c;
    }

    num = _light_data.x + _light_data.y + _light_data.z;
    for(int i = _light_data.x + _light_data.y; i < num; i++)
    {
        vec4 s_c = calculate_spot_light(normal, view_dir, albedo, frag_pos, i, metal, rough, ssao_res);
        final_color += s_c;
    }

    vec4 ambient = (vec4( (ambeint_factor * albedo).rgb, 1.0f ) * ambeint_factor) * ssao_res;
    final_color += ambient;

    //final_color.rgb = pow(final_color.rgb, vec3(1.0/2.2));
    FragColor = final_color;

}


vec4 calculate_directional_light(vec3 normal, vec3 view_direction, vec3 albedo, int index, float metallic, float roughness, float ssao)
{
    vec3 light_pos =  ( _view * vec4(light_positions[index].xyz, 1.0f)).xyz;
    mat3 view_mat = mat3(_view);
    vec3 light_direction = normalize(view_mat * light_directions[index].xyz);
    float _lgt_intensity = light_params2s[index].y;
    vec4 _lgt_color = light_colors[index] * _lgt_intensity;
    

    // vec3 half_direction = normalize(view_direction - (light_direction));   
    // float specular_strenght = pow(max(dot(half_direction, normal), 0.0f), shine + 1.0f);
    // float diffuse_strenght = max(dot(-light_direction, normal ), 0.0f);

    //vec4 ambient = vec4( (ambeint_factor * albedo).rgb, 1.0f ) * ssao;
    // vec4 diffuse = vec4( (albedo * diffuse_strenght ).rgb, 1.0f );
    // vec4 specular = vec4( (specular_strenght * _lgt_color).rgb, 1.0f );

    // specular *= spec;

    vec3 radiance = _lgt_color.rgb;

    vec3 out_color = CookTorrance_BRDF(normal, (-light_direction), view_direction, radiance, albedo, metallic, roughness);

    return vec4(out_color, 1.0f);//( (ambient + diffuse) * _lgt_color + specular);

}

vec4 calculate_point_light(vec3 normal, vec3 view_direction, vec3 albedo, vec3 position, int index, float metallic, float roughness, float ssao)
{
    vec3 light_pos =  ( _view * vec4(light_positions[index].xyz, 1.0f)).xyz;
    vec3 light_dir;
    float _lgt_intensity = light_params2s[index].y;
    vec4 _lgt_color = light_colors[index] * _lgt_intensity;
    float _lgt_constant = light_params1s[index].x;
    float _lgt_linear = light_params1s[index].y;
    float _lgt_quadratic = light_params1s[index].z;

    float distance = length(light_pos - position);
    float attenuation = 1 / ( _lgt_constant + (_lgt_linear * distance) + (_lgt_quadratic * (distance * distance)) );
    light_dir = normalize(light_pos - position);
   
    // vec3 half_direction = normalize(view_direction - (-light_dir));   
    // float specular_strenght = pow(max(dot(half_direction, normal), 0.0f), shine + 1.0f);

    // float diffuse_strenght = max(dot(light_dir, normal ), 0.0f);

    // vec4 ambient = vec4( (ambeint_factor * albedo).rgb, 1.0f ) * ssao;
    // vec4 diffuse = vec4( (albedo * diffuse_strenght ).rgb, 1.0f );
    // vec4 specular = vec4( (specular_strenght * _lgt_color).rgb, 1.0f );


    // specular *= spec;

    vec3 radiance = vec4(_lgt_color * attenuation).rgb;

    vec3 out_color = CookTorrance_BRDF(normal, (light_dir), view_direction, radiance, albedo, metallic, roughness);

    return vec4(out_color, 1.0f);//( (ambient + diffuse) * _lgt_color + specular) * attenuation;   
}

vec4 calculate_spot_light(vec3 normal, vec3 view_direction, vec3 albedo, vec3 position, int index, float metallic, float roughness, float ssao)
{
    vec3 light_pos =  ( _view * vec4(light_positions[index].xyz, 1.0f)).xyz;
    mat3 view_mat = mat3(_view);
    vec3 light_direction = normalize(view_mat * light_directions[index].xyz);
    vec3 light_dir = light_pos - position;
    float _lgt_intensity = light_params2s[index].y;
    vec4 _lgt_color = light_colors[index] * _lgt_intensity;
    float _lgt_innerCutOff = light_params1s[index].w;
    float _lgt_outerCutOff = light_params2s[index].x;
    light_dir = normalize(light_dir);
    float theta = max(dot(light_dir, -light_direction), 0.0f);
    float NdotL = max(dot(normal, -light_direction), 0.0f);
    
    vec4 ambient = vec4( (ambeint_factor * albedo).rgb, 1.0f ) * ssao;
    if(theta > _lgt_outerCutOff && NdotL > 0.0f)
    {
        float epslion = _lgt_innerCutOff - _lgt_outerCutOff;
        float intensity = clamp(( theta - _lgt_outerCutOff ) / epslion, 0.0, 1.0);
    
        // vec3 half_direction = normalize(view_direction - (-light_dir));   
        // float specular_strenght = pow(max(dot(half_direction, normal), 0.0f), shine + 1.0f);

    
        // float diffuse_strenght = max(dot(light_dir, normal ), 0.0f);

        // vec4 diffuse = vec4( (diffuse_strenght * albedo * intensity).rgb, 1.0f );
        // vec4 specular = vec4( (specular_strenght * _lgt_color * intensity ).rgb, 1.0f );
        // float _lgt_constant = light_params1s[index].x;
        // float _lgt_linear = light_params1s[index].y;
        // float _lgt_quadratic = light_params1s[index].z;

        // float distance = length(light_pos - position);
        // float attenuation = 1 / ( _lgt_constant + (_lgt_linear * distance) + (_lgt_quadratic * (distance * distance)) );

        // specular *= spec;

        vec3 radiance = vec4(_lgt_color * intensity).rgb;

        vec3 out_color = CookTorrance_BRDF(normal, (light_dir), view_direction, radiance, albedo, metallic, roughness);
        
        return vec4(out_color, 1.0f);//( (ambient + diffuse) * _lgt_color + specular) ;

    }

    return ( ambient);

}