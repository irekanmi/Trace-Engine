#version 450

#include "globals_data.glsl"
#include "bindless.glsl"


DEFAULT_VERTEX_INPUT

#define MAX_QUAD_INSTANCE 512

layout(location = 0)out vec2 out_texCoord;

layout(set = 0, binding = 0)uniform SceneData
{
    mat4 _projection;
    vec3 camera_position;
};



struct VertexData{
    vec4 positions[MAX_QUAD_INSTANCE];
    vec4 colors[MAX_QUAD_INSTANCE];
    vec4 scale[MAX_QUAD_INSTANCE];
    mat4 model;
};

layout(std140, set = 1, binding = 3) readonly buffer DrawData{
    VertexData objects[];
};

layout(location = 2) out Data{
    vec4 color;
};


void main()
{
    out_texCoord = in_texCoord;
    color = objects[binding_index.draw_instance_index.x].colors[gl_InstanceIndex];

    vec3 pos = objects[binding_index.draw_instance_index.x].positions[gl_InstanceIndex].xyz;
    vec3 scale = objects[binding_index.draw_instance_index.x].scale[gl_InstanceIndex].xyz;
    vec3 cam_pos = camera_position;
    vec3 forward = -normalize(cam_pos - pos);
    vec3 world_up = vec3(0.0f, 1.0f, 0.0f); 
    vec3 ortho_right = normalize(cross(forward, world_up));
    vec3 ortho_up = normalize(cross(ortho_right, forward));

    mat4 transform = mat4(
        vec4(ortho_right * scale, 0.0f),
        vec4(ortho_up * scale, 0.0f),
        vec4(forward * scale, 0.0f),
        vec4(pos, 1.0f)
    );

    mat4 local_pose = objects[binding_index.draw_instance_index.x].model;

    gl_Position = _projection * local_pose * transform * vec4(in_pos, 1.0f);
}