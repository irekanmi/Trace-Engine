#version 450

#include "bindless.glsl"

#define MAX_QUAD_VERTICES 512

layout(location = 0)out vec2 out_texCoord;
layout(location = 1)out float out_texIndex;

layout(set = 0, binding = 0)uniform SceneData
{
    mat4 _projection;
};


// INSTANCE_UNIFORM_BUFFER(VertexData,
// {
//     vec4 positions[MAX_QUAD_VERTICES];  // xyz : position, w : draw_index
//     vec4 tex_coords[MAX_QUAD_VERTICES]; // x: tex_coord, y : tex_coord, z: tex_index
// }
// );

struct VertexData{
    vec4 positions[MAX_QUAD_VERTICES];  // xyz : position, w : draw_index
    vec4 tex_coords[MAX_QUAD_VERTICES]; // x: tex_coord, y : tex_coord, z: tex_index
};

layout(std140, set = 1, binding = 3) readonly buffer DrawData{
    VertexData objects[];
};

layout(location = 2) out Data{
    uint color;
};


void main()
{
    vec4 current_pos;
    vec4 current_tex;
    //current_pos = GET_INSTANCE_PARAM(positions, VertexData)[gl_VertexIndex];
    current_pos = objects[binding_index.draw_instance_index.x].positions[gl_VertexIndex];
    //current_tex = GET_INSTANCE_PARAM(tex_coords, VertexData)[gl_VertexIndex];
    current_tex = objects[binding_index.draw_instance_index.x].tex_coords[gl_VertexIndex];
    out_texCoord = vec2(current_tex.x, current_tex.y);
    out_texIndex = current_tex.z;
    color = floatBitsToUint(current_pos.a);
    gl_Position = _projection * vec4(current_pos.xyz, 1.0);
}