#version 450

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

layout(set = 0, binding = 1)uniform sampler2D g_bufferData[3];

layout(set = 0, binding = 2)uniform Lights {
    vec4 light_positions[MAX_LIGHT_COUNT];
    vec4 light_directions[MAX_LIGHT_COUNT];
    vec4 light_colors[MAX_LIGHT_COUNT];
    vec4 light_params1s[MAX_LIGHT_COUNT];
    vec4 light_params2s[MAX_LIGHT_COUNT];
};

layout(set = 0, binding = 4)uniform sampler2D ssao_blur;



const float ambeint_factor = 0.005f;

vec4 calculate_directional_light(vec3 normal, vec3 view_direction, float spec, vec3 albedo, float shine, int index, float ssao);
vec4 calculate_point_light(vec3 normal, vec3 view_direction, float spec, vec3 albedo, vec3 position,float shine, int index, float ssao);
vec4 calculate_spot_light(vec3 normal, vec3 view_direction, float spec, vec3 albedo, vec3 position, float shine, int index, float ssao);

void main()
{
    vec4 pData = texture(g_bufferData[G_POSITION], in_texCoord);
    vec3 frag_pos = pData.xyz;
    vec4 nData = texture(g_bufferData[G_NORMAL], in_texCoord);
    vec3 normal = nData.xyz;
    float shine = nData.w;
    vec4 g_color_data = texture(g_bufferData[G_COLOR], in_texCoord);
    vec3 albedo = g_color_data.rgb;
    float specular = g_color_data.a;
    vec3 view_dir = normalize(-frag_pos);

    vec4 final_color = vec4(0.0f);


    float ssao_res = 1.0f;
    if(_ssao_dat == 1)
    {
        ssao_res = texture(ssao_blur, in_texCoord).r;
    }

    for(int i = 0; i < _light_data.x; i++)
    {
        vec4 dir_c = calculate_directional_light(normal, view_dir, specular, albedo, shine, i, ssao_res);
        final_color += dir_c;
    }

    int num = _light_data.x + _light_data.y;
    for(int i = _light_data.x; i < num; i++)
    {
        vec4 p_c = calculate_point_light(normal, view_dir, specular, albedo, frag_pos, shine, i, ssao_res);
        final_color += p_c;
    }

    num = _light_data.x + _light_data.y + _light_data.z;
    for(int i = _light_data.x + _light_data.y; i < num; i++)
    {
        vec4 s_c = calculate_spot_light(normal, view_dir, specular, albedo, frag_pos, shine, i, ssao_res);
        final_color += s_c;
    }

    FragColor = final_color;

}


vec4 calculate_directional_light(vec3 normal, vec3 view_direction, float spec, vec3 albedo, float shine, int index, float ssao)
{
    vec3 light_pos =  ( _view * vec4(light_positions[index].xyz, 1.0f)).xyz;
    mat3 view_mat = mat3(_view);
    vec3 light_direction = normalize(view_mat * light_directions[index].xyz);
    float _lgt_intensity = light_params2s[index].y;
    vec4 _lgt_color = light_colors[index] * _lgt_intensity;
    

    vec3 half_direction = normalize(view_direction - (light_direction));   
    float specular_strenght = pow(max(dot(half_direction, normal), 0.0f), shine + 1.0f);
    float diffuse_strenght = max(dot(-light_direction, normal ), 0.0f);

    vec4 ambient = vec4( (ambeint_factor * albedo).rgb, 1.0f ) * ssao;
    vec4 diffuse = vec4( (albedo * diffuse_strenght ).rgb, 1.0f );
    vec4 specular = vec4( (specular_strenght * _lgt_color).rgb, 1.0f );

    specular *= spec;

    return ( (ambient + diffuse) * _lgt_color + specular);

}

vec4 calculate_point_light(vec3 normal, vec3 view_direction, float spec, vec3 albedo, vec3 position, float shine, int index, float ssao)
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
   
    vec3 half_direction = normalize(view_direction - (-light_dir));   
    float specular_strenght = pow(max(dot(half_direction, normal), 0.0f), shine + 1.0f);

    float diffuse_strenght = max(dot(light_dir, normal ), 0.0f);

    vec4 ambient = vec4( (ambeint_factor * albedo).rgb, 1.0f ) * ssao;
    vec4 diffuse = vec4( (albedo * diffuse_strenght ).rgb, 1.0f );
    vec4 specular = vec4( (specular_strenght * _lgt_color).rgb, 1.0f );


    specular *= spec;

    return ( (ambient + diffuse) * _lgt_color + specular) * attenuation;   
}

vec4 calculate_spot_light(vec3 normal, vec3 view_direction, float spec, vec3 albedo, vec3 position, float shine, int index, float ssao)
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
    float theta = dot(light_dir, -light_direction);
    
    vec4 ambient = vec4( (ambeint_factor * albedo).rgb, 1.0f ) * ssao;
    if(theta > _lgt_outerCutOff)
    {
        float epslion = _lgt_innerCutOff - _lgt_outerCutOff;
        float intensity = clamp(( theta - _lgt_outerCutOff ) / epslion, 0.0, 1.0);
    
        vec3 half_direction = normalize(view_direction - (-light_dir));   
        float specular_strenght = pow(max(dot(half_direction, normal), 0.0f), shine + 1.0f);

    
        float diffuse_strenght = max(dot(light_dir, normal ), 0.0f);

        vec4 diffuse = vec4( (diffuse_strenght * albedo * intensity).rgb, 1.0f );
        vec4 specular = vec4( (specular_strenght * _lgt_color * intensity ).rgb, 1.0f );
        float _lgt_constant = light_params1s[index].x;
        float _lgt_linear = light_params1s[index].y;
        float _lgt_quadratic = light_params1s[index].z;

        float distance = length(light_pos - position);
        float attenuation = 1 / ( _lgt_constant + (_lgt_linear * distance) + (_lgt_quadratic * (distance * distance)) );

        specular *= spec;
        
        return ( (ambient + diffuse) * _lgt_color + specular) ;

    }

    return ( ambient);

}