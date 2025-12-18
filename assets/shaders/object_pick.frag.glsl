#version 450


#include "globals_data.glsl"
#include "bindless.glsl"

layout(location = 0)out vec4 out_id;

IN_VERTEX_DATA


struct InstanceBufferObject
{
    vec4 object_id;
};

layout(std140, set = 1, binding = 3) readonly buffer MaterialData{
    InstanceBufferObject objects[];
};



void main()
{
    out_id = objects[binding_index.draw_instance_index.x].object_id;
}
