#version 450

layout(location = 0)out vec4 FragColor;

layout(location = 0)in data_object
{
    vec3 _normal;
    vec3 _fragPos;
    vec3 _view_position;
    vec2 _texCoord;
    vec4 _tangent;
    flat ivec4 light_data;
} _data;

#define DIFFUSE_MAP 0
#define SPECULAR_MAP 1
#define NORMAL_MAP 2
#define MAX_LIGHT_COUNT 4

layout(set = 1, binding = 1)uniform sampler2D testing[3];
layout(set = 1, binding = 0)uniform InstanceBufferObject{
    vec4 diffuse_color;
    float shininess;
} instance_data;


layout(set = 0, binding = 2)uniform Testing{
    ivec4 rest;
};


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

layout(set = 0, binding = 3)uniform Lights {
    vec4 position;
    vec4 direction;
    vec4 color;
    vec4 params1;
    vec4 params2;
} u_gLights[MAX_LIGHT_COUNT];

const float ambeint_factor = 0.005;




vec4 calculate_directional_light(Light light, vec3 normal, vec3 view_direction);
vec4 calculate_point_light(Light light, vec3 normal, vec3 view_direction);
vec4 calculate_spot_light(Light light, vec3 normal, vec3 view_direction);


void main()
{

    vec3 _normal = texture(testing[NORMAL_MAP], _data._texCoord).rgb;
    _normal = 2.0 * _normal - 1.0f;
    
    vec3 obj_norm = normalize(_data._normal);
    vec3 _tangent = normalize( _data._tangent.xyz - (dot(_data._tangent.xyz, obj_norm) * obj_norm) );
    vec3 _bitangent = cross(obj_norm, _tangent) * _data._tangent.w;
    mat3 TBN = mat3(_tangent, _bitangent, obj_norm);


    _normal = normalize(TBN * _normal);

    vec3 view_dir = _data._view_position - _data._fragPos;

    if(rest.x == 0)
    {
        FragColor = vec4(_data._fragPos, 1.0);
    }
   else if(rest.x == 1)
   {
        FragColor = vec4(abs(_normal), 1.0);
   }
   else if(rest.x == 2)
   {
        Light _lgt;
        for(int i = 0; i < _data.light_data.x; i++)
        {
            _lgt.position = u_gLights[i].position.xyz;
            _lgt.direction = u_gLights[i].direction.xyz;
            _lgt.color = u_gLights[i].color;
            _lgt.constant = u_gLights[i].params1.x;
            _lgt.linear = u_gLights[i].params1.y;
            _lgt.quadratic = u_gLights[i].params1.z;
            _lgt.innerCutOff = u_gLights[i].params1.w;
            _lgt.outerCutOff = u_gLights[i].params2.x;
            FragColor += calculate_directional_light(_lgt, _normal, view_dir);
        }
   }
   else if(rest.x == 3)
   {
        Light _lgt;
        int num = _data.light_data.x + _data.light_data.y;
        for(int i = _data.light_data.x; i < num; i++)
        {
            _lgt.position = u_gLights[i].position.xyz;
            _lgt.direction = u_gLights[i].direction.xyz;
            _lgt.color = u_gLights[i].color;
            _lgt.constant = u_gLights[i].params1.x;
            _lgt.linear = u_gLights[i].params1.y;
            _lgt.quadratic = u_gLights[i].params1.z;
            _lgt.innerCutOff = u_gLights[i].params1.w;
            _lgt.outerCutOff = u_gLights[i].params2.x;
            FragColor += calculate_point_light(_lgt, _normal, view_dir);
        }
   }
   else if(rest.x == 4)
   {
        Light _lgt;
        int num = _data.light_data.x + _data.light_data.y + _data.light_data.z;
        for(int i = _data.light_data.x + _data.light_data.y; i < num; i++)
        {
            _lgt.position = u_gLights[i].position.xyz;
            _lgt.direction = u_gLights[i].direction.xyz;
            _lgt.color = u_gLights[i].color;
            _lgt.constant = u_gLights[i].params1.x;
            _lgt.linear = u_gLights[i].params1.y;
            _lgt.quadratic = u_gLights[i].params1.z;
            _lgt.innerCutOff = u_gLights[i].params1.w;
            _lgt.outerCutOff = u_gLights[i].params2.x;
            FragColor += calculate_spot_light(_lgt, _normal, view_dir);
        }
   }
   else if(rest.x == 5)
   {
        Light _lgt;
        for(int i = 0; i < _data.light_data.x; i++)
        {
            _lgt.position = u_gLights[i].position.xyz;
            _lgt.direction = u_gLights[i].direction.xyz;
            _lgt.color = u_gLights[i].color;
            _lgt.constant = u_gLights[i].params1.x;
            _lgt.linear = u_gLights[i].params1.y;
            _lgt.quadratic = u_gLights[i].params1.z;
            _lgt.innerCutOff = u_gLights[i].params1.w;
            _lgt.outerCutOff = u_gLights[i].params2.x;
            FragColor += calculate_directional_light(_lgt, _normal, view_dir);
        }

        int num = _data.light_data.x + _data.light_data.y;
        for(int i = _data.light_data.x; i < num; i++)
        {
            _lgt.position = u_gLights[i].position.xyz;
            _lgt.direction = u_gLights[i].direction.xyz;
            _lgt.color = u_gLights[i].color;
            _lgt.constant = u_gLights[i].params1.x;
            _lgt.linear = u_gLights[i].params1.y;
            _lgt.quadratic = u_gLights[i].params1.z;
            _lgt.innerCutOff = u_gLights[i].params1.w;
            _lgt.outerCutOff = u_gLights[i].params2.x;
            FragColor += calculate_point_light(_lgt, _normal, view_dir);
        }

        num = _data.light_data.x + _data.light_data.y + _data.light_data.z;
        for(int i = _data.light_data.x + _data.light_data.y; i < num; i++)
        {
            _lgt.position = u_gLights[i].position.xyz;
            _lgt.direction = u_gLights[i].direction.xyz;
            _lgt.color = u_gLights[i].color;
            _lgt.constant = u_gLights[i].params1.x;
            _lgt.linear = u_gLights[i].params1.y;
            _lgt.quadratic = u_gLights[i].params1.z;
            _lgt.innerCutOff = u_gLights[i].params1.w;
            _lgt.outerCutOff = u_gLights[i].params2.x;
            FragColor += calculate_spot_light(_lgt, _normal, view_dir);
        }

   }
}

vec4 calculate_directional_light(Light light, vec3 normal, vec3 view_direction)
{
    normal = normalize(normal);   
    view_direction = normalize(view_direction);
   
    vec3 half_direction = normalize(view_direction - (light.direction));   
    float specular_strenght = pow(max(dot(half_direction, normal), 0.0f), instance_data.shininess);

    

   
    float diffuse_strenght = max(dot(-light.direction, normal ), 0.0f);

    vec4 diffuse_samp_color = texture(testing[DIFFUSE_MAP], _data._texCoord);
    vec4 specular_intensity = vec4((texture(testing[SPECULAR_MAP], _data._texCoord)).rgb, diffuse_samp_color.a);
    vec4 ambient = vec4( (ambeint_factor * diffuse_samp_color).rgb, diffuse_samp_color.a );
    vec4 diffuse = vec4( (diffuse_samp_color * diffuse_strenght ).rgb, diffuse_samp_color.a );
    vec4 specular = vec4( (specular_strenght * light.color).rgb, diffuse_samp_color.a );

    specular *= specular_intensity;

    return ( (ambient + diffuse) * light.color + specular);

}

vec4 calculate_point_light(Light light, vec3 normal, vec3 view_direction)
{
    normal = normalize(normal);   
    view_direction = normalize(view_direction);
    float distance = length(light.position - _data._fragPos);
    float attenuation = 1 / ( light.constant + (light.linear * distance) + (light.quadratic * (distance * distance)) );
    light.direction = normalize(light.position - _data._fragPos);
   
    vec3 half_direction = normalize(view_direction - (-light.direction));   
    float specular_strenght = pow(max(dot(half_direction, normal), 0.0f), instance_data.shininess);

    //vec3 reflect_direction = reflect(-light.direction, normal);
    //float specular_strenght = pow(max(dot(reflect_direction, view_direction), 0.0f), instance_data.shininess);

   
    float diffuse_strenght = max(dot(light.direction, normal ), 0.0f);


    vec4 diffuse_samp_color = texture(testing[DIFFUSE_MAP], _data._texCoord);
    vec4 specular_intensity = vec4((texture(testing[SPECULAR_MAP], _data._texCoord)).rgb, diffuse_samp_color.a);
    vec4 ambient = vec4( (ambeint_factor * diffuse_samp_color).rgb, diffuse_samp_color.a );
    vec4 diffuse = vec4( (diffuse_samp_color * diffuse_strenght ).rgb, diffuse_samp_color.a );
    vec4 specular = vec4( (specular_strenght * light.color).rgb, diffuse_samp_color.a );


    specular *= specular_intensity;

    return ( (ambient + diffuse) * light.color + specular) * attenuation;   
}

vec4 calculate_spot_light(Light light, vec3 normal, vec3 view_direction)
{
    vec3 light_dir = light.position - _data._fragPos;
    light_dir = normalize(light_dir);
    float theta = dot(light_dir, normalize(-light.direction));
    
    vec4 diffuse_samp_color = texture(testing[DIFFUSE_MAP], _data._texCoord);
    vec4 ambient = vec4( (ambeint_factor * diffuse_samp_color).rgb, diffuse_samp_color.a );
    if(theta > light.outerCutOff)
    {
        float epslion = light.innerCutOff - light.outerCutOff;
        float intensity = clamp(( theta - light.outerCutOff ) / epslion, 0.0, 1.0);

        normal = normalize(normal);   
        view_direction = normalize(view_direction);
    
        vec3 half_direction = normalize(view_direction - (-light_dir));   
        float specular_strenght = pow(max(dot(half_direction, normal), 0.0f), instance_data.shininess);

        //vec3 reflect_direction = reflect(light.direction, normal);
        //float specular_strenght = pow(max(dot(reflect_direction, view_direction), 0.0f), instance_data.shininess);

    
        float diffuse_strenght = max(dot(light_dir, normal ), 0.0f);

        vec4 diffuse_samp_color = texture(testing[DIFFUSE_MAP], _data._texCoord);
        vec4 specular_intensity = vec4((texture(testing[SPECULAR_MAP], _data._texCoord)).rgb, diffuse_samp_color.a);
        vec4 diffuse = vec4( (diffuse_strenght * diffuse_samp_color * intensity).rgb, diffuse_samp_color.a );
        vec4 specular = vec4( (specular_strenght * light.color * intensity ).rgb, diffuse_samp_color.a );


        specular *= specular_intensity;
        
        return ( (ambient + diffuse) * light.color + specular);
        //return vec4(1.0f);

    }

    return ( ambient);

}
