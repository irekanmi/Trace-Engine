#version 450

layout(location = 0)out vec4 FragColor;

layout(location = 0)in vec2 in_texCoord;

layout(set = 0, binding = 0)uniform sampler2D color_buffer;
layout(set = 0, binding = 1)uniform sampler2D scene_color_buffer;
layout(set = 0, binding = 2)uniform FrameData{
    float matrix_size;
};


void main()
{
    ivec2 tex_size = textureSize(color_buffer, 0);
    float texel_size_X = tex_size.x;
    float texel_size_Y = tex_size.y;
    texel_size_X = 1.0f / texel_size_X;
    texel_size_Y = 1.0f / texel_size_Y;

    for(float i = (-1.0f * matrix_size); i < matrix_size; i++)
    {
        for(float j = (-1.0f * matrix_size); j < matrix_size; j++)
        {
            vec4 value = texture(color_buffer, in_texCoord + vec2(i * texel_size_X, j * texel_size_Y));
            float total = value.r + value.g + value.b + value.a;
            if(total > 0.0f)
            {
                FragColor = vec4(1.0f, 1.0f, 0.878f, 1.0f);//TODO: Use custom color
                return;
            }
        }
    }

    FragColor = texture(scene_color_buffer, in_texCoord);
}