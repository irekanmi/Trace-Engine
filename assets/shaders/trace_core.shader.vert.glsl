#version 450

layout(location = 0)in vec3 in_pos;
layout(location = 1)in vec3 in_normal;
layout(location = 2)in vec2 in_texCoord;
layout(location = 3)in vec4 in_tangent;

layout(location = 0)out data_object
{
    vec3 _normal;
    vec3 _fragPos;
    vec3 _view_position;
    vec2 _texCoord;
    vec4 _tangent;
    mat3 tbn;
} _data;

layout(std140, set = 0, binding = 0)uniform SceneBufferObject{
    mat4 projection;
    mat4 view;
    vec3 view_position;
    vec2 _test;
} scene_globals;



layout(set = 2, binding = 3)uniform LocalBufferObject{
        mat4 model;
} local_data;

void main()
{
    _data._texCoord = in_texCoord;
    //_data._normal =  (transpose(inverse(local_data.model)) * vec4(in_normal, 0.0f)).xyz;

    mat3 model_mat3 = mat3(local_data.model);
    _data._normal = model_mat3  * in_normal;
    _data._tangent = vec4( normalize(model_mat3 * in_tangent.xyz), in_tangent.w );
    _data._fragPos = (local_data.model * vec4(in_pos, 1.0f)).xyz;
    _data._view_position = scene_globals.view_position;

    vec3 obj_norm = normalize(_data._normal);
    vec3 _tangent = normalize( _data._tangent.xyz - (dot(_data._tangent.xyz, obj_norm) * obj_norm) );
    vec3 _bitangent = cross(obj_norm, _tangent) * _data._tangent.w;
    mat3 TBN = mat3(_tangent, _bitangent, obj_norm);

    _data.tbn = TBN;

    
    
    gl_Position = scene_globals.projection * scene_globals.view * local_data.model * vec4(in_pos, 1.0f);
}