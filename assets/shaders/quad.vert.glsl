#version 450

#define MAX_QUAD_VERTICES 512

layout(location = 0)out vec2 out_texCoord;
layout(location = 1)out float out_texIndex;

layout(set = 0, binding = 0)uniform SceneData
{
    mat4 _projection;
};

// TODO: use SSBO insted of a uniform buffer
layout(set = 1, binding = 3)uniform VertexData
{
    vec4 positions[MAX_QUAD_VERTICES];  // xyz : position, w : draw_index
    vec4 tex_coords[MAX_QUAD_VERTICES]; // x: tex_coord, y : tex_coord, z: tex_index
};


void main()
{
    vec4 current_pos;
    vec4 current_tex;
    current_pos = positions[gl_VertexIndex];
    current_tex = tex_coords[gl_VertexIndex];
    out_texCoord = vec2(current_tex.x, current_tex.y);
    out_texIndex = current_tex.z;
    gl_Position = _projection * vec4(current_pos.xyz, 1.0);
}