#version 450


layout(location = 0)out vec4 FragColor;

layout(location = 0)in vec2 in_texCoord;

#include "functions.glsl"
#include "bindless.glsl"



//layout(set = 1, binding = 0)uniform sampler2D u_srcTexture[];
//INSTANCE_COMBINED_SAMPLER2D(u_srcTexture);
BINDLESS_COMBINED_SAMPLER2D;
layout(set = 0, binding = 3)uniform SampleData{
    float filterRadius;
};

void main()
{
    INSTANCE_TEXTURE_INDEX(up, 0);

    FragColor = sampleTent_3x3(GET_BINDLESS_TEXTURE2D(up), in_texCoord, filterRadius);
    FragColor.a = 1.0f;

}

