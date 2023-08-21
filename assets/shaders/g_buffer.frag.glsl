#version 450


#define DIFFUSE_MAP 0
#define SPECULAR_MAP 1
#define NORMAL_MAP 2
#define MAX_LIGHT_COUNT 4

layout(location = 0)out vec4 g_Position;
layout(location = 1)out vec4 g_Normal;
layout(location = 2)out vec4 g_ColorSpecular;

layout(location = 0)in data_object
{
    vec3 _normal;
    vec3 _fragPos;
    vec3 _view_position;
    vec2 _texCoord;
    vec4 _tangent;
    flat ivec4 light_data;
} _data;


layout(set = 1, binding = 0)uniform InstanceBufferObject{
    vec4 diffuse_color;
    float shininess;
};
layout(set = 1, binding = 1)uniform sampler2D testing[3];


void main()
{
    vec3 _normal = texture(testing[NORMAL_MAP], _data._texCoord).rgb;
    _normal = _normal * 2.0f - 1.0f;
    
    vec3 obj_norm = normalize(_data._normal);
    vec3 _tangent = normalize( _data._tangent.xyz - (dot(_data._tangent.xyz, obj_norm) * obj_norm) );
    vec3 _bitangent = cross(obj_norm, _tangent) * _data._tangent.w;
    mat3 TBN = mat3(_tangent, _bitangent, obj_norm);
    vec3 normal = normalize(TBN * _normal);


    g_Position = vec4(_data._fragPos, shininess);
    g_Normal = vec4(normal, 1.0f);
    g_ColorSpecular = vec4(texture(testing[DIFFUSE_MAP], _data._texCoord).rgb, texture(testing[SPECULAR_MAP], _data._texCoord).r);

}