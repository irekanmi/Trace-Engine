#version 450

#include "globals_data.glsl"
#include "bindless.glsl"
#include "functions.glsl"


DEFAULT_VERTEX_INPUT

#define MAX_QUAD_INSTANCE 512

layout(location = 0)out vec2 out_texCoord;

layout(set = 0, binding = 0)uniform SceneData
{
    mat4 _projection;
};



struct VertexData{
    vec4 _positions[MAX_QUAD_INSTANCE];
    vec4 _colors[MAX_QUAD_INSTANCE];
    vec4 _scales[MAX_QUAD_INSTANCE];
    vec4 _velocities[MAX_QUAD_INSTANCE];
    mat4 _model;
};

layout(std140, set = 1, binding = 3) readonly buffer DrawData{
    VertexData objects[];
};

layout(location = 2) out Data{
    vec3 position;
    vec3 color;
    vec3 scale;
    vec2 lifetime;
};


void main()
{
    out_texCoord = in_texCoord;

    vec4 pos = objects[binding_index.draw_instance_index.x]._positions[gl_InstanceIndex];
    vec3 col = objects[binding_index.draw_instance_index.x]._colors[gl_InstanceIndex].xyz;
    vec4 scl = objects[binding_index.draw_instance_index.x]._scales[gl_InstanceIndex];
    vec3 velocity = objects[binding_index.draw_instance_index.x]._velocities[gl_InstanceIndex].xyz;

    position = pos.xyz;
    color = col;
    scale = scl.xyz;
    lifetime.x = pos.w;
    lifetime.y = lifetime.x / scl.w;

    vec3 forward = -normalize(velocity);
    vec3 world_up = vec3(0.0f, 1.0f, 0.0f); 
    vec3 ortho_right = normalize(cross(forward, world_up));
    vec3 ortho_up = normalize(cross(ortho_right, forward));


    mat4 transform = PositionLookAt(ortho_right, scale);
    transform[3] = vec4(position, 1.0f);

    mat4 local_pose = objects[binding_index.draw_instance_index.x]._model;

    gl_Position = _projection * local_pose * transform * vec4(in_pos, 1.0f);
}