#version 450


layout(location = 0)out vec4 FragColor;

layout(location = 0)in vec2 in_texCoord;

#include "functions.glsl"
#include "bindless.glsl"


layout(set = 0, binding = 4)uniform sampler2D u_Texture[];
//INSTANCE_COMBINED_SAMPLER2D(u_srcTexture);
BINDLESS_COMBINED_SAMPLER2D;

void main()
{
    INSTANCE_TEXTURE_INDEX(down, 0);

    FragColor = sampleTexture13Tap(GET_BINDLESS_TEXTURE2D(down), in_texCoord);
}
