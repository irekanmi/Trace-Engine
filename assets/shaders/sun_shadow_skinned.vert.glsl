#version 450

#include "globals_data.glsl"
#include "bindless.glsl"
#include "defines.glsl"

layout(location = 0)in vec3 in_pos;
layout(location = 1)in vec3 in_normal;
layout(location = 2)in vec2 in_texCoord;
layout(location = 3)in vec4 in_tangent;
layout(location = 4)in ivec4 in_bone_ids;
layout(location = 5)in vec4 in_bone_weights;



layout(std140, set = 0, binding = 0)uniform SceneBufferObject{
    mat4 _view_proj;
};


// INSTANCE_UNIFORM_BUFFER_SLOT(BoneMatrix,
// {
//     mat4 _bone_matrices[MAX_BONES_PER_MESH];
// },
// 5);

struct BoneMatrix
{
    mat4 _bone_matrices[MAX_BONES_PER_MESH];
};

layout(std140, set = 1, binding = 1) readonly buffer BoneData{
    BoneMatrix objects[];
};

void main()
{

    vec3 pos = vec3(0.0f);

    mat4 bone_mat4 = objects[binding_index.draw_instance_index.x]._bone_matrices[in_bone_ids.x];
    pos += vec3(bone_mat4 * vec4(in_pos, 1.0f) ) * in_bone_weights.x;

    bone_mat4 = objects[binding_index.draw_instance_index.x]._bone_matrices[in_bone_ids.y];
    pos += vec3(bone_mat4 * vec4(in_pos, 1.0f) * in_bone_weights.y);

    bone_mat4 = objects[binding_index.draw_instance_index.x]._bone_matrices[in_bone_ids.z];
    pos += vec3(bone_mat4 * vec4(in_pos, 1.0f) * in_bone_weights.z);

    bone_mat4 = objects[binding_index.draw_instance_index.x]._bone_matrices[in_bone_ids.w];
    pos += vec3(bone_mat4 * vec4(in_pos, 1.0f) * in_bone_weights.w);


    gl_Position = _view_proj * vec4(pos, 1.0f);
}