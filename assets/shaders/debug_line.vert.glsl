#version 450

#define MAX_LINE_VERTICES 1024

layout(set = 0, binding = 0)uniform SceneData
{
    mat4 _projection;
};

// TODO: use SSBO insted of a uniform buffer
layout(set = 1, binding = 3)uniform VertexData
{
    vec4 positions[MAX_LINE_VERTICES];  // xyz : position
};

void main()
{
    vec4 current_pos;
    current_pos = positions[gl_VertexIndex];
    gl_Position = _projection * vec4(current_pos.xyz, 1.0);
}