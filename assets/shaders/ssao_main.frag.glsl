#version 450

#define MAX_NUM_KERNEL 64

layout(location = 0)out float FragColor;

layout(location = 0)in vec2 in_texCoord;

layout(set = 0, binding = 1)uniform sampler2D u_noiseTexture;
layout(set = 0, binding = 2)uniform Kernel{
    vec4 rand_vec;
} u_kernel[MAX_NUM_KERNEL];
layout(set = 0, binding = 3)uniform sampler2D g_bufferData[2];
layout(set = 0, binding = 4)uniform FrameData{
    mat4 projection;
    vec2 frame_size;
};

void main()
{
    vec2 noise_scale = frame_size / 4.0;

    vec3 frag_pos = texture(g_bufferData[0], in_texCoord).xyz;
    vec3 normal = texture(g_bufferData[1], in_texCoord).xyz;
    vec3 rand_vec = texture(u_noiseTexture, in_texCoord * noise_scale).xyz;
    vec3 tangent = normalize( rand_vec - normal * dot(rand_vec, normal) );
    vec3 bi_tangent = cross(normal, tangent);
    mat3 TBN = mat3( tangent, bi_tangent, normal );
    float radius = 0.5f;
    float bias = 0.025f;

    float occulsion = 0.0f;
    for(int i = 0; i < MAX_NUM_KERNEL; i++)
    {

        vec3 samp_pos = TBN * u_kernel[i].rand_vec.xyz;
        samp_pos = frag_pos + (samp_pos * radius);
        
        vec4 offset = projection * vec4(samp_pos, 1.0f);
        offset.xyz /= offset.w;
        offset.xyz = offset.xyz * 0.5f + 0.5f;

        if(offset.x > 1.0f || offset.y > 1.0f)
        {
            FragColor = offset.x;
            return;
        }

        float samp_depth = texture(g_bufferData[0], offset.xy).z;

        float range = smoothstep(0.0f, 1.0f, radius / abs(frag_pos.z - samp_depth));
        occulsion += ( samp_depth >= samp_pos.z + bias ? 1.0f : 0.0f ) * range;
    }

    occulsion = 1.0f - (occulsion / MAX_NUM_KERNEL);
    FragColor = occulsion;


}