#version 450

#include "globals_data.glsl"
#include "bindless.glsl"
#include "functions.glsl"


DEFAULT_VERTEX_INPUT

#define MAX_QUAD_INSTANCE 512

layout(location = 0)out vec2 out_texCoord;

layout(set = 0, binding = 0)uniform SceneData
{
    mat4 _projection;
    vec4 _camera_position;
};



struct VertexData{
    vec4 _positions[MAX_QUAD_INSTANCE];
    vec4 _colors[MAX_QUAD_INSTANCE];
    vec4 _scales[MAX_QUAD_INSTANCE];
    vec4 _rotation[MAX_QUAD_INSTANCE];
    mat4 _model;
};

layout(std140, set = 1, binding = 2) readonly buffer DrawData{
    VertexData objects[];
};

layout(location = 2) out Data{
    vec3 position;
    vec3 color;
    vec3 scale;
    float lifetime;
};


void main()
{
    out_texCoord = in_texCoord;
    

    vec4 pos = objects[binding_index.draw_instance_index.x]._positions[gl_InstanceIndex];
    vec3 col = objects[binding_index.draw_instance_index.x]._colors[gl_InstanceIndex].xyz;
    vec3 scl = objects[binding_index.draw_instance_index.x]._scales[gl_InstanceIndex].xyz;
    vec4 rot = objects[binding_index.draw_instance_index.x]._rotation[gl_InstanceIndex];
    vec3 cam_pos = _camera_position.xyz;

    position = pos.xyz;
    color = col;
    scale = scl;
    lifetime = pos.w;
    
    //mat3 rot_scale = quatToMat3(rot) * mat3(vec3(scale.x, 0.0f, 0.0f,), vec3(0.0f, scale.y, 0.0f), vec3(0.0f, 0.0f, scale.z));


    mat4 transform = PositionLookAt(cam_pos - position, scale);
    transform[3] = vec4(position, 1.0f);

    mat4 local_pose = objects[binding_index.draw_instance_index.x]._model;

    gl_Position = _projection * local_pose * transform * vec4(in_pos, 1.0f);
}