#version 450

#define MAX_FONT_TEXTURE_SLOT 16

layout(location = 0)out vec4 FragColor;

layout(location = 0)in vec2 in_texCoord;
layout(location = 1)in flat float in_texIndex;

layout(set = 0, binding = 1)uniform sampler2D u_textures[MAX_FONT_TEXTURE_SLOT];

const float pxRange = 2.0f; // set to distance field's pixel range

float screenPxRange(sampler2D msdf);
float median(float r, float g, float b) {
    return max(min(r, g), min(max(r, g), b));
}

void main()
{
    vec4 color = vec4(0.0f);
    float sPxRange;
    switch(int(in_texIndex))
    {
        case 0:
        {
            color = texture(u_textures[0], in_texCoord);
            sPxRange = screenPxRange(u_textures[0]);
            break;
        }
        case 1:
        {
            color = texture(u_textures[1], in_texCoord);
            sPxRange = screenPxRange(u_textures[1]);
            break;
        }
        case 2:
        {
            color = texture(u_textures[2], in_texCoord);
            sPxRange = screenPxRange(u_textures[2]);
            break;
        }
        case 3:
        {
            color = texture(u_textures[3], in_texCoord);
            sPxRange = screenPxRange(u_textures[3]);
            break;
        }
    }
    vec4 bgColor = vec4(0.0f);
    vec4 fgColor = vec4(0.75f, 0.22f, 0.71f,1.0f);
    vec3 msd = color.rgb;
    float sd = median(msd.r, msd.g, msd.b);
    float screenPxDistance = sPxRange*(sd - 0.5);
    float opacity = clamp(screenPxDistance + 0.5, 0.0, 1.0);
    color = mix(bgColor, fgColor, opacity);

    FragColor = color;
}

float screenPxRange(sampler2D msdf) {
    vec2 unitRange = vec2(pxRange)/vec2(textureSize(msdf, 0));
    vec2 screenTexSize = vec2(1.0)/fwidth(in_texCoord);
    return max(0.5*dot(unitRange, screenTexSize), 1.0);
}
