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
    vec4 camera_position;
};



struct VertexData{
    vec4 positions[MAX_QUAD_INSTANCE];
    vec4 colors[MAX_QUAD_INSTANCE];
    vec4 scale[MAX_QUAD_INSTANCE];
    mat4 model;
};

layout(std140, set = 1, binding = 3) readonly buffer DrawData{
    VertexData objects[];
};

layout(location = 2) out Data{
    vec4 color;
};


void main()
{
    out_texCoord = in_texCoord;
    color = objects[binding_index.draw_instance_index.x].colors[gl_InstanceIndex];

    vec3 pos = objects[binding_index.draw_instance_index.x].positions[gl_InstanceIndex].xyz;
    vec3 scale = objects[binding_index.draw_instance_index.x].scale[gl_InstanceIndex].xyz;
    vec3 cam_pos = camera_position.xyz;
    
    mat4 transform = PositionLookAt(cam_pos - pos, scale);
    transform[3] = vec4(pos, 1.0f);

    mat4 local_pose = objects[binding_index.draw_instance_index.x].model;

    gl_Position = _projection * local_pose * transform * vec4(in_pos, 1.0f);
}