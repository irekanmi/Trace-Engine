#version 450


#include "globals_data.glsl"
#include "utils.glsl"
#include "bindless.glsl"
#include "functions.glsl"

IN_VERTEX_DATA

layout(location = 0)out vec4 out_color;

layout(std140, set = 0, binding = 0)uniform SceneBufferObject{
    mat4 _projection;
    mat4 _view;
    vec3 _view_position;
};


void main()
{
    out_color = vec4(1.0f, 1.0f, 1.0f, 1.0f);
}
