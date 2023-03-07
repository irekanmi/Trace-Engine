#version 450

layout(location = 0)out vec4 FragColor;

layout(location = 0)in vec3 out_texCoord;

layout(set = 0, binding = 1)uniform samplerCube CubeMap;

void main()
{
   
    FragColor = texture(CubeMap, out_texCoord);
}