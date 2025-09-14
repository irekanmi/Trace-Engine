#include "pch.h"
#include "VulkanMaterial.h"
#include "VulkanPipeline.h"
#include "VulkanTexture.h"

extern trace::VKHandle g_Vkhandle;
extern trace::VKDeviceHandle g_VkDevice;


namespace vk {

    bool __InitializeMaterial(trace::MaterialInstance* mat_instance, Ref<trace::GPipeline> pipeline)
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
        mat_instance->SetRenderPipeline(pipeline);
        mat_instance->GetRenderHandle()->m_internalData = _handle;

        trace::VKPipeline* sp = (trace::VKPipeline*)pipeline->GetRenderHandle()->m_internalData;
        

        if (!sp->Instance_sets[0])
        {
            TRC_TRACE("The render pipeline assigned to these material does not use any material data");
            return true;
        }

        /*VkDescriptorSetLayout layouts[] = {
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
        mat_instance->m_renderPipeline = pipeline;*/

        __PostInitializeMaterial(mat_instance, pipeline);



        return result;
    }
    bool __DestroyMaterial(trace::MaterialInstance* mat_instance)
    {
        bool result = true;


        if (!mat_instance)
        {
            TRC_ERROR("Please input valid pointer -> {}, Function -> {}", (const void*)mat_instance, __FUNCTION__);
            return false;
        }

        if (!mat_instance->GetRenderHandle()->m_internalData)
        {
            TRC_WARN("This material is not valid or has not been initialized, {}", (const void*)mat_instance->GetRenderHandle()->m_internalData);
            return false;
        }
        
        //TODO: Implement destruction of descriptor sets
        delete mat_instance->GetRenderHandle()->m_internalData;
        mat_instance->GetRenderHandle()->m_internalData = nullptr;

        return result;
    }
    bool __PostInitializeMaterial(trace::MaterialInstance* mat_instance, Ref<trace::GPipeline> pipeline)
    {
        bool result = true;



        if (!mat_instance || !pipeline.get())
        {
            TRC_ERROR("Please input valid pointer -> {} || {}, Function -> {}", (const void*)mat_instance, (const void*)pipeline.get(), __FUNCTION__);
            return false;
        }

        if (!mat_instance->GetRenderHandle()->m_internalData)
        {
            TRC_WARN("This material is not valid or has not been initialized, {}", (const void*)mat_instance->GetRenderHandle()->m_internalData);
            return false;
        }

        if (!pipeline->GetRenderHandle()->m_internalData)
        {
            TRC_ERROR("Invalid render handle, {} || {}, Function -> {}", (const void*)mat_instance->GetRenderHandle()->m_internalData, (const void*)pipeline->GetRenderHandle()->m_internalData, __FUNCTION__);
            return false;
        }

        trace::VKMaterialData* _handle = (trace::VKMaterialData*)mat_instance->GetRenderHandle()->m_internalData;
        trace::VKHandle* _instance = (trace::VKHandle*)_handle->m_instance;
        trace::VKDeviceHandle* _device = (trace::VKDeviceHandle*)_handle->m_device;
        

        trace::VKPipeline* sp = (trace::VKPipeline*)pipeline->GetRenderHandle()->m_internalData;

        trace::PipelineStateDesc& desc = mat_instance->GetRenderPipline()->GetDesc();



        //for (uint32_t i = 0; i < 3; i++)
        //{
        //    std::vector<VkWriteDescriptorSet> _writes;
        //    std::vector<VkCopyDescriptorSet> _copies;

        //    uint32_t set_index = i;


        //    //New Material Processing
        //    {
        //        uint32_t img_index = 0;
        //        VkDescriptorImageInfo img_infos[16];

        //        for (auto m_data : mat_instance->m_data)
        //        {
        //            trace::UniformMetaData& meta_data = pipeline->Scene_uniforms[m_data.second.second];
        //            if (meta_data._resource_type == trace::ShaderResourceType::SHADER_RESOURCE_TYPE_UNIFORM_BUFFER)
        //            {
        //                VkCopyDescriptorSet copy = {};
        //                copy.sType = VK_STRUCTURE_TYPE_COPY_DESCRIPTOR_SET;
        //                copy.descriptorCount = 1;
        //                copy.dstArrayElement = meta_data._index;
        //                copy.dstBinding = meta_data._slot;
        //                copy.srcArrayElement = meta_data._index;
        //                copy.srcBinding = meta_data._slot;
        //                copy.srcSet = sp->Instance_sets[set_index];
        //                copy.dstSet = _handle->m_sets[i];
        //                _copies.push_back(copy);
        //            }
        //            if (meta_data._resource_type == trace::ShaderResourceType::SHADER_RESOURCE_TYPE_COMBINED_SAMPLER)
        //            {
        //                Ref<trace::GTexture> tex = std::any_cast<Ref<trace::GTexture>>(m_data.second.first);
        //                VkWriteDescriptorSet write = {};
        //                write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        //                write.descriptorCount = 1;
        //                write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        //                write.dstArrayElement = meta_data._index;
        //                write.dstBinding = meta_data._slot;
        //                write.dstSet = _handle->m_sets[i];
        //                trace::VKImage* tex_handle = (trace::VKImage*)tex.get()->GetRenderHandle()->m_internalData;
        //                img_infos[img_index].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        //                img_infos[img_index].imageView = tex_handle->m_view;
        //                img_infos[img_index].sampler = tex_handle->m_sampler;
        //                write.pImageInfo = &img_infos[img_index];
        //                img_index++;
        //                _writes.push_back(write);
        //            }
        //        }

        //    };

        //    vkUpdateDescriptorSets(
        //        _device->m_device,
        //        static_cast<uint32_t>(_writes.size()),
        //        _writes.data(),
        //        static_cast<uint32_t>(_copies.size()),
        //        _copies.data()
        //    );
        //}

        return result;
    }
    bool __ApplyMaterial(trace::MaterialInstance* mat_instance, int32_t render_graph_index)
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


        trace::GPipeline* pipeline = mat_instance->GetRenderPipline().get();
        trace::VKPipeline* sp = (trace::VKPipeline*)pipeline->GetRenderHandle()->m_internalData;


        //New Application
         {
            auto lambda = [&](trace::ShaderData type, std::any& dst, void*& loc)
            {
                switch (type)
                {
                case trace::ShaderData::CUSTOM_DATA_BOOL:    { loc = &std::any_cast<bool&>(dst); break; }
                case trace::ShaderData::CUSTOM_DATA_FLOAT:   { loc = &std::any_cast<float&>(dst); break; }
                case trace::ShaderData::CUSTOM_DATA_INT:     { loc = &std::any_cast<int&>(dst); break; }
                case trace::ShaderData::CUSTOM_DATA_IVEC2:   { loc = &std::any_cast<glm::ivec2&>(dst); break; }
                case trace::ShaderData::CUSTOM_DATA_IVEC3:   { loc = &std::any_cast<glm::ivec3&>(dst); break; }
                case trace::ShaderData::CUSTOM_DATA_IVEC4:   { loc = &std::any_cast<glm::ivec4&>(dst); break; }
                case trace::ShaderData::CUSTOM_DATA_MAT2:    { loc = &std::any_cast<glm::mat2&>(dst); break; }
                case trace::ShaderData::CUSTOM_DATA_MAT3:    { loc = &std::any_cast<glm::mat3&>(dst); break; }
                case trace::ShaderData::CUSTOM_DATA_MAT4:    { loc = &std::any_cast<glm::mat4&>(dst); break; }
                case trace::ShaderData::CUSTOM_DATA_TEXTURE: 
                { 
                    Ref<trace::GTexture>& tex = std::any_cast<Ref<trace::GTexture>&>(dst);
                    loc = (trace::VKImage*)tex->GetRenderHandle()->m_internalData;
                    break;
                }
                case trace::ShaderData::CUSTOM_DATA_VEC2:    { loc = &std::any_cast<glm::vec2&>(dst); break; }
                case trace::ShaderData::CUSTOM_DATA_VEC3:    { loc = &std::any_cast<glm::vec3&>(dst); break; }
                case trace::ShaderData::CUSTOM_DATA_VEC4:    { loc = &std::any_cast<glm::vec4&>(dst); break; }
                }
            };

            for (auto& m_data : mat_instance->GetMaterialData())
            {
                trace::UniformMetaData& meta_data = pipeline->GetSceneUniforms()[m_data.second.hash];

                bool is_buffer = meta_data._resource_type == trace::ShaderResourceType::SHADER_RESOURCE_TYPE_UNIFORM_BUFFER || meta_data._resource_type == trace::ShaderResourceType::SHADER_RESOURCE_TYPE_STORAGE_BUFFER;
                if (is_buffer)
                {
                    void* data = nullptr;
                    lambda(m_data.second.type, m_data.second.internal_data, data);
                    
                    if (data)
                    {
                        __SetPipelineData_Meta(pipeline, meta_data, trace::ShaderResourceStage::RESOURCE_STAGE_INSTANCE, data, meta_data._size, m_data.second.offset, render_graph_index);
                    }
                }
                if (meta_data._resource_type == trace::ShaderResourceType::SHADER_RESOURCE_TYPE_COMBINED_SAMPLER)
                {
                    trace::VKImage* tex = nullptr;
                    lambda(m_data.second.type, m_data.second.internal_data, (void*&)tex);

                    if (tex)
                    {
                        __SetPipelineTextureData_Meta(pipeline, meta_data, trace::ShaderResourceStage::RESOURCE_STAGE_INSTANCE, tex, render_graph_index);
                    }
                }
                
            }
        };


        /*uint32_t set_count = 0;
        VkDescriptorSet _sets[3];

        uint32_t set_index = (_device->m_imageIndex * VK_MAX_DESCRIPTOR_SET_PER_FRAME);
        if (sp->Scene_sets[0])
        {
            _sets[set_count++] = sp->Scene_sets[set_index];
        }

        if (sp->Instance_sets[0])
        {
            _sets[set_count++] = _handle->m_sets[_device->m_imageIndex];
        }

        if (sp->Local_sets[0])
        {
            _sets[set_count++] = sp->Local_sets[set_index];
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
        );*/
        

        return result;
    }

}
