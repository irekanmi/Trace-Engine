#include "pch.h"
#include "VulkanMaterial.h"
#include "VulkanPipeline.h"
#include "VulkanTexture.h"

extern trace::VKHandle g_Vkhandle;
extern trace::VKDeviceHandle g_VkDevice;
namespace trace {
    VulkanMaterial::VulkanMaterial()
    {
    }
    VulkanMaterial::~VulkanMaterial()
    {

    }
    bool VulkanMaterial::Init(Ref<GPipeline> pipeline, Material material)
    {
        m_instance = &g_Vkhandle;
        m_device = &g_VkDevice;
        m_renderPipeline = pipeline;
        m_material.m_albedoMap = material.m_albedoMap;
        m_material.m_diffuseColor = material.m_diffuseColor;
        m_material.m_normalMap = material.m_normalMap;
        m_material.m_shininess = material.m_shininess;
        m_material.m_specularMap = material.m_specularMap;

        m_shaderData[ShaderData::MATERIAL_ALBEDO]        = std::make_pair(nullptr, INVALID_ID);
        m_shaderData[ShaderData::MATERIAL_DIFFUSE_COLOR] = std::make_pair(nullptr, INVALID_ID);
        m_shaderData[ShaderData::MATERIAL_NORMAL]        = std::make_pair(nullptr, INVALID_ID);
        m_shaderData[ShaderData::MATERIAL_SHININESS]     = std::make_pair(nullptr, INVALID_ID);
        m_shaderData[ShaderData::MATERIAL_SPECULAR]      = std::make_pair(nullptr, INVALID_ID);

        VulkanPipeline* sp = (VulkanPipeline*)m_renderPipeline.get();

        if (!sp->m_handle.Instance_sets[0])
        {
            TRC_TRACE("The render pipeline assigned to these material does not use any material data");
            return true;
        }

        VkDescriptorSetLayout layouts[] = {
            sp->m_handle.Instance_layout,
            sp->m_handle.Instance_layout,
            sp->m_handle.Instance_layout
        };

        VkDescriptorSetAllocateInfo alloc_info = {};
        alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        alloc_info.descriptorPool = sp->m_handle.Instance_pool;
        alloc_info.descriptorSetCount = 3;
        alloc_info.pSetLayouts = layouts;

        VkResult result = vkAllocateDescriptorSets(
            m_device->m_device,
            &alloc_info,
            m_sets
        );

        VK_ASSERT(result);

        PipelineStateDesc& desc = m_renderPipeline->GetDesc();

        std::vector<ShaderResourceBinding>& res = desc.resource_bindings;



        for (uint32_t i = 0; i < 3; i++)
        {
            std::vector<VkWriteDescriptorSet> _writes;
            std::vector<VkCopyDescriptorSet> _copies;
            for (uint32_t j = 0; j < desc.resource_bindings_count; j++)
            {
                if (res[j].resource_stage == ShaderResourceStage::RESOURCE_STAGE_INSTANCE)
                {
                    VkWriteDescriptorSet write = {};
                    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    write.descriptorCount = 1;
                    write.dstArrayElement = res[j].index;
                    write.dstBinding = res[j].slot;
                    write.dstSet = m_sets[i];
                    switch (res[j].resource_data_type)
                    {
                    case ShaderData::MATERIAL_ALBEDO:
                    {
                        m_shaderData[ShaderData::MATERIAL_ALBEDO] = std::make_pair(m_material.m_albedoMap.get(), sp->_hashTable.Get(res[j].resource_name));
                        VkDescriptorImageInfo img_info = {};
                        img_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                        img_info.imageView = ((VulkanTexture*)m_shaderData[res[j].resource_data_type].first)->m_handle.m_view;
                        img_info.sampler = ((VulkanTexture*)m_shaderData[res[j].resource_data_type].first)->m_sampler;
                        write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                        write.pImageInfo = &img_info;
                        _writes.push_back(write);
                        break;
                    }
                    case ShaderData::MATERIAL_DIFFUSE_COLOR:
                    {
                        m_shaderData[ShaderData::MATERIAL_DIFFUSE_COLOR] = std::make_pair(&m_material.m_diffuseColor, sp->_hashTable.Get(res[j].resource_name));
                        VkCopyDescriptorSet copy = {};
                        copy.sType = VK_STRUCTURE_TYPE_COPY_DESCRIPTOR_SET;
                        copy.descriptorCount = 1;
                        copy.dstArrayElement = res[j].index;
                        copy.dstBinding = res[j].slot;
                        copy.dstSet = m_sets[i];
                        copy.srcArrayElement = res[j].index;
                        copy.srcBinding = res[j].slot;
                        copy.srcSet = sp->m_handle.Instance_sets[i];
                        _copies.push_back(copy);
                        break;
                    }
                    case ShaderData::MATERIAL_NORMAL:
                    {
                        m_shaderData[ShaderData::MATERIAL_NORMAL] = std::make_pair(m_material.m_normalMap.get(), sp->_hashTable.Get(res[j].resource_name));
                        VkDescriptorImageInfo img_info = {};
                        img_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                        img_info.imageView = ((VulkanTexture*)m_shaderData[res[j].resource_data_type].first)->m_handle.m_view;
                        img_info.sampler = ((VulkanTexture*)m_shaderData[res[j].resource_data_type].first)->m_sampler;
                        write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                        write.pImageInfo = &img_info;
                        _writes.push_back(write);
                        break;
                    }
                    case ShaderData::MATERIAL_SHININESS:
                    {
                        m_shaderData[ShaderData::MATERIAL_SHININESS] = std::make_pair(&m_material.m_shininess, sp->_hashTable.Get(res[j].resource_name));
                        VkCopyDescriptorSet copy = {};
                        copy.sType = VK_STRUCTURE_TYPE_COPY_DESCRIPTOR_SET;
                        copy.descriptorCount = 1;
                        copy.dstArrayElement = res[j].index;
                        copy.dstBinding = res[j].slot;
                        copy.dstSet = m_sets[i];
                        copy.srcArrayElement = res[j].index;
                        copy.srcBinding = res[j].slot;
                        copy.srcSet = sp->m_handle.Instance_sets[i];
                        _copies.push_back(copy);
                        break;
                    }
                    case ShaderData::MATERIAL_SPECULAR:
                    {
                        m_shaderData[ShaderData::MATERIAL_SPECULAR] = std::make_pair(m_material.m_specularMap.get(), sp->_hashTable.Get(res[j].resource_name));
                        VkDescriptorImageInfo img_info = {};
                        img_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                        img_info.imageView = ((VulkanTexture*)m_shaderData[res[j].resource_data_type].first)->m_handle.m_view;
                        img_info.sampler = ((VulkanTexture*)m_shaderData[res[j].resource_data_type].first)->m_sampler;
                        write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                        write.pImageInfo = &img_info;
                        _writes.push_back(write);
                        break;
                    }
                    }
                }
            }
            vkUpdateDescriptorSets(
                m_device->m_device,
                static_cast<uint32_t>(_writes.size()),
                _writes.data(),
                static_cast<uint32_t>(_copies.size()),
                _copies.data()
            );
        }

        

        return true;
    }

    void VulkanMaterial::Apply()
    {

        VulkanPipeline* sp = (VulkanPipeline*)m_renderPipeline.get();
 

        if (m_shaderData[ShaderData::MATERIAL_DIFFUSE_COLOR].first)
        {
            void* _src = m_shaderData[ShaderData::MATERIAL_DIFFUSE_COLOR].first;
            uint32_t hash_ = m_shaderData[ShaderData::MATERIAL_DIFFUSE_COLOR].second;
            UniformMetaData& _meta_data = sp->Scene_uniforms[hash_];
            void* _map_point = sp->cache_data + _meta_data._offset;

            memcpy(_map_point, _src, sizeof(glm::vec4));
        }
        if (m_shaderData[ShaderData::MATERIAL_SHININESS].first)
        {
            void* _src = m_shaderData[ShaderData::MATERIAL_SHININESS].first;
            uint32_t hash_ = m_shaderData[ShaderData::MATERIAL_SHININESS].second;
            UniformMetaData& _meta_data = sp->Scene_uniforms[hash_];
            void* _map_point = sp->cache_data + _meta_data._offset;

            memcpy(_map_point, _src, sizeof(float));
        }

        uint32_t set_count = 0;
        VkDescriptorSet _sets[3];

        if (sp->m_handle.Scene_sets[0])
        {
            _sets[set_count++] = sp->m_handle.Scene_sets[m_device->m_imageIndex];
        }

        if (sp->m_handle.Instance_sets[0])
        {
            _sets[set_count++] = m_sets[m_device->m_imageIndex];
        }

        if (sp->m_handle.Local_sets[0])
        {
            _sets[set_count++] = sp->m_handle.Local_sets[m_device->m_imageIndex];
        }

        vkCmdBindDescriptorSets(
            m_device->m_graphicsCommandBuffers[m_device->m_imageIndex].m_handle,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            sp->m_handle.m_layout,
            0,
            set_count,
            _sets,
            0,
            nullptr
        );
    }

}


namespace vk {

    bool __InitializeMaterial(trace::MaterialInstance* mat_instance, Ref<trace::GPipeline> pipeline, trace::Material material)
    {
        bool result = true;

        

        if (!mat_instance || !pipeline.get())
        {
            TRC_ERROR("Please input valid pointer -> {} || {}, Function -> {}", (const void*)mat_instance, (const void*)pipeline.get(), __FUNCTION__);
            return false;
        }

        if (mat_instance->GetRenderHandle()->m_internalData)
        {
            TRC_WARN("This material is valid or has been initialized, {}", (const void*)mat_instance->GetRenderHandle()->m_internalData);
            return false;
        }

        if (!pipeline->GetRenderHandle()->m_internalData)
        {
            TRC_ERROR("Invalid render handle, {} || {}, Function -> {}", (const void*)mat_instance->GetRenderHandle()->m_internalData, (const void*)pipeline->GetRenderHandle()->m_internalData, __FUNCTION__);
            return false;
        }

        trace::VKMaterialData* _handle = new trace::VKMaterialData(); //TODO: Use a custom allocator
        _handle->m_device = &g_VkDevice;
        _handle->m_instance = &g_Vkhandle;
        trace::VKHandle* _instance = (trace::VKHandle*)_handle->m_instance;
        trace::VKDeviceHandle* _device = (trace::VKDeviceHandle*)_handle->m_device;
        mat_instance->m_renderPipeline = pipeline;
        mat_instance->GetRenderHandle()->m_internalData = _handle;

        trace::VKPipeline* sp = (trace::VKPipeline*)pipeline->GetRenderHandle()->m_internalData;
        mat_instance->m_material.m_albedoMap = material.m_albedoMap;
        mat_instance->m_material.m_diffuseColor = material.m_diffuseColor;
        mat_instance->m_material.m_normalMap = material.m_normalMap;
        mat_instance->m_material.m_shininess = material.m_shininess;
        mat_instance->m_material.m_specularMap = material.m_specularMap;

        mat_instance->m_shaderData[trace::ShaderData::MATERIAL_ALBEDO] = std::make_pair(nullptr, INVALID_ID);
        mat_instance->m_shaderData[trace::ShaderData::MATERIAL_DIFFUSE_COLOR] = std::make_pair(nullptr, INVALID_ID);
        mat_instance->m_shaderData[trace::ShaderData::MATERIAL_NORMAL] = std::make_pair(nullptr, INVALID_ID);
        mat_instance->m_shaderData[trace::ShaderData::MATERIAL_SHININESS] = std::make_pair(nullptr, INVALID_ID);
        mat_instance->m_shaderData[trace::ShaderData::MATERIAL_SPECULAR] = std::make_pair(nullptr, INVALID_ID);


        if (!sp->Instance_sets[0])
        {
            TRC_TRACE("The render pipeline assigned to these material does not use any material data");
            return true;
        }

        VkDescriptorSetLayout layouts[] = {
            sp->Instance_layout,
            sp->Instance_layout,
            sp->Instance_layout
        };

        VkDescriptorSetAllocateInfo alloc_info = {};
        alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        alloc_info.descriptorPool = sp->Instance_pool;
        alloc_info.descriptorSetCount = 3;
        alloc_info.pSetLayouts = layouts;

        VkResult _result = vkAllocateDescriptorSets(
            _device->m_device,
            &alloc_info,
            _handle->m_sets
        );

        VK_ASSERT(_result);

        trace::PipelineStateDesc& desc = mat_instance->m_renderPipeline->GetDesc();

        std::vector<trace::ShaderResourceBinding>& res = desc.resource_bindings;
        std::unordered_map<trace::ShaderData, std::pair<void*, uint32_t>>& _shaderData = mat_instance->m_shaderData;


        for (uint32_t i = 0; i < 3; i++)
        {
            std::vector<VkWriteDescriptorSet> _writes;
            std::vector<VkCopyDescriptorSet> _copies;
            for (uint32_t j = 0; j < desc.resource_bindings_count; j++)
            {
                if (res[j].resource_stage == trace::ShaderResourceStage::RESOURCE_STAGE_INSTANCE)
                {
                    VkWriteDescriptorSet write = {};
                    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    write.descriptorCount = 1;
                    write.dstArrayElement = res[j].index;
                    write.dstBinding = res[j].slot;
                    write.dstSet = _handle->m_sets[i];
                    switch (res[j].resource_data_type)
                    {
                    case trace::ShaderData::MATERIAL_ALBEDO:
                    {
                        _shaderData[trace::ShaderData::MATERIAL_ALBEDO] = std::make_pair(mat_instance->m_material.m_albedoMap.get(), pipeline->_hashTable.Get(res[j].resource_name));
                        trace::VKImage* tex_handle = (trace::VKImage*)mat_instance->m_material.m_albedoMap.get()->GetRenderHandle()->m_internalData;
                        VkDescriptorImageInfo img_info = {};
                        img_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                        img_info.imageView = tex_handle->m_view;
                        img_info.sampler = tex_handle->m_sampler;
                        write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                        write.pImageInfo = &img_info;
                        _writes.push_back(write);
                        break;
                    }
                    case trace::ShaderData::MATERIAL_DIFFUSE_COLOR:
                    {
                        _shaderData[trace::ShaderData::MATERIAL_DIFFUSE_COLOR] = std::make_pair(&mat_instance->m_material.m_diffuseColor, pipeline->_hashTable.Get(res[j].resource_name));
                        VkCopyDescriptorSet copy = {};
                        copy.sType = VK_STRUCTURE_TYPE_COPY_DESCRIPTOR_SET;
                        copy.descriptorCount = 1;
                        copy.dstArrayElement = res[j].index;
                        copy.dstBinding = res[j].slot;
                        copy.dstSet = _handle->m_sets[i];
                        copy.srcArrayElement = res[j].index;
                        copy.srcBinding = res[j].slot;
                        copy.srcSet = sp->Instance_sets[i];
                        _copies.push_back(copy);
                        break;
                    }
                    case trace::ShaderData::MATERIAL_NORMAL:
                    {
                        _shaderData[trace::ShaderData::MATERIAL_NORMAL] = std::make_pair(mat_instance->m_material.m_normalMap.get(), pipeline->_hashTable.Get(res[j].resource_name));
                        trace::VKImage* tex_handle = (trace::VKImage*)mat_instance->m_material.m_normalMap.get()->GetRenderHandle()->m_internalData;
                        VkDescriptorImageInfo img_info = {};
                        img_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                        img_info.imageView = tex_handle->m_view;
                        img_info.sampler = tex_handle->m_sampler;
                        write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                        write.pImageInfo = &img_info;
                        _writes.push_back(write);
                        break;
                    }
                    case trace::ShaderData::MATERIAL_SHININESS:
                    {
                        _shaderData[trace::ShaderData::MATERIAL_SHININESS] = std::make_pair(&mat_instance->m_material.m_shininess, pipeline->_hashTable.Get(res[j].resource_name));
                        VkCopyDescriptorSet copy = {};
                        copy.sType = VK_STRUCTURE_TYPE_COPY_DESCRIPTOR_SET;
                        copy.descriptorCount = 1;
                        copy.dstArrayElement = res[j].index;
                        copy.dstBinding = res[j].slot;
                        copy.dstSet = _handle->m_sets[i];
                        copy.srcArrayElement = res[j].index;
                        copy.srcBinding = res[j].slot;
                        copy.srcSet = sp->Instance_sets[i];
                        _copies.push_back(copy);
                        break;
                    }
                    case trace::ShaderData::MATERIAL_SPECULAR:
                    {
                        _shaderData[trace::ShaderData::MATERIAL_SPECULAR] = std::make_pair(mat_instance->m_material.m_specularMap.get(), pipeline->_hashTable.Get(res[j].resource_name));
                        trace::VKImage* tex_handle = (trace::VKImage*)mat_instance->m_material.m_specularMap.get()->GetRenderHandle()->m_internalData;
                        VkDescriptorImageInfo img_info = {};
                        img_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                        img_info.imageView = tex_handle->m_view;
                        img_info.sampler = tex_handle->m_sampler;
                        write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                        write.pImageInfo = &img_info;
                        _writes.push_back(write);
                        break;
                    }
                    }
                }
            }
            vkUpdateDescriptorSets(
                _device->m_device,
                static_cast<uint32_t>(_writes.size()),
                _writes.data(),
                static_cast<uint32_t>(_copies.size()),
                _copies.data()
            );
        }



        return result;
    }
    bool __ApplyMaterial(trace::MaterialInstance* mat_instance)
    {
        bool result = true;

        

        if (!mat_instance)
        {
            TRC_ERROR("Please input valid pointer -> {}, Function -> {}", (const void*)mat_instance, __FUNCTION__);
            return false;
        }

        if (!mat_instance->GetRenderHandle()->m_internalData)
        {
            TRC_ERROR("Invalid render handle, {} || {}, Function -> {}", (const void*)mat_instance->GetRenderHandle()->m_internalData, __FUNCTION__);
            return false;
        }

        trace::VKMaterialData* _handle = (trace::VKMaterialData*)mat_instance->GetRenderHandle()->m_internalData;
        trace::VKHandle* _instance = (trace::VKHandle*)_handle->m_instance;
        trace::VKDeviceHandle* _device = (trace::VKDeviceHandle*)_handle->m_device;


        trace::GPipeline* pipeline = mat_instance->m_renderPipeline.get();
        trace::VKPipeline* sp = (trace::VKPipeline*)pipeline->GetRenderHandle()->m_internalData;
        std::unordered_map<trace::ShaderData, std::pair<void*, uint32_t>>& _shaderData = mat_instance->m_shaderData;



        if (_shaderData[trace::ShaderData::MATERIAL_DIFFUSE_COLOR].first)
        {
            void* _src = _shaderData[trace::ShaderData::MATERIAL_DIFFUSE_COLOR].first;
            uint32_t hash_ = _shaderData[trace::ShaderData::MATERIAL_DIFFUSE_COLOR].second;
            trace::UniformMetaData& _meta_data = pipeline->Scene_uniforms[hash_];
            void* _map_point = sp->cache_data + _meta_data._offset;

            memcpy(_map_point, _src, sizeof(glm::vec4));
        }
        if (_shaderData[trace::ShaderData::MATERIAL_SHININESS].first)
        {
            void* _src = _shaderData[trace::ShaderData::MATERIAL_SHININESS].first;
            uint32_t hash_ = _shaderData[trace::ShaderData::MATERIAL_SHININESS].second;
            trace::UniformMetaData& _meta_data = pipeline->Scene_uniforms[hash_];
            void* _map_point = sp->cache_data + _meta_data._offset;

            memcpy(_map_point, _src, sizeof(float));
        }

        uint32_t set_count = 0;
        VkDescriptorSet _sets[3];

        if (sp->Scene_sets[0])
        {
            _sets[set_count++] = sp->Scene_sets[_device->m_imageIndex];
        }

        if (sp->Instance_sets[0])
        {
            _sets[set_count++] = _handle->m_sets[_device->m_imageIndex];
        }

        if (sp->Local_sets[0])
        {
            _sets[set_count++] = sp->Local_sets[_device->m_imageIndex];
        }

        vkCmdBindDescriptorSets(
            _device->m_graphicsCommandBuffers[_device->m_imageIndex].m_handle,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            sp->m_layout,
            0,
            set_count,
            _sets,
            0,
            nullptr
        );
        

        return result;
    }

}
