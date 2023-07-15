#include "pch.h"
#include "VulkanMaterial.h"
#include "VulkanPipeline.h"
#include "VulkanTexture.h"

extern trace::VKHandle g_Vkhandle;
extern trace::VKDeviceHandle g_VkDevice;


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

        std::unordered_map<trace::ShaderData, std::pair<void*, uint32_t>>& _shaderData = mat_instance->m_shaderData;


        for (uint32_t i = 0; i < 3; i++)
        {
            std::vector<VkWriteDescriptorSet> _writes;
            std::vector<VkCopyDescriptorSet> _copies;
    
            for (auto& k : desc.resources.resources)
            {
                bool is_struct = k.def == trace::ShaderDataDef::STRUCTURE;
                bool is_array = k.def == trace::ShaderDataDef::ARRAY;
                bool is_varible = k.def == trace::ShaderDataDef::VARIABLE;

                trace::ShaderResourceStage res_stage = trace::ShaderResourceStage::RESOURCE_STAGE_NONE;
                if (is_struct) res_stage = k._struct.resource_stage;
                else if (is_array) res_stage = k._array.resource_stage;
                else if (is_varible) res_stage = k._variable.resource_stage;

                if (is_struct)
                {
                    for (auto& mem : k._struct.members)
                    {
                        if (k._struct.resource_stage == trace::ShaderResourceStage::RESOURCE_STAGE_INSTANCE)
                        {
                            VkWriteDescriptorSet write = {};
                            write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                            write.descriptorCount = 1;
                            write.dstArrayElement = k._struct.index;
                            write.dstBinding = k._struct.slot;
                            write.dstSet = _handle->m_sets[i];
                            switch (mem.resource_data_type)
                            {
                            case trace::ShaderData::MATERIAL_ALBEDO:
                            {
                                _shaderData[trace::ShaderData::MATERIAL_ALBEDO] = std::make_pair(mat_instance->m_material.m_albedoMap.get(), pipeline->_hashTable.Get(mem.resource_name));
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
                                _shaderData[trace::ShaderData::MATERIAL_DIFFUSE_COLOR] = std::make_pair(&mat_instance->m_material.m_diffuseColor, pipeline->_hashTable.Get(mem.resource_name));
                                VkCopyDescriptorSet copy = {};
                                copy.sType = VK_STRUCTURE_TYPE_COPY_DESCRIPTOR_SET;
                                copy.descriptorCount = 1;
                                copy.dstArrayElement = k._struct.index;
                                copy.dstBinding = k._struct.slot;
                                copy.dstSet = _handle->m_sets[i];
                                copy.srcArrayElement = k._struct.index;
                                copy.srcBinding = k._struct.slot;
                                copy.srcSet = sp->Instance_sets[i];
                                _copies.push_back(copy);
                                break;
                            }
                            case trace::ShaderData::MATERIAL_NORMAL:
                            {
                                _shaderData[trace::ShaderData::MATERIAL_NORMAL] = std::make_pair(mat_instance->m_material.m_normalMap.get(), pipeline->_hashTable.Get(mem.resource_name));
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
                                _shaderData[trace::ShaderData::MATERIAL_SHININESS] = std::make_pair(&mat_instance->m_material.m_shininess, pipeline->_hashTable.Get(mem.resource_name));
                                VkCopyDescriptorSet copy = {};
                                copy.sType = VK_STRUCTURE_TYPE_COPY_DESCRIPTOR_SET;
                                copy.descriptorCount = 1;
                                copy.dstArrayElement = k._struct.index;
                                copy.dstBinding = k._struct.slot;
                                copy.dstSet = _handle->m_sets[i];
                                copy.srcArrayElement = k._struct.index;
                                copy.srcBinding = k._struct.slot;
                                copy.srcSet = sp->Instance_sets[i];
                                _copies.push_back(copy);
                                break;
                            }
                            case trace::ShaderData::MATERIAL_SPECULAR:
                            {
                                _shaderData[trace::ShaderData::MATERIAL_SPECULAR] = std::make_pair(mat_instance->m_material.m_specularMap.get(), pipeline->_hashTable.Get(mem.resource_name));
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
                }
                if (is_array)
                {
                    for (auto& mem : k._array.members)
                    {
                        if (k._array.resource_stage == trace::ShaderResourceStage::RESOURCE_STAGE_INSTANCE)
                        {
                            VkWriteDescriptorSet write = {};
                            write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                            write.descriptorCount = 1;
                            write.dstArrayElement = mem.index;
                            write.dstBinding = k._array.slot;
                            write.dstSet = _handle->m_sets[i];
                            switch (mem.resource_data_type)
                            {
                            case trace::ShaderData::MATERIAL_ALBEDO:
                            {
                                _shaderData[trace::ShaderData::MATERIAL_ALBEDO] = std::make_pair(mat_instance->m_material.m_albedoMap.get(), pipeline->_hashTable.Get(mem.resource_name));
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
                                _shaderData[trace::ShaderData::MATERIAL_DIFFUSE_COLOR] = std::make_pair(&mat_instance->m_material.m_diffuseColor, pipeline->_hashTable.Get(mem.resource_name));
                                VkCopyDescriptorSet copy = {};
                                copy.sType = VK_STRUCTURE_TYPE_COPY_DESCRIPTOR_SET;
                                copy.descriptorCount = 1;
                                copy.dstArrayElement = mem.index;
                                copy.dstBinding = k._array.slot;
                                copy.dstSet = _handle->m_sets[i];
                                copy.srcArrayElement = mem.index;
                                copy.srcBinding = k._array.slot;
                                copy.srcSet = sp->Instance_sets[i];
                                _copies.push_back(copy);
                                break;
                            }
                            case trace::ShaderData::MATERIAL_NORMAL:
                            {
                                _shaderData[trace::ShaderData::MATERIAL_NORMAL] = std::make_pair(mat_instance->m_material.m_normalMap.get(), pipeline->_hashTable.Get(mem.resource_name));
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
                                _shaderData[trace::ShaderData::MATERIAL_SHININESS] = std::make_pair(&mat_instance->m_material.m_shininess, pipeline->_hashTable.Get(mem.resource_name));
                                VkCopyDescriptorSet copy = {};
                                copy.sType = VK_STRUCTURE_TYPE_COPY_DESCRIPTOR_SET;
                                copy.descriptorCount = 1;
                                copy.dstArrayElement = mem.index;
                                copy.dstBinding = k._array.slot;
                                copy.dstSet = _handle->m_sets[i];
                                copy.srcArrayElement = mem.index;
                                copy.srcBinding = k._array.slot;
                                copy.srcSet = sp->Instance_sets[i];
                                _copies.push_back(copy);
                                break;
                            }
                            case trace::ShaderData::MATERIAL_SPECULAR:
                            {
                                _shaderData[trace::ShaderData::MATERIAL_SPECULAR] = std::make_pair(mat_instance->m_material.m_specularMap.get(), pipeline->_hashTable.Get(mem.resource_name));
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
            char* data_point = _device->m_bufferData;
            uint32_t location = pipeline->Scence_struct[_meta_data._struct_index].second + _meta_data._offset;

            void* map_point = data_point + location;
            memcpy(map_point, &mat_instance->m_material.m_diffuseColor, _meta_data._size);

        }
        if (_shaderData[trace::ShaderData::MATERIAL_SHININESS].first)
        {
            void* _src = _shaderData[trace::ShaderData::MATERIAL_SHININESS].first;
            uint32_t hash_ = _shaderData[trace::ShaderData::MATERIAL_SHININESS].second;
            trace::UniformMetaData& _meta_data = pipeline->Scene_uniforms[hash_];
            char* data_point = _device->m_bufferData;
            uint32_t location = pipeline->Scence_struct[_meta_data._struct_index].second + _meta_data._offset;

            void* map_point = data_point + location;
            memcpy(map_point, &mat_instance->m_material.m_shininess, _meta_data._size);
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

        uint32_t offset_count = 0;
        uint32_t offsets[12] = {};

        for (auto& stct : mat_instance->GetRenderPipline()->Scence_struct)
        {
            offsets[offset_count++] = stct.second;
        }

        vkCmdBindDescriptorSets(
            _device->m_graphicsCommandBuffers[_device->m_imageIndex].m_handle,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            sp->m_layout,
            0,
            set_count,
            _sets,
            offset_count,
            offsets
        );
        

        return result;
    }

}
