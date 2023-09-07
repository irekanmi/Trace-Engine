#version 450

layout(location = 0)out vec4 FragColor;

layout(location = 0)in vec2 texCoord;

layout(set = 0, binding = 0)uniform sampler2D color;


void main()
{
    vec4 value = texture(color, texCoord);
    FragColor = vec4(vec3((value.x + value.y + value.z) / 3), 1.0f);
}