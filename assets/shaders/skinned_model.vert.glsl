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
OUT_VERTEX_DATA

layout(std140, set = 0, binding = 0)uniform SceneBufferObject{
    mat4 _projection;
    mat4 _view;
    vec3 _view_position;
};

struct BoneMatrix
{
    mat4 _bone_matrices[MAX_BONES_PER_MESH];
};

layout(std140, set = 1, binding = 1) readonly buffer BoneData{
    BoneMatrix objects[];
};



void main()
{
    _texCoord = in_texCoord;

    vec3 tang = vec3(0.0f);
    vec3 pos = vec3(0.0f);
    vec3 norm = vec3(0.0f);

    //mat4 bone_mat4 = GET_INSTANCE_PARAM(_bone_matrices, BoneMatrix)[in_bone_ids.x];
    mat4 bone_mat4 = objects[binding_index.draw_instance_index.x]._bone_matrices[in_bone_ids.x];
    pos += vec3(bone_mat4 * vec4(in_pos, 1.0f) ) * in_bone_weights.x;
    mat3 _mat3 = mat3(bone_mat4);
    norm += (_mat3 * in_normal) * in_bone_weights.x;
    tang += (_mat3 * in_tangent.xyz) * in_bone_weights.x;

    bone_mat4 = objects[binding_index.draw_instance_index.x]._bone_matrices[in_bone_ids.y];
    pos += vec3(bone_mat4 * vec4(in_pos, 1.0f) * in_bone_weights.y);
    _mat3 = mat3(bone_mat4);
    norm += _mat3 * in_normal * in_bone_weights.y;
    tang += _mat3 * in_tangent.xyz * in_bone_weights.y;

    bone_mat4 = objects[binding_index.draw_instance_index.x]._bone_matrices[in_bone_ids.z];
    pos += vec3(bone_mat4 * vec4(in_pos, 1.0f) * in_bone_weights.z);
    _mat3 = mat3(bone_mat4);
    norm += _mat3 * in_normal * in_bone_weights.z;
    tang += _mat3 * in_tangent.xyz * in_bone_weights.z;

    bone_mat4 = objects[binding_index.draw_instance_index.x]._bone_matrices[in_bone_ids.w];
    pos += vec3(bone_mat4 * vec4(in_pos, 1.0f) * in_bone_weights.w);
    _mat3 = mat3(bone_mat4);
    norm += _mat3 * in_normal * in_bone_weights.w;
    tang += _mat3 * in_tangent.xyz * in_bone_weights.w;

    // mat3 model_view_mat3 = mat3(_view * GET_INSTANCE_PARAM(_model, LocalBufferObject));
    //mat3 norm_mat3 = transpose(inverse(model_view_mat3));
    _normal_ = normalize(mat3(_view)  * norm);
    _tangent_ = vec4( normalize(mat3(_view) * tang), in_tangent.w );
    //world_position = (GET_INSTANCE_PARAM(_model, LocalBufferObject) * vec4(in_pos, 1.0f)).xyz;
    world_position = pos;
    _fragPos = (_view * vec4(world_position, 1.0f)).xyz;
    

    gl_Position = _projection * vec4(_fragPos, 1.0f);
}