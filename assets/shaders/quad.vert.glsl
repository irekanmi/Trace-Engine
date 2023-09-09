#version 450

layout(location = 0)in vec3 in_pos;
layout(location = 1)in vec2 in_texCoord;
layout(location = 2)in int in_texIndex;

layout(location = 0)out vec2 out_texCoord;
layout(location = 1)out int out_texIndex;

layout(set = 0, binding = 0)uniform SceneData
{
    mat4 projection;
};

void main()
{
    out_texCoord = in_texCoord;
    out_texIndex = in_texIndex;
    gl_Position = projection * vec4(in_pos, 1.0);
}