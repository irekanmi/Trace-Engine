#version 450

#define MAX_LIGHT_COUNT 4

#include "globals_data.glsl"
#include "utils.glsl"

OUT_FRAG_DATA
IN_VERTEX_DATA


layout(set = 1, binding = 0)uniform InstanceBufferObject{
    vec4 diffuse_color;
    float shininess;
};
layout(set = 1, binding = 1)uniform sampler2D DIFFUSE_MAP;
layout(set = 1, binding = 2)uniform sampler2D SPECULAR_MAP;
layout(set = 1, binding = 3)uniform sampler2D NORMAL_MAP;

void main()
{
    vec3 normal;
    SAMPLE_NORMAL_MAP(NORMAL_MAP, _texCoord, _normal_, _tangent_, normal );

    FRAG_POS = _fragPos;
    FRAG_NORMAL = normal;
    FRAG_SHININESS = shininess;
    FRAG_COLOR = SampleTextureMap_RGB(DIFFUSE_MAP, _texCoord);
    FRAG_SPECULAR = SampleTextureMap_R(SPECULAR_MAP, _texCoord);
}
