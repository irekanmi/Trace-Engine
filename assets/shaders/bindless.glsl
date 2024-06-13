
#extension GL_EXT_nonuniform_qualifier : require

layout( push_constant )uniform Bindless{
    ivec4 draw_instance_index; // x: uniform buffer index, y: combined sampler index
} binding_index;

#define UNIFORM_BUFFER_BINDING 6
#define COMBINED_SAMPLER2D_BINDING 7

#define INSTANCE_COMBINED_SAMPLER2D(name) layout(set = 1, binding = COMBINED_SAMPLER2D_BINDING)uniform sampler2D name[]

#define INSTANCE_UNIFORM_BUFFER_SLOT(name, _struct, slot) layout(set = 1, binding = slot)uniform name _struct u_##name[]

#define INSTANCE_UNIFORM_BUFFER(name, _struct) INSTANCE_UNIFORM_BUFFER_SLOT(name, _struct, UNIFORM_BUFFER_BINDING)

#define BINDLESS_COMBINED_SAMPLER2D INSTANCE_COMBINED_SAMPLER2D(_bindlessSampler2D)


#define KB 1024

struct TexIndex
{
    int index;
};

#define INSTANCE_TEXTURE_INDEX(name, index) TexIndex name = { index }

#define GET_INSTANCE_TEXTURE(uniform_texture_name, name) uniform_texture_name[binding_index.draw_instance_index.y + name.index]

#define GET_BINDLESS_TEXTURE2D(name) GET_INSTANCE_TEXTURE(_bindlessSampler2D, name)

#define GET_INSTANCE_PARAM(param, name) u_##name[binding_index.draw_instance_index.x].param