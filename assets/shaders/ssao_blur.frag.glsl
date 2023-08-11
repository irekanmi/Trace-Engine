#version 450

layout(location = 0)out float FragColor;

layout(location = 0)in vec2 in_texCoord;
layout(set = 0, binding = 1)uniform sampler2D ssao_main;

void main()
{
    vec2 tex_size = 1.0f / vec2(textureSize(ssao_main, 0));

    float result = 0.0f;
    for(int x = -2; x < 2; x++)
    {

        for(int y = -2; y < 2; y++)
        {
            vec2 offset = vec2(float(x), float(y)) * tex_size;
            result += texture(ssao_main, in_texCoord + offset).r;
        }

    }

    result /= 16.0f;
    FragColor = result;

}