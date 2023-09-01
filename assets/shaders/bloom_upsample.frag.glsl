#version 450

layout(location = 0)out vec4 FragColor;

layout(location = 0)in vec2 in_texCoord;

layout(set = 0, binding = 0)uniform sampler2D u_srcTexture;
layout(set = 0, binding = 3)uniform SampleData{
    float filterRadius;
};

vec4 sampleTent_3x3(sampler2D tex, vec2 inUV, float radius);
vec4 sampleBox_4x4(sampler2D tex, vec2 inUV, float radius);

void main()
{
    FragColor = sampleTent_3x3(u_srcTexture, in_texCoord, filterRadius);
    FragColor.a = 1.0f;

}

vec4 sampleTent_3x3(sampler2D tex, vec2 inUV, float radius)
{
    // Apply a 3x3 tent filter to upsample using
    // 1| 1 2 1 |
    // -| 2 4 2 |
    //16| 1 2 1 |

    // a - b - c
    // d - e - f
    // g - h - i
    //  e => begin the current texel

    ivec2 tex_size = textureSize(tex, 0);
    float texel_size_X = tex_size.x;
    float texel_size_Y = tex_size.y;
    texel_size_X = 1.0f / texel_size_X;
    texel_size_Y = 1.0f / texel_size_Y;
    texel_size_X *= radius;
    texel_size_Y *= radius;

    vec4 a = texture(tex, inUV + vec2(-1 * texel_size_X, 1 * texel_size_Y));
    vec4 b = texture(tex, inUV + vec2(0.0f, 1 * texel_size_Y));
    vec4 c = texture(tex, inUV + vec2(1 * texel_size_X, 1 * texel_size_Y));
    vec4 d = texture(tex, inUV + vec2(-1 * texel_size_X, 0.0f));
    vec4 e = texture(tex, inUV + vec2(0.0f, 0.0f));
    vec4 f = texture(tex, inUV + vec2(1 * texel_size_X, 0.0f));
    vec4 g = texture(tex, inUV + vec2(-1 * texel_size_X, -1 * texel_size_Y));
    vec4 h = texture(tex, inUV + vec2(0.0f, -1 * texel_size_Y));
    vec4 i = texture(tex, inUV + vec2(1 * texel_size_X, -1 * texel_size_Y));

    vec4 result;
    result = e * 4;
    result += (b + d + f + h) * 2;
    result += (a + c + g + i);
    result *= 1.0 / 16.0;

    return result;
}

vec4 sampleBox_4x4(sampler2D tex, vec2 inUV, float radius)
{
    //a - b
    //c - d

    vec4 result;
    ivec2 tex_size = textureSize(tex, 0);
    float texel_size_X = tex_size.x;
    float texel_size_Y = tex_size.y;
    texel_size_X = 1.0f / texel_size_X;
    texel_size_Y = 1.0f / texel_size_Y;
    texel_size_X *= radius;
    texel_size_Y *= radius;


    vec4 a = texture(tex, inUV + vec2(-1 * texel_size_X, 1 * texel_size_Y));
    vec4 b = texture(tex, inUV + vec2(1 * texel_size_X, 1 * texel_size_Y));
    vec4 c = texture(tex, inUV + vec2(-1 * texel_size_X, -1 * texel_size_Y));
    vec4 d = texture(tex, inUV + vec2(1 * texel_size_X, -1 * texel_size_Y));

    result = a + b + c + d;
    result *= 0.25f;
    return result;
}