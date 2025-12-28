## Vulkan Descriptor Set

``cpp

struct VKDescriptorSet
{
    VkDescriptorSet internal_handle;

    // Resources
    buffers
    std::map<uint32_t, VKBuffer> buffers;
    //TODO texture cahces
    
}

``

- This structure holds the resources reqiured for a vulkan descriptor set 


### How to be used 

- The set with index zero in the shader `layout(set = 0)` is the set that holds
data that only changes once per frame for example the camera view matrix or 
position, this set will be stored in the `struct VKPipeline` so that data can
be modified. Note new `struct VKDescriptorSet` will be created on the fly 
when required per view(render graph index) because camera data from each view
will not necessarily be the same for each view.

``cpp
struct VKPipeline
{
    ...
    std::unordered_map<uint32_t, VKDescriptorSet> views_set;// Per view descriptor set; key: view_index, value: view_set
    ...
}

``

-  The set with index zero in the shader `layout(set = 1)` is the set that holds 
per instance(object) data mostly material data, with that this set will stored 
in the `struct VKMaterialData`

``cpp
struct VKMaterialData
{
    VKDescriptorSet instance_set;
}

``


### --...--

``cpp

struct VKDeviceHandle
{
    ...
    VkDescriptorPool global_descriptor_pool;
}


``