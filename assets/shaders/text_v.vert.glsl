#version 450

layout(location = 0)in vec3 in_pos;
layout(location = 1)in vec3 in_color;
layout(location = 2)in vec2 in_texCoord;

layout(set = 0, binding = 0)uniform SceneData
{
    mat4 _projection;
};

layout(location = 0)out TextData
{
    vec3 Color;
    vec2 Tex_coord;
};

void main()
{
    Color = in_color;
    Tex_coord = in_texCoord;
    gl_Position = _projection * vec4(in_pos, 1.0);
}