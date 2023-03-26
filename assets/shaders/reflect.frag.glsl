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

layout(set = 0, binding = 1) uniform samplerCube CubeMap;
layout(set = 1, binding = 1) uniform sampler2D normal_map;

layout(set = 0, binding = 2)uniform Testing{
    ivec4 rest;
};

void main()
{

    vec3 _normal = texture(normal_map, _data._texCoord).rgb;
    _normal = 2.0 * _normal - 1.0f;
    
    vec3 obj_norm = normalize(_data._normal);
    vec3 _tangent = normalize( _data._tangent.xyz - (dot(_data._tangent.xyz, obj_norm) * obj_norm) );
    vec3 _bitangent = cross(obj_norm, _tangent) * _data._tangent.w;
    mat3 TBN = mat3(_tangent, _bitangent, obj_norm);


    _normal = normalize(TBN * _normal);
    
    vec3 view_dir = _data._view_position - _data._fragPos;
    vec3 reflect_dir = reflect(-view_dir, _normal);
    float refract_ratio = 1.0/1.52;
    vec3 refract_dir = refract(-view_dir, _normal, refract_ratio);
    
    if(rest.x == 0)
    {
        FragColor = vec4(_data._fragPos, 1.0);
    }
   else if(rest.x == 1)
   {
        FragColor = vec4(abs(_normal), 1.0);
        return;
   }
   else if(rest.x == 2)
   {
        FragColor = texture(CubeMap, refract_dir);
        return;
   }
   else if(rest.x == 3)
   {
        FragColor = texture(CubeMap, reflect_dir);
        return;
   }
   else if(rest.x == 4)
   {
        FragColor = texture(CubeMap, mix(reflect_dir, refract_dir, refract_ratio));
        return;
   }
   else if(rest.x == 5)
   {
        FragColor = texture(CubeMap, refract_dir);
        FragColor += texture(CubeMap, reflect_dir);
        return;
   }

}