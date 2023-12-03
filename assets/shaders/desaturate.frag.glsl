#version 450


#include "globals_data.glsl"
#include "utils.glsl"

OUT_FRAG_DATA
IN_VERTEX_DATA


layout(set = 1, binding = 0)uniform InstanceBufferObject{
    vec4 diffuse_color;
    float shininess;
    float mix_ratio;
};
layout(set = 1, binding = 1)uniform sampler2D DIFFUSE_MAP;
layout(set = 1, binding = 3)uniform sampler2D NORMAL_MAP;

void main()
{
    vec3 normal;
    SAMPLE_NORMAL_MAP(NORMAL_MAP, _texCoord, _normal_, _tangent_, normal );

    vec3 color = SampleTextureMap_RGB(DIFFUSE_MAP, _texCoord);
    float average = (color.r + color.g + color.b) / 3;
    vec3 result = vec3(average);

    FRAG_POS = _fragPos;
    FRAG_NORMAL = normal;
    FRAG_SHININESS = shininess;
    FRAG_COLOR = result;
    FRAG_SPECULAR = 0.5f;
}
