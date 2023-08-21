#version 450

layout(location = 0)out vec4 FragColor;

layout(location = 0)in vec2 in_texCoord;

layout(set = 0, binding = 0)uniform sampler2D u_HdrTarget;

void main()
{
    float gamma = 2.2f;
    vec3 hdr_result = texture(u_HdrTarget, in_texCoord).rgb;
    
    hdr_result = hdr_result / (hdr_result + vec3(1.0f));
    hdr_result = pow(hdr_result, vec3(1.0f / gamma));

    FragColor = vec4(hdr_result, 1.0f);
}