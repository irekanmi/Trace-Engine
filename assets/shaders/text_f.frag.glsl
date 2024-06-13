#version 450

#include "bindless.glsl"

layout(location = 0)out vec4 FragColor;

layout(location = 0)in TextData
{
    vec3 Color;
    vec2 Tex_coord;
};

//layout(set = 1, binding = 0)uniform sampler2D u_texture;
BINDLESS_COMBINED_SAMPLER2D;

const float pxRange = 2.0f; // set to distance field's pixel range

float screenPxRange(sampler2D msdf);
float median(float r, float g, float b) {
    return max(min(r, g), min(max(r, g), b));
}

void main()
{
    INSTANCE_TEXTURE_INDEX(u_texture, 0);

    vec4 color = vec4(0.0f);
    float sPxRange;
    color = texture(GET_BINDLESS_TEXTURE2D(u_texture), Tex_coord);
    sPxRange = screenPxRange(GET_BINDLESS_TEXTURE2D(u_texture));
    
    vec4 bgColor = vec4(0.0f);
    vec4 fgColor = vec4(Color,1.0f);
    vec3 msd = color.rgb;
    float sd = median(msd.r, msd.g, msd.b);
    float screenPxDistance = sPxRange*(sd - 0.5);
    float opacity = clamp(screenPxDistance + 0.5, 0.0, 1.0);
    color = mix(bgColor, fgColor, opacity);

    FragColor = color;
}

float screenPxRange(sampler2D msdf) {
    vec2 unitRange = vec2(pxRange)/vec2(textureSize(msdf, 0));
    vec2 screenTexSize = vec2(1.0)/fwidth(Tex_coord);
    return max(0.5*dot(unitRange, screenTexSize), 1.0);
}
