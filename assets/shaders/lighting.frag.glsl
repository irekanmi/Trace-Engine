#version 450

#define G_POSITION 0
#define G_NORMAL 1
#define G_COLOR 2
#define MAX_LIGHT_COUNT 15
#define MAX_SPECULAR_VALUE 256.0f


layout(location = 0)out vec4 FragColor;

layout(location = 0)in vec2 in_texCoord;

layout(std140, set = 0, binding = 0)uniform SceneBufferObject{
    vec3 view_position;
    ivec4 light_data;
} scene_globals;

layout(set = 0, binding = 1)uniform sampler2D g_bufferData[3];

layout(set = 0, binding = 2)uniform DebugData{
    ivec4 rest;
    bool ssao_dat;
};
layout(set = 0, binding = 3)uniform Lights {
    vec4 position;
    vec4 direction;
    vec4 color;
    vec4 params1;
    vec4 params2;
} u_gLights[MAX_LIGHT_COUNT];

layout(set = 0, binding = 4)uniform sampler2D ssao_blur;



const float ambeint_factor = 0.005;

vec4 calculate_directional_light(vec3 normal, vec3 view_direction, float spec, vec3 albedo, float shine, int index, float ssao);
vec4 calculate_point_light(vec3 normal, vec3 view_direction, float spec, vec3 albedo, vec3 position,float shine, int index, float ssao);
vec4 calculate_spot_light(vec3 normal, vec3 view_direction, float spec, vec3 albedo, vec3 position, float shine, int index, float ssao);

void main()
{
    vec4 pData = texture(g_bufferData[G_POSITION], in_texCoord);
    vec3 frag_pos = pData.xyz;
    float shine = pData.w;
    vec3 normal = texture(g_bufferData[G_NORMAL], in_texCoord).xyz;
    vec4 g_color_data = texture(g_bufferData[G_COLOR], in_texCoord);
    vec3 albedo = g_color_data.rgb;
    float specular = g_color_data.a;
    vec3 view_dir = scene_globals.view_position - frag_pos;

    float ssao_res = 1.0f;
    if(ssao_dat)
    {
        ssao_res = texture(ssao_blur, in_texCoord).r;
    }
    

    if(rest.x == 0)
    {
        FragColor = vec4(frag_pos, 1.0);
    }
   else if(rest.x == 1)
   {
        FragColor = vec4(normal, 1.0);
   }
   else if(rest.x == 2)
   {
        for(int i = 0; i < scene_globals.light_data.x; i++)
        {
            FragColor += calculate_directional_light(normal, view_dir, specular, albedo, shine, i, ssao_res);
        }
   }
   else if(rest.x == 3)
   {
        int num = scene_globals.light_data.x + scene_globals.light_data.y;
        for(int i = scene_globals.light_data.x; i < num; i++)
        {
            FragColor += calculate_point_light(normal, view_dir, specular, albedo, frag_pos, shine, i, ssao_res);
        }
   }
   else if(rest.x == 4)
   {
        int num = scene_globals.light_data.x + scene_globals.light_data.y + scene_globals.light_data.z;
        for(int i = scene_globals.light_data.x + scene_globals.light_data.y; i < num; i++)
        {
            FragColor += calculate_spot_light(normal, view_dir, specular, albedo, frag_pos, shine, i, ssao_res);
        }
   }
   else if(rest.x == 5)
   {
        for(int i = 0; i < scene_globals.light_data.x; i++)
        {
            FragColor += calculate_directional_light(normal, view_dir, specular, albedo, shine, i, ssao_res);
        }

        int num = scene_globals.light_data.x + scene_globals.light_data.y;
        for(int i = scene_globals.light_data.x; i < num; i++)
        {
            FragColor += calculate_point_light(normal, view_dir, specular, albedo, frag_pos, shine, i, ssao_res);
        }

        num = scene_globals.light_data.x + scene_globals.light_data.y + scene_globals.light_data.z;
        for(int i = scene_globals.light_data.x + scene_globals.light_data.y; i < num; i++)
        {
            FragColor += calculate_spot_light(normal, view_dir, specular, albedo, frag_pos, shine, i, ssao_res);
        }

   }
   else if(rest.x == 6)
   {
        FragColor = vec4(albedo, 1.0);
   }
   else if(rest.x == 7)
   {
        FragColor = vec4(vec3(specular), 1.0);
   }
}


vec4 calculate_directional_light(vec3 normal, vec3 view_direction, float spec, vec3 albedo, float shine, int index, float ssao)
{
    vec3 light_pos = u_gLights[index].position.xyz;
    vec3 light_direction = u_gLights[index].direction.xyz;
    vec4 _lgt_color = u_gLights[index].color;

    vec3 half_direction = normalize(view_direction - (light_direction));   
    float specular_strenght = pow(max(dot(half_direction, normal), 0.0f), shine);
    float diffuse_strenght = max(dot(-light_direction, normal ), 0.0f);

    vec4 ambient = vec4( (ambeint_factor * albedo).rgb, 1.0f ) * ssao;
    vec4 diffuse = vec4( (albedo * diffuse_strenght ).rgb, 1.0f );
    vec4 specular = vec4( (specular_strenght * _lgt_color).rgb, 1.0f );

    specular *= spec;

    return ( (ambient + diffuse) * _lgt_color + specular);

}

vec4 calculate_point_light(vec3 normal, vec3 view_direction, float spec, vec3 albedo, vec3 position, float shine, int index, float ssao)
{
    vec3 light_pos = u_gLights[index].position.xyz;
    vec3 light_dir;
    vec4 _lgt_color = u_gLights[index].color;
    float _lgt_constant = u_gLights[index].params1.x;
    float _lgt_linear = u_gLights[index].params1.y;
    float _lgt_quadratic = u_gLights[index].params1.z;

    float distance = length(light_pos - position);
    float attenuation = 1 / ( _lgt_constant + (_lgt_linear * distance) + (_lgt_quadratic * (distance * distance)) );
    light_dir = normalize(light_pos - position);
   
    vec3 half_direction = normalize(view_direction - (-light_dir));   
    float specular_strenght = pow(max(dot(half_direction, normal), 0.0f), shine);

    float diffuse_strenght = max(dot(light_dir, normal ), 0.0f);

    vec4 ambient = vec4( (ambeint_factor * albedo).rgb, 1.0f ) * ssao;
    vec4 diffuse = vec4( (albedo * diffuse_strenght ).rgb, 1.0f );
    vec4 specular = vec4( (specular_strenght * _lgt_color).rgb, 1.0f );


    specular *= spec;

    return ( (ambient + diffuse) * _lgt_color + specular) * attenuation;   
}

vec4 calculate_spot_light(vec3 normal, vec3 view_direction, float spec, vec3 albedo, vec3 position, float shine, int index, float ssao)
{
    vec3 light_pos = u_gLights[index].position.xyz;
    vec3 light_direction = u_gLights[index].direction.xyz;
    vec3 light_dir = light_pos - position;
    vec4 _lgt_color = u_gLights[index].color;
    float _lgt_innerCutOff = u_gLights[index].params1.w;
    float _lgt_outerCutOff = u_gLights[index].params2.x;
    light_dir = normalize(light_dir);
    float theta = dot(light_dir, normalize(-light_direction));
    
    vec4 ambient = vec4( (ambeint_factor * albedo).rgb, 1.0f ) * ssao;
    if(theta > _lgt_outerCutOff)
    {
        float epslion = _lgt_innerCutOff - _lgt_outerCutOff;
        float intensity = clamp(( theta - _lgt_outerCutOff ) / epslion, 0.0, 1.0);
    
        vec3 half_direction = normalize(view_direction - (-light_dir));   
        float specular_strenght = pow(max(dot(half_direction, normal), 0.0f), shine);

    
        float diffuse_strenght = max(dot(light_dir, normal ), 0.0f);

        vec4 diffuse = vec4( (diffuse_strenght * albedo * intensity).rgb, 1.0f );
        vec4 specular = vec4( (specular_strenght * _lgt_color * intensity ).rgb, 1.0f );


        specular *= spec;
        
        return ( (ambient + diffuse) * _lgt_color + specular);

    }

    return ( ambient);

}