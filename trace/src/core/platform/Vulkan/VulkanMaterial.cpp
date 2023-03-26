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
		/*for (uint32_t j = 0; j < 3; j++)
		{
			eastl::vector<VkWriteDescriptorSet> _writes;


			for (uint32_t i = 0; i < desc.resource_bindings_count; i++)
			{

				switch (res[i].resource_stage)
				{
				case ShaderResourceStage::RESOURCE_STAGE_INSTANCE:
				{
					switch (res[i].resource_type)
					{
					case ShaderResourceType::SHADER_RESOURCE_TYPE_UNIFORM_BUFFER:
					{
						VkDescriptorBufferInfo buffer_info = {};
						buffer_info.buffer = sp->m_handle.Scene_buffers.m_handle;
						buffer_info.offset = sp->Scene_uniforms[sp->_hashTable.Get(res[i].resource_name.data())]._offset;
						buffer_info.range = sp->Scene_uniforms[sp->_hashTable.Get(res[i].resource_name.data())]._size;

						VkWriteDescriptorSet write = {};
						write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
						write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
						write.descriptorCount = res[i].count;
						write.dstArrayElement = res[i].index;
						write.dstBinding = res[i].slot;
						write.dstSet = m_sets[j];
						write.pBufferInfo = &buffer_info;

						_writes.push_back(write);

						break;
					}

                    case ShaderResourceType::SHADER_RESOURCE_TYPE_COMBINED_SAMPLER:
                    {
                        VkDescriptorImageInfo img_info[3] = {};
                        uint32_t img_count = 1;
                        if (res[i].count >= 3)
                        {
                            img_info[0].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                            img_info[0].imageView = ((VulkanTexture*)m_material.m_albedoMap.get())->m_handle.m_view;
                            img_info[0].sampler = ((VulkanTexture*)m_material.m_albedoMap.get())->m_sampler;

                            img_info[1].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                            img_info[1].imageView = ((VulkanTexture*)m_material.m_specularMap.get())->m_handle.m_view;
                            img_info[1].sampler = ((VulkanTexture*)m_material.m_specularMap.get())->m_sampler;

                            img_info[2].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                            img_info[2].imageView = ((VulkanTexture*)m_material.m_normalMap.get())->m_handle.m_view;
                            img_info[2].sampler = ((VulkanTexture*)m_material.m_normalMap.get())->m_sampler;
                            img_count = 3;
                        }

                        VkWriteDescriptorSet write = {};
                        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                        write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                        write.descriptorCount = img_count;
                        write.dstArrayElement = res[i].index;
                        write.dstBinding = res[i].slot;
                        write.dstSet = m_sets[j];
                        write.pImageInfo = img_info;
                        _writes.push_back(write);


                        break;
                    }
					}
					break;
				}
				}



			}
            vkUpdateDescriptorSets(
                m_device->m_device,
                _writes.size(),
                _writes.data(),
                0,
                nullptr
            );
		}*/


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
        //MaterialRenderData mrd = {};
        //mrd.diffuse_color = m_material.m_diffuseColor;
        //mrd.shininess = m_material.m_shininess;
        VulkanPipeline* sp = (VulkanPipeline*)m_renderPipeline.get();
        //uint32_t hash_id = sp->_hashTable.Get("instance_data");
        //
        //if (hash_id == INVALID_ID)
        //{
        //    TRC_CRITICAL("Can't set value for an invalid resource. please check if pipeline has been initialized");
        //    return;
        //}

        //UniformMetaData& meta_data = sp->Scene_uniforms[hash_id];
        //void* map_point = sp->cache_data + meta_data._offset;
        //
        //memcpy(map_point, &mrd, sizeof(MaterialRenderData));
        

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
