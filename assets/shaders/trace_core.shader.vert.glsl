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
    flat ivec4 light_data;
} _data;

layout(std140, set = 0, binding = 0)uniform SceneBufferObject{
    mat4 projection;
    mat4 view;
    vec3 view_position;
    ivec4 light_data;
} scene_globals;



layout( push_constant )uniform LocalBufferObject{
        mat4 model;
} local_data;



void main()
{
    _data._texCoord = in_texCoord;
    _data.light_data = scene_globals.light_data;

    mat3 model_mat3 = mat3(scene_globals.view * local_data.model);
    mat3 norm_mat3 = transpose(inverse(model_mat3));
    _data._normal = normalize(norm_mat3  * in_normal);
    _data._tangent = vec4( normalize(model_mat3 * in_tangent.xyz), in_tangent.w );
    _data._fragPos = (scene_globals.view * local_data.model * vec4(in_pos, 1.0f)).xyz;
    _data._view_position = scene_globals.view_position;

    

    
    
    gl_Position = scene_globals.projection * scene_globals.view * local_data.model * vec4(in_pos, 1.0f);
}