#version 450

#define G_POSITION 0
#define G_NORMAL 1
#define G_COLOR 2
#define MAX_LIGHT_COUNT 4
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
};
layout(set = 0, binding = 3)uniform Lights {
    vec4 position;
    vec4 direction;
    vec4 color;
    vec4 params1;
    vec4 params2;
} u_gLights[MAX_LIGHT_COUNT];

struct Light
{
    vec3 position;
    vec3 direction;
    vec4 color;
    float constant;
    float linear;
    float quadratic;
    float innerCutOff;
    float outerCutOff;
};

const float ambeint_factor = 0.005;

vec4 calculate_directional_light(Light light, vec3 normal, vec3 view_direction, float spec, vec3 albedo, float shine);
vec4 calculate_point_light(Light light, vec3 normal, vec3 view_direction, float spec, vec3 albedo, vec3 position,float shine);
vec4 calculate_spot_light(Light light, vec3 normal, vec3 view_direction, float spec, vec3 albedo, vec3 position, float shine);

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
        Light _lgt;
        for(int i = 0; i < scene_globals.light_data.x; i++)
        {
            _lgt.position = u_gLights[i].position.xyz;
            _lgt.direction = u_gLights[i].direction.xyz;
            _lgt.color = u_gLights[i].color;
            _lgt.constant = u_gLights[i].params1.x;
            _lgt.linear = u_gLights[i].params1.y;
            _lgt.quadratic = u_gLights[i].params1.z;
            _lgt.innerCutOff = u_gLights[i].params1.w;
            _lgt.outerCutOff = u_gLights[i].params2.x;
            FragColor += calculate_directional_light(_lgt, normal, view_dir, specular, albedo, shine);
        }
   }
   else if(rest.x == 3)
   {
        Light _lgt;
        int num = scene_globals.light_data.x + scene_globals.light_data.y;
        for(int i = scene_globals.light_data.x; i < num; i++)
        {
            _lgt.position = u_gLights[i].position.xyz;
            _lgt.direction = u_gLights[i].direction.xyz;
            _lgt.color = u_gLights[i].color;
            _lgt.constant = u_gLights[i].params1.x;
            _lgt.linear = u_gLights[i].params1.y;
            _lgt.quadratic = u_gLights[i].params1.z;
            _lgt.innerCutOff = u_gLights[i].params1.w;
            _lgt.outerCutOff = u_gLights[i].params2.x;
            FragColor += calculate_point_light(_lgt, normal, view_dir, specular, albedo, frag_pos, shine);
        }
   }
   else if(rest.x == 4)
   {
        Light _lgt;
        int num = scene_globals.light_data.x + scene_globals.light_data.y + scene_globals.light_data.z;
        for(int i = scene_globals.light_data.x + scene_globals.light_data.y; i < num; i++)
        {
            _lgt.position = u_gLights[i].position.xyz;
            _lgt.direction = u_gLights[i].direction.xyz;
            _lgt.color = u_gLights[i].color;
            _lgt.constant = u_gLights[i].params1.x;
            _lgt.linear = u_gLights[i].params1.y;
            _lgt.quadratic = u_gLights[i].params1.z;
            _lgt.innerCutOff = u_gLights[i].params1.w;
            _lgt.outerCutOff = u_gLights[i].params2.x;
            FragColor += calculate_spot_light(_lgt, normal, view_dir, specular, albedo, frag_pos, shine);
        }
   }
   else if(rest.x == 5)
   {
        Light _lgt;
        for(int i = 0; i < scene_globals.light_data.x; i++)
        {
            _lgt.position = u_gLights[i].position.xyz;
            _lgt.direction = u_gLights[i].direction.xyz;
            _lgt.color = u_gLights[i].color;
            _lgt.constant = u_gLights[i].params1.x;
            _lgt.linear = u_gLights[i].params1.y;
            _lgt.quadratic = u_gLights[i].params1.z;
            _lgt.innerCutOff = u_gLights[i].params1.w;
            _lgt.outerCutOff = u_gLights[i].params2.x;
            FragColor += calculate_directional_light(_lgt, normal, view_dir, specular, albedo, shine);
        }

        int num = scene_globals.light_data.x + scene_globals.light_data.y;
        for(int i = scene_globals.light_data.x; i < num; i++)
        {
            _lgt.position = u_gLights[i].position.xyz;
            _lgt.direction = u_gLights[i].direction.xyz;
            _lgt.color = u_gLights[i].color;
            _lgt.constant = u_gLights[i].params1.x;
            _lgt.linear = u_gLights[i].params1.y;
            _lgt.quadratic = u_gLights[i].params1.z;
            _lgt.innerCutOff = u_gLights[i].params1.w;
            _lgt.outerCutOff = u_gLights[i].params2.x;
            FragColor += calculate_point_light(_lgt, normal, view_dir, specular, albedo, frag_pos, shine);
        }

        num = scene_globals.light_data.x + scene_globals.light_data.y + scene_globals.light_data.z;
        for(int i = scene_globals.light_data.x + scene_globals.light_data.y; i < num; i++)
        {
            _lgt.position = u_gLights[i].position.xyz;
            _lgt.direction = u_gLights[i].direction.xyz;
            _lgt.color = u_gLights[i].color;
            _lgt.constant = u_gLights[i].params1.x;
            _lgt.linear = u_gLights[i].params1.y;
            _lgt.quadratic = u_gLights[i].params1.z;
            _lgt.innerCutOff = u_gLights[i].params1.w;
            _lgt.outerCutOff = u_gLights[i].params2.x;
            FragColor += calculate_spot_light(_lgt, normal, view_dir, specular, albedo, frag_pos, shine);
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


vec4 calculate_directional_light(Light light, vec3 normal, vec3 view_direction, float spec, vec3 albedo, float shine)
{
    vec3 half_direction = normalize(view_direction - (light.direction));   
    float specular_strenght = pow(max(dot(half_direction, normal), 0.0f), shine);
    float diffuse_strenght = max(dot(-light.direction, normal ), 0.0f);

    //vec4 diffuse_samp_color = texture(testing[DIFFUSE_MAP], _data._texCoord);
    //vec4 specular_intensity = vec4((spec), diffuse_samp_color.a);
    vec4 ambient = vec4( (ambeint_factor * albedo).rgb, 1.0f );
    vec4 diffuse = vec4( (albedo * diffuse_strenght ).rgb, 1.0f );
    vec4 specular = vec4( (specular_strenght * light.color).rgb, 1.0f );

    specular *= spec;

    return ( (ambient + diffuse) * light.color + specular);

}

vec4 calculate_point_light(Light light, vec3 normal, vec3 view_direction, float spec, vec3 albedo, vec3 position, float shine)
{
    float distance = length(light.position - position);
    float attenuation = 1 / ( light.constant + (light.linear * distance) + (light.quadratic * (distance * distance)) );
    light.direction = normalize(light.position - position);
   
    vec3 half_direction = normalize(view_direction - (-light.direction));   
    float specular_strenght = pow(max(dot(half_direction, normal), 0.0f), shine);

    float diffuse_strenght = max(dot(light.direction, normal ), 0.0f);


    //vec4 diffuse_samp_color = texture(testing[DIFFUSE_MAP], _data._texCoord);
    //vec4 specular_intensity = vec4((texture(testing[SPECULAR_MAP], _data._texCoord)).rgb, diffuse_samp_color.a);
    vec4 ambient = vec4( (ambeint_factor * albedo).rgb, 1.0f );
    vec4 diffuse = vec4( (albedo * diffuse_strenght ).rgb, 1.0f );
    vec4 specular = vec4( (specular_strenght * light.color).rgb, 1.0f );


    specular *= spec;

    return ( (ambient + diffuse) * light.color + specular) * attenuation;   
}

vec4 calculate_spot_light(Light light, vec3 normal, vec3 view_direction, float spec, vec3 albedo, vec3 position, float shine)
{
    vec3 light_dir = light.position - position;
    light_dir = normalize(light_dir);
    float theta = dot(light_dir, normalize(-light.direction));
    
    //vec4 diffuse_samp_color = texture(testing[DIFFUSE_MAP], _data._texCoord);
    vec4 ambient = vec4( (ambeint_factor * albedo).rgb, 1.0f );
    if(theta > light.outerCutOff)
    {
        float epslion = light.innerCutOff - light.outerCutOff;
        float intensity = clamp(( theta - light.outerCutOff ) / epslion, 0.0, 1.0);
    
        vec3 half_direction = normalize(view_direction - (-light_dir));   
        float specular_strenght = pow(max(dot(half_direction, normal), 0.0f), shine);

    
        float diffuse_strenght = max(dot(light_dir, normal ), 0.0f);

        //vec4 diffuse_samp_color = texture(testing[DIFFUSE_MAP], _data._texCoord);
        //vec4 specular_intensity = vec4((texture(testing[SPECULAR_MAP], _data._texCoord)).rgb, diffuse_samp_color.a);
        vec4 diffuse = vec4( (diffuse_strenght * albedo * intensity).rgb, 1.0f );
        vec4 specular = vec4( (specular_strenght * light.color * intensity ).rgb, 1.0f );


        specular *= spec;
        
        return ( (ambient + diffuse) * light.color + specular);

    }

    return ( ambient);

}