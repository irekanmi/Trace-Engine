#version 450

layout(location = 0)out vec4 FragColor;

layout(location = 0)in vec2 texCoord;

layout(set = 0, binding = 0)uniform sampler2D input_texture;


void main()
{
    vec4 value = texture(input_texture, texCoord);
    FragColor = vec4((value.x + value.y + value.z + value.w) / 4);
}