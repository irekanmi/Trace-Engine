#version 450

#define G_POSITION 0
#define G_NORMAL 1
#define G_COLOR 2

layout(location = 0)out vec4 FragColor;

layout(location = 1)in vec2 in_texCoord;

layout(set = 0, binding = 0)uniform sampler2D g_bufferData[3];

layout(set = 0, binding = 1)uniform DebugData{
    ivec4 rest;
};

void main()
{
    if(rest.x == 0)
    {
        FragColor = texture(g_bufferData[G_POSITION], in_texCoord);
    }
    else if(rest.x == 1)
    {
        FragColor = texture(g_bufferData[G_NORMAL], in_texCoord);
    }
    else if(rest.x == 2)
    {
        FragColor = texture(g_bufferData[G_COLOR], in_texCoord);
    }
}