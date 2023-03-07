#version 450

layout(location = 0)out vec4 FragColor;

layout(location = 0)in data_object
{
    vec3 _normal;
    vec3 _fragPos;
    vec3 _view_position;
    vec2 _texCoord;
    vec4 _tangent;
} _data;

#define DIFFUSE_MAP 0
#define SPECULAR_MAP 1
#define NORMAL_MAP 2

layout(set = 1, binding = 1)uniform sampler2D testing[3];
layout(set = 1, binding = 0)uniform InstanceBufferObject{
    vec4 diffuse_color;
    float shininess;
} instance_data;



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

const float ambeint_factor = 0.25f * 0.2;
Light dir_light = {
    { -0.57735, -0.57735, -0.57735 },
    { -0.57735, -0.57735, -0.57735 },
    { 0.8f, 0.8f, 0.8f, 1.0f },
    1.0,
    0.2,
    0.37,
    0.0,
    0.0
};

Light point_light = {
    { 0.0, 2.5, 2.0 },
    { -0.57735, -0.57735, -0.57735 },
    { 0.8f, 0.8f, 0.8f, 1.0f },
    1.0,
    0.22,
    0.20,
    0.0,
    0.0
};

Light spot_light = {
    { 0.0, 2.0, 2.0 },
    { 0.0, 0.0, -1.0f },
    { 0.6f, 0.8f, 0.0f, 1.0f },
    1.0,
    0.07,
    0.017,
    0.939,
    0.866
};

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
    FragColor = calculate_spot_light(spot_light, _normal, view_dir);
    FragColor += calculate_directional_light(dir_light, _normal, view_dir);
    FragColor = calculate_point_light(point_light, _normal, view_dir);
    //FragColor = vec4(abs(_normal), 1.0);
   // FragColor = vec4(vec3(-0.316f, 0.011f, -0.948f), 1.0);
}

vec4 calculate_directional_light(Light light, vec3 normal, vec3 view_direction)
{
    normal = normalize(normal);   
    view_direction = normalize(view_direction);
   
    vec3 half_direction = normalize(view_direction - (light.direction));   
    float specular_strenght = pow(max(dot(half_direction, normal), 0.0f), instance_data.shininess);

    //vec3 reflect_direction = reflect(light.direction, normal);
    //float specular_strenght = pow(max(dot(reflect_direction, view_direction), 0.0f), instance_data.shininess);

   
    float diffuse_strenght = max(dot(-light.direction, normal ), 0.0f);

    vec4 diffuse_samp_color = texture(testing[DIFFUSE_MAP], _data._texCoord);
    vec4 specular_intensity = vec4((texture(testing[SPECULAR_MAP], _data._texCoord)).rgb, diffuse_samp_color.a);
    vec4 ambient = vec4( (ambeint_factor * diffuse_samp_color).rgb, diffuse_samp_color.a );
    vec4 diffuse = vec4( (light.color * diffuse_strenght ).rgb, diffuse_samp_color.a );
    vec4 specular = vec4( (specular_strenght * light.color).rgb, diffuse_samp_color.a );

    ambient *= diffuse_samp_color;
    diffuse *= diffuse_samp_color;
    specular *= specular_intensity;

    return ( ambient + diffuse + specular);

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
    vec4 diffuse = vec4( (light.color * diffuse_strenght ).rgb, diffuse_samp_color.a );
    vec4 specular = vec4( (specular_strenght * light.color).rgb, diffuse_samp_color.a );

    ambient *= diffuse_samp_color;
    diffuse *= diffuse_samp_color;
    specular *= specular_intensity;

    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    return ( ambient + diffuse + specular);   
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
        vec4 diffuse = vec4( (diffuse_strenght * light.color * intensity).rgb, diffuse_samp_color.a );
        vec4 specular = vec4( (specular_strenght * light.color * intensity ).rgb, diffuse_samp_color.a );

    

        ambient *= diffuse_samp_color;
        diffuse *= diffuse_samp_color;
        specular *= specular_intensity;
        
        return ( ambient + diffuse + specular);
        //return vec4(1.0f);

    }

    ambient *= diffuse_samp_color;
    return ( ambient);

}
