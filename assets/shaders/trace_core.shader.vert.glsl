#version 450

layout(location = 0)in vec3 in_pos;
layout(location = 1)in vec2 in_texCoord;

layout(location = 0)out vec2 out_tex;

layout(std140, set = 0, binding = 0)uniform SceneBufferObject{
    mat4 view;
    mat4 projection;
    vec2 _test;
} scene_globals;

void main()
{
    gl_Position = scene_globals.projection * scene_globals.view * vec4(in_pos, 1.0f);
    out_tex = in_texCoord;
}