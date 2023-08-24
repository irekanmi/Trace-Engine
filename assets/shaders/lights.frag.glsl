#version 450

layout(location = 0)out vec4 FragColor;

layout(set = 1, binding = 0)uniform LightData{
    vec4 color;
};

void main()
{
    FragColor = vec4(color.rgb * color.a, 1.0f);
}
