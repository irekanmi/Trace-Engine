#version 450

layout(location = 0)in vec3 in_pos;
layout(location = 1)in vec3 in_normal;
layout(location = 2)in vec2 in_texCoord;
layout(location = 3)in vec4 in_tangent;

layout(location = 0)out vec3 out_texCoord;

layout( set = 0, binding = 0)uniform SceneBufferObject{
    mat4 projection;
    mat4 view;
    vec3 view_position;
    vec2 _test;
} scene_globals;

void main()
{
    out_texCoord = in_pos;
    vec3 position = mat3(scene_globals.view) * in_pos;
    vec4 out_pos = scene_globals.projection * vec4(position, 1.0f);
    gl_Position = out_pos.xyww;
}
