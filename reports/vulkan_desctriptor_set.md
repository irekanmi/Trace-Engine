## Vulkan Descriptor Set

```cpp

struct VKDescriptorSet
{
    VkDescriptorSet internal_handle;

    // Resources
    //buffers
    std::map<uint32_t, VKBuffer> buffers;
    //TODO texture cahces
    std::map<uint32_t, std::vector<VKImage*>> textures;
    
}

```

- This structure holds the resources reqiured for a vulkan descriptor set 


### How to be used 

- The set with index zero in the shader `layout(set = 0)` is the set that holds
data that only changes once per frame for example the camera view matrix or 
position, this set will be stored in the `struct VKPipeline` so that data can
be modified. Note new `struct VKDescriptorSet` will be created on the fly 
when required per view(render graph index) because camera data from each view
will not necessarily be the same for each view.

```cpp
struct VKPipeline
{
    //...
    std::unordered_map<uint32_t, std::array<VKDescriptorSet, VK_MAX_NUM_FRAMES>> views_set;// Per view descriptor set; key: view_index, value: view_set
    //...
}

```

- The set with index zero in the shader `layout(set = 1)` is the set that holds material data, with that this set will
stored in the `struct VKMaterialData`

```cpp
struct VKMaterialData
{
    VKDescriptorSet instance_set;
}

```

- The set with index zero in the shader `layout(set = 2)` is the set that holds per object(for each draw call) for 
example and object model matrix or the skinning matrix. Note this set is assumed to a single structure and only holds 
buffer and no 

```cpp

struct VKPipeline
{
    //...
    std::array<VKDescriptorSet, VK_MAX_NUM_FRAMES> draw_call_sets;
    //...
}

```


### --...--

```cpp

struct VKDeviceHandle
{
    //...
    VkDescriptorPool global_descriptor_pool;
}


```