#version 450

#include "globals_data.glsl"
#include "bindless.glsl"

DEFAULT_VERTEX_INPUT
OUT_VERTEX_DATA

layout(std140, set = 0, binding = 0)uniform SceneBufferObject{
    mat4 _projection;
    mat4 _view;
    vec4 _time_values;
    vec3 _view_position;
};

// layout( push_constant )uniform LocalBufferObject{
//         mat4 _model;
// } local_data;

// INSTANCE_UNIFORM_BUFFER_SLOT(LocalBufferObject,
// {
//     mat4 _model;
// },
// 5);

struct ObjectData
{
    mat4 _model;
};

layout(std140, set = 1, binding = 0)readonly buffer LocalBufferObject{
    ObjectData objects[];
};


void main()
{
    _texCoord = in_texCoord;

    //mat3 model_view_mat3 = mat3(_view * GET_INSTANCE_PARAM(_model, LocalBufferObject));
    mat3 model_view_mat3 = mat3(_view * objects[binding_index.draw_instance_index.x]._model);
    //mat3 norm_mat3 = transpose(inverse(model_view_mat3));
    _normal_ = normalize(model_view_mat3  * in_normal);
    _tangent_ = vec4( normalize(model_view_mat3 * in_tangent.xyz), in_tangent.w );
    //world_position = (GET_INSTANCE_PARAM(_model, LocalBufferObject) * vec4(in_pos, 1.0f)).xyz;
    world_position = (objects[binding_index.draw_instance_index.x]._model * vec4(in_pos, 1.0f)).xyz;
    _fragPos = (_view * vec4(world_position, 1.0f)).xyz;
    

    _clip_space_pos = _projection * vec4(_fragPos, 1.0f);
    gl_Position = _clip_space_pos;
}