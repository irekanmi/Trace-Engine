#version 450

layout(location = 0)out vec4 FragColor;

layout(location = 0)in vec2 in_texCoord;

layout(set = 0, binding = 0)uniform sampler2D u_srcTexture;
layout(set = 0, binding = 1)uniform FilterData{
    float threshold;
};

vec4 prefilter_color(vec4 color, float falloff);
vec4 sampleTexture13Tap(sampler2D tex, vec2 inUV, ivec2 tex_size);
vec4 sampleBox_4x4(sampler2D tex, vec2 inUV, ivec2 tex_size, float radius);

void main()
{
    ivec2 src_resolution = textureSize(u_srcTexture, 0);
    FragColor = prefilter_color(sampleTexture13Tap(u_srcTexture, in_texCoord, src_resolution), threshold);
    FragColor.a = 1.0f;
}

vec4 prefilter_color(vec4 color, float falloff)
{
    float brightness = max(max(color.r, color.g), color.b);
    float numerator = max(brightness - falloff, 0.0001f);
    float denominator = max(brightness, 0.0001f);
    float result = numerator / denominator;
    return color * result;
}

vec4 sampleTexture13Tap(sampler2D tex, vec2 inUV, ivec2 tex_size)
{
    //a-b-c
    //-d-e-
    //f-g-h
    //-i-j-
    //k-l-m
    // g => being the current texel
    vec4 result;
    float texel_size_X = tex_size.x;
    float texel_size_Y = tex_size.y;
    texel_size_X = 1.0f / texel_size_X;
    texel_size_Y = 1.0f / texel_size_Y;

    vec4 a = texture(tex, inUV + vec2(-2 * texel_size_X, 2 * texel_size_Y)); 
    vec4 b = texture(tex, inUV + vec2(texel_size_X, 2 * texel_size_Y)); 
    vec4 c = texture(tex, inUV + vec2(2 * texel_size_X, 2 * texel_size_Y)); 
    vec4 d = texture(tex, inUV + vec2(-1 * texel_size_X, 1 * texel_size_Y)); 
    vec4 e = texture(tex, inUV + vec2(1 * texel_size_X, 1 * texel_size_Y)); 
    vec4 f = texture(tex, inUV + vec2(-1 * texel_size_X, 0.0f)); 
    vec4 g = texture(tex, inUV + vec2(0.0f, 0.0f));
    vec4 h = texture(tex, inUV + vec2(1 * texel_size_X, 0.0f));
    vec4 i = texture(tex, inUV + vec2(-1 * texel_size_X, -1 * texel_size_Y));
    vec4 j = texture(tex, inUV + vec2(1 * texel_size_X, -1 * texel_size_Y));
    vec4 k = texture(tex, inUV + vec2(-2 * texel_size_X, -2 * texel_size_Y));
    vec4 l = texture(tex, inUV + vec2(0.0f, -2 * texel_size_Y));
    vec4 m = texture(tex, inUV + vec2(2 * texel_size_X, -2 * texel_size_Y));
    
    // Weight to be appiled to ensure energy preservation
    // 0.5 + 0.125 + 0.125 + 0.125 + 0.125 = 1;
    result = g * 0.125f;
    result += (a + c + k + m) * 0.03125f;
    result += (b + f + h + l) * 0.0625f;
    result += (d + e + i + j) * 0.125f;

    return result;
}

vec4 sampleBox_4x4(sampler2D tex, vec2 inUV, ivec2 tex_size, float radius)
{
    //a - b
    //c - d

    vec4 result;
    float texel_size_X = tex_size.x;
    float texel_size_Y = tex_size.y;
    texel_size_X = 1.0f / texel_size_X;
    texel_size_Y = 1.0f / texel_size_Y;


    vec4 a = texture(tex, inUV + vec2(-1 * texel_size_X, 1 * texel_size_Y));
    vec4 b = texture(tex, inUV + vec2(1 * texel_size_X, 1 * texel_size_Y));
    vec4 c = texture(tex, inUV + vec2(-1 * texel_size_X, -1 * texel_size_Y));
    vec4 d = texture(tex, inUV + vec2(1 * texel_size_X, -1 * texel_size_Y));

    result = a + b + c + d;
    result *= 0.25f;
    return result;
}