#include "pch.h"
#include "VulkanMaterial.h"
#include "VulkanPipeline.h"
#include "VulkanTexture.h"
#include "core/platform/Vulkan/VkUtils.h"

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
        
        vk::_CreateDescriptorSetResources(pipeline.get(), sp, _handle->m_set, trace::ShaderResourceStage::RESOURCE_STAGE_INSTANCE);

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

        trace::VKMaterialData* _handle = (trace::VKMaterialData*)mat_instance->GetRenderHandle()->m_internalData;
        trace::VKHandle* _instance = (trace::VKHandle*)_handle->m_instance;
        trace::VKDeviceHandle* _device = (trace::VKDeviceHandle*)_handle->m_device;


        trace::GPipeline* pipeline = mat_instance->GetRenderPipline().get();
        trace::VKPipeline* sp = (trace::VKPipeline*)pipeline->GetRenderHandle()->m_internalData;

        vk::_DestroyDescriptorSetResources(sp, _handle->m_set);
        
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

        vkQueueWaitIdle(_device->m_graphicsQueue);

        trace::PipelineStateDesc& desc = mat_instance->GetRenderPipline()->GetDesc();

        {
            auto lambda = [&](trace::ShaderData type, std::any& dst, void*& loc)
                {
                    switch (type)
                    {
                    case trace::ShaderData::CUSTOM_DATA_BOOL: { loc = &std::any_cast<bool&>(dst); break; }
                    case trace::ShaderData::CUSTOM_DATA_FLOAT: { loc = &std::any_cast<float&>(dst); break; }
                    case trace::ShaderData::CUSTOM_DATA_INT: { loc = &std::any_cast<int&>(dst); break; }
                    case trace::ShaderData::CUSTOM_DATA_IVEC2: { loc = &std::any_cast<glm::ivec2&>(dst); break; }
                    case trace::ShaderData::CUSTOM_DATA_IVEC3: { loc = &std::any_cast<glm::ivec3&>(dst); break; }
                    case trace::ShaderData::CUSTOM_DATA_IVEC4: { loc = &std::any_cast<glm::ivec4&>(dst); break; }
                    case trace::ShaderData::CUSTOM_DATA_MAT2: { loc = &std::any_cast<glm::mat2&>(dst); break; }
                    case trace::ShaderData::CUSTOM_DATA_MAT3: { loc = &std::any_cast<glm::mat3&>(dst); break; }
                    case trace::ShaderData::CUSTOM_DATA_MAT4: { loc = &std::any_cast<glm::mat4&>(dst); break; }
                    case trace::ShaderData::CUSTOM_DATA_TEXTURE:
                    {
                        Ref<trace::GTexture>& tex = std::any_cast<Ref<trace::GTexture>&>(dst);
                        loc = (trace::VKImage*)tex->GetRenderHandle()->m_internalData;
                        break;
                    }
                    case trace::ShaderData::CUSTOM_DATA_VEC2: { loc = &std::any_cast<glm::vec2&>(dst); break; }
                    case trace::ShaderData::CUSTOM_DATA_VEC3: { loc = &std::any_cast<glm::vec3&>(dst); break; }
                    case trace::ShaderData::CUSTOM_DATA_VEC4: { loc = &std::any_cast<glm::vec4&>(dst); break; }
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
                        uint32_t data_size = trace::getShaderDataSize(m_data.second.type);
                        SetPipelineData(pipeline.get(), meta_data, _handle->m_set, trace::ShaderResourceStage::RESOURCE_STAGE_INSTANCE, data, data_size, m_data.second.offset, 0);
                    }
                }
                if (meta_data._resource_type == trace::ShaderResourceType::SHADER_RESOURCE_TYPE_COMBINED_SAMPLER)
                {
                    trace::VKImage* tex = nullptr;
                    lambda(m_data.second.type, m_data.second.internal_data, (void*&)tex);

                    if (tex)
                    {
                        SetPipelineTextureData(pipeline.get(), meta_data, _handle->m_set, trace::ShaderResourceStage::RESOURCE_STAGE_INSTANCE, tex, 0, meta_data._index);
                    }
                }

            }
        };

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
        
        if (!mat_instance->GetRenderPipline()->GetRenderHandle()->m_internalData)
        {
            TRC_ERROR("Invalid render handle, {} || {}, Function -> {}", (const void*)mat_instance->GetRenderHandle()->m_internalData, __FUNCTION__);
            return false;
        }

        trace::VKMaterialData* _handle = (trace::VKMaterialData*)mat_instance->GetRenderHandle()->m_internalData;
        trace::VKHandle* _instance = (trace::VKHandle*)_handle->m_instance;
        trace::VKDeviceHandle* _device = (trace::VKDeviceHandle*)_handle->m_device;


        trace::GPipeline* pipeline = mat_instance->GetRenderPipline().get();
        trace::VKPipeline* sp = (trace::VKPipeline*)pipeline->GetRenderHandle()->m_internalData;


        sp->Instance_set = _handle->m_set.internal_handle;
        
        

        return result;
    }

}
