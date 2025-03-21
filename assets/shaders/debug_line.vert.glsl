#version 450

#include "bindless.glsl"

#define MAX_LINE_VERTICES 1024

layout(set = 0, binding = 0)uniform SceneData
{
    mat4 _projection;
};

layout(location = 0) out Data{
    uint color;
};


// INSTANCE_UNIFORM_BUFFER(VertexData, {
//     vec4 positions[MAX_LINE_VERTICES];
// });

struct PositionData{
    vec4 positions[MAX_LINE_VERTICES];
};

layout(std140, set = 1, binding = 3) readonly buffer VertexData{
    PositionData objects[];
};

void main()
{
    vec4 current_pos;
    //current_pos = GET_INSTANCE_PARAM(positions, VertexData)[gl_VertexIndex];
    current_pos = objects[binding_index.draw_instance_index.x].positions[gl_VertexIndex];
    color = floatBitsToUint(current_pos.a);
    gl_Position = _projection * vec4(current_pos.xyz, 1.0);
}