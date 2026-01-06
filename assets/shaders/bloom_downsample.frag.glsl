#version 450


#include "functions.glsl"
#include "bindless.glsl"

layout(location = 0)out vec4 FragColor;

layout(location = 0)in vec2 in_texCoord;




layout(set = 0, binding = 4)uniform sampler2D u_Texture;
//INSTANCE_COMBINED_SAMPLER2D(u_srcTexture);
BINDLESS_COMBINED_SAMPLER2D_SET(1, 7);

void main() 
{
    INSTANCE_TEXTURE_INDEX_DRAW_CALL(down, 0);

    FragColor = sampleTexture13Tap(GET_BINDLESS_TEXTURE2D(down), in_texCoord);
}
