#version 450

layout(location = 0)out vec4 FragColor;

layout(location = 0)in vec2 in_texCoord;

layout(set = 0, binding = 0)uniform sampler2D u_HdrTarget;
layout(set = 0, binding = 1)uniform FrameData{
    float exposure;
};


void main()
{
    float gamma = 2.2f;
    vec3 result;
    vec4 texture_data = texture(u_HdrTarget, in_texCoord);
    vec3 hdr_result = texture_data.rgb;

    result = vec3(1.0f) - exp(-hdr_result * exposure);
    //result = (hdr_result) / (hdr_result + vec3(1.0f));
    //result = pow(result, vec3(1.0f / gamma)); // gamma correction

    FragColor = vec4(result, texture_data.a);
}