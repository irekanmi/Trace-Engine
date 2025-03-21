#version 450

#include "globals_data.glsl"
#include "bindless.glsl"

DEFAULT_VERTEX_INPUT

layout(std140, set = 0, binding = 0)uniform SceneBufferObject{
    mat4 _view_proj;
};


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
    //gl_Position = _view_proj * GET_INSTANCE_PARAM(_model, LocalBufferObject) * vec4(in_pos, 1.0f);
    gl_Position = _view_proj * objects[binding_index.draw_instance_index.x]._model * vec4(in_pos, 1.0f);
}