layout(location = 0)in vec3 in_pos;
layout(location = 1)in vec3 in_normal;
layout(location = 2)in vec2 in_texCoord;
layout(location = 3)in vec4 in_tangent;

layout(location = 0)out vec2 out_texCoord;


void main()
{
    out_texCoord = in_texCoord;
    gl_Position = in_pos;
}