#version 450

#define MAX_NUM_KERNEL 64

layout(location = 0)out float FragColor;

layout(location = 0)in vec3 in_texCoord;

layout(set = 0, binding = 1)uniform sampler2D u_noiseTexture;
layout(set = 0, binding = 2)uniform Kernel{
    vec3 rand_vec;
} u_kernel[MAX_NUM_KERNEL];
layout(set = 0, binding = 3)uniform sampler2D g_bufferData[2];
layout(set = 0, binding = 4)uniform FrameData{
    vec2 frame_size;
};

void main()
{

}