#version 450

#include "globals_data.glsl"

DEFAULT_VERTEX_INPUT
OUT_VERTEX_DATA

layout(std140, set = 0, binding = 0)uniform SceneBufferObject{
    mat4 _projection;
    mat4 _view;
    vec3 _view_position;
};

layout( push_constant )uniform LocalBufferObject{
        mat4 _model;
} local_data;



void main()
{
    _texCoord = in_texCoord;

    mat3 model_mat3 = mat3(_view * local_data._model);
    mat3 norm_mat3 = transpose(inverse(model_mat3));
    _normal_ = normalize(norm_mat3  * in_normal);
    _tangent_ = vec4( normalize(model_mat3 * in_tangent.xyz), in_tangent.w );
    _fragPos = (_view * local_data._model * vec4(in_pos, 1.0f)).xyz;
    
    gl_Position = _projection * _view * local_data._model * vec4(in_pos, 1.0f);
}