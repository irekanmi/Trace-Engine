#version 450


#include "globals_data.glsl"
#include "utils.glsl"
#include "bindless.glsl"

OUT_FRAG_DATA
IN_VERTEX_DATA


// layout(set = 1, binding = 0)uniform InstanceBufferObject{
//     vec4 diffuse_color;
//     float shininess;
// };
INSTANCE_UNIFORM_BUFFER(InstanceBufferObject, {
    vec4 diffuse_color;
    float shininess;
});

BINDLESS_COMBINED_SAMPLER2D;
// layout(set = 1, binding = 1)uniform sampler2D DIFFUSE_MAP;
// layout(set = 1, binding = 2)uniform sampler2D SPECULAR_MAP;
// layout(set = 1, binding = 3)uniform sampler2D NORMAL_MAP;

#define GBUFFER_FRAG 1



void main()
{
    INSTANCE_TEXTURE_INDEX(DIFFUSE_MAP, 0);
    INSTANCE_TEXTURE_INDEX(SPECULAR_MAP, 1);
    INSTANCE_TEXTURE_INDEX(NORMAL_MAP, 2);

    vec3 normal;
    SAMPLE_NORMAL_MAP(GET_BINDLESS_TEXTURE2D(NORMAL_MAP), _texCoord, _normal_, _tangent_, normal );

    FRAG_POS = _fragPos;
    FRAG_NORMAL = normal;
    FRAG_SHININESS = GET_INSTANCE_PARAM(shininess, InstanceBufferObject);
    FRAG_COLOR = SampleTextureMap_RGB(GET_BINDLESS_TEXTURE2D(DIFFUSE_MAP), _texCoord);
    FRAG_SPECULAR = SampleTextureMap_R(GET_BINDLESS_TEXTURE2D(SPECULAR_MAP), _texCoord);
}
