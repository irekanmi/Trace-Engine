#version 450

layout(location = 0)in vec3 in_pos;
layout(location = 1)in vec3 in_normal;
layout(location = 2)in vec2 in_texCoord;
layout(location = 3)in vec4 in_tangent;

layout(set = 0, binding = 0)uniform SceneData{
    mat4 _view_projection;
};

layout(push_constant)uniform Model{
    mat4 _model;
};

void main()
{

    gl_Position = _view_projection * _model * vec4(in_pos, 1.0f);

}