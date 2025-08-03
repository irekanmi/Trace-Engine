#version 450

#include "globals_data.glsl"
#include "bindless.glsl"


DEFAULT_VERTEX_INPUT

#define MAX_QUAD_INSTANCE 512
#define NUM_QUAD_VERT 4


layout(set = 0, binding = 0)uniform SceneData
{
    mat4 _projection;
};



struct VertexData{
    vec4 positions[MAX_QUAD_INSTANCE];// w: color.r
    vec4 tex_coords[MAX_QUAD_INSTANCE]; // z: color.g w: color.b
};

layout(std140, set = 1, binding = 3) readonly buffer DrawData{
    VertexData objects[];
};

layout(location = 2)out TextData
{
    vec3 Color;
    vec2 Tex_coord;
};

void main()
{
    uint index = (gl_InstanceIndex * NUM_QUAD_VERT) + gl_VertexIndex;
    vec4 pos = objects[binding_index.draw_instance_index.x].positions[index];
    vec4 tex_coord = objects[binding_index.draw_instance_index.x].tex_coords[index];
    Color = vec3(pos.w, tex_coord.z, tex_coord.w);
    Tex_coord = tex_coord.xy;
    gl_Position = _projection * vec4(pos.xyz, 1.0);
}