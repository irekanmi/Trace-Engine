


vec4 prefilter_color(vec4 color, float falloff)
{
    float brightness = max(max(color.r, color.g), color.b);
    float numerator = max(brightness - falloff, 0.0001f);
    float denominator = max(brightness, 0.0001f);
    float result = numerator / denominator;
    return color * result;
}

vec4 sampleTexture13Tap(sampler2D tex, vec2 inUV)
{
    //a - b - c
    //- d - e -
    //f - g - h
    //- i - j -
    //k - l - m
    // g => being the current texel
    vec4 result;
    ivec2 tex_size = textureSize(tex, 0);
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


    vec4 a = texture(tex, inUV + vec2(-1 * texel_size_X, 1 * texel_size_Y));
    vec4 b = texture(tex, inUV + vec2(1 * texel_size_X, 1 * texel_size_Y));
    vec4 c = texture(tex, inUV + vec2(-1 * texel_size_X, -1 * texel_size_Y));
    vec4 d = texture(tex, inUV + vec2(1 * texel_size_X, -1 * texel_size_Y));

    result = a + b + c + d;
    result *= 0.25f;
    return result;
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

vec4 colorFromUint32(uint color)
{
    vec4 result;

    float a = float(color >> 24) / 255.0f;
    float b = float( (color >> 16) & 0x000000FF) / 255.0f;
    float g = float( (color >> 8) & 0x000000FF) / 255.0f;
    float r = float( (color >> 0) & 0x000000FF) / 255.0f;

    result.r = r;
    result.g = g;
    result.b = b;
    result.a = a;

    return result;
}

