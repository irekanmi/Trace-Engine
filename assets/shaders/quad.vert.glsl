#version 450

#include "globals_data.glsl"
#include "bindless.glsl"


DEFAULT_VERTEX_INPUT

#define MAX_QUAD_VERTICES 512

layout(location = 0)out vec2 out_texCoord;

layout(set = 0, binding = 0)uniform SceneData
{
    mat4 _projection;
};



struct VertexData{
    mat4 transforms[MAX_QUAD_VERTICES];
    uint colors[MAX_QUAD_VERTICES];
};

layout(std140, set = 1, binding = 3) readonly buffer DrawData{
    VertexData objects[];
};

layout(location = 2) out Data{
    uint color;
};


void main()
{
    out_texCoord = in_texCoord;
    color = objects[binding_index.draw_instance_index.x].colors[gl_InstanceIndex];
    gl_Position = _projection * objects[binding_index.draw_instance_index.x].transforms[gl_InstanceIndex] * vec4(in_pos, 1.0f);
}