#include "pch.h"

#include "VulkanPipeline.h"
#include "VkUtils.h"
#include "VulkanRenderPass.h"
#include "VulkanTexture.h"
#include "core/memory/memory.h"
#include "render/Renderutils.h"

	extern trace::VKHandle g_Vkhandle;
	extern trace::VKDeviceHandle g_VkDevice;
namespace trace {

	
	bool processShaderResources(
		uint32_t& resource_bindings_count,
		ShaderResourceBinding* resource_bindings,
		VulkanPipeline*_pipeline,
		HashTable<uint32_t>& _hashTable,
		VKHandle* instance,
		VKDeviceHandle* device,
		uint32_t& out_size
		);

	VulkanPipeline::VulkanPipeline(PipelineStateDesc desc)
	{
		m_desc = desc;
		m_handle = {};
		m_instance = &g_Vkhandle;
		m_device = &g_VkDevice;

		_hashTable.Init(512);// TODO: let number be configurable or more dynamic

		uint32_t _ids = INVALID_ID;
		_hashTable.Fill(_ids);

		VkViewport viewport = {};
		viewport.x = 0;
		viewport.y = desc.view_port.y;
		viewport.width = desc.view_port.width;
		viewport.height = -desc.view_port.height;
		viewport.minDepth = desc.view_port.minDepth;
		viewport.maxDepth = desc.view_port.maxDepth;

		VkRect2D scissor = {};
		scissor.offset.x = scissor.offset.y = 0;
		scissor.extent.width = static_cast<uint32_t>(desc.view_port.width);
		scissor.extent.height = static_cast<uint32_t>(desc.view_port.height);

		VulkanRenderPass* pass = reinterpret_cast<VulkanRenderPass*>(desc.render_pass);

		VkResult pipeline_result = vk::_CreatePipeline(
			m_instance,
			m_device,
			1,
			&viewport,
			1,
			&scissor,
			desc,
			&m_handle,
			&pass->m_handle,
			desc.subpass_index
		);

		if (pipeline_result == VK_SUCCESS)
		{
			TRC_INFO("Pipeline created  |__// ...");
		}
	}

	VulkanPipeline::~VulkanPipeline()
	{
		if(m_handle.m_handle)
			Shutdown();

	}

	bool VulkanPipeline::Initialize()
	{
		uint32_t total_size_global = 0;
		uint32_t& resource_bindings_count = m_desc.resource_bindings_count;

		std::vector<ShaderResourceBinding>& resource_bindings = m_desc.resource_bindings;
		
		uint32_t map_data_size;


		bool result = processShaderResources(
			resource_bindings_count,
			resource_bindings.data(),
			this,
			_hashTable,
			m_instance,
			m_device,
			map_data_size
		);

		//_mapped_data = (char*)AllocAligned(map_data_size, m_device->m_properties.limits.minMemoryMapAlignment);

		vkMapMemory(
			m_device->m_device,
			m_handle.Scene_buffers.m_memory,
			0,
			map_data_size,
			0,
			(void**) &cache_data
		);

		return result;
	}

	void VulkanPipeline::SetData(ShaderResourceStage resource_scope, void* data, uint32_t size, uint32_t slot, uint32_t index)
	{
	}

	void VulkanPipeline::SetTextureData(ShaderResourceStage resource_scope, GTexture* texture, uint32_t slot, uint32_t index)
	{
	}

	void VulkanPipeline::SetData(const std::string& resource_name, ShaderResourceStage resource_scope, void* data, uint32_t size)
	{

		uint32_t hash_id = _hashTable.Get(resource_name.c_str());

		if (hash_id == INVALID_ID)
		{
			TRC_CRITICAL("Can't set value for an invalid resource. please check if pipeline has been initialized");
			return;
		}


		UniformMetaData& meta_data = Scene_uniforms[hash_id];
		if (size > meta_data._size || size < meta_data._size)
			TRC_ERROR("Please ensure data size is not greater than resource data");
		if (resource_scope == ShaderResourceStage::RESOURCE_STAGE_LOCAL)
		{
			vkCmdPushConstants(
				m_device->m_graphicsCommandBuffers[m_device->m_imageIndex].m_handle,
				m_handle.m_layout,
				vk::convertShaderStage(meta_data._shader_stage),
				meta_data._offset,
				meta_data._size,
				data
			);
			return;
		}
		void* map_point = cache_data + meta_data._offset;
		memcpy(map_point, data, meta_data._size);

	}

	void VulkanPipeline::SetTextureData(const std::string& resource_name, ShaderResourceStage resource_scope, GTexture* texture, uint32_t index)
	{

		uint32_t hash_id = _hashTable.Get(resource_name.c_str());

		if (hash_id == INVALID_ID)
		{
			TRC_CRITICAL("Can't set value for an invalid resource. please check if pipeline has been initialized");
			return;
		}

		if (last_tex_update[m_device->m_imageIndex] == texture)
			return;

		VulkanTexture* _tex = reinterpret_cast<VulkanTexture*>(texture);


		VkWriteDescriptorSet write = {};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;


		UniformMetaData& meta_data = Scene_uniforms[hash_id];
		VkDescriptorImageInfo image_info = {};
		image_info.sampler = _tex->m_sampler;
		image_info.imageView = _tex->m_handle.m_view;
		image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		write.descriptorCount = 1; //HACK: Fix
		write.dstBinding = meta_data._slot;
		write.pImageInfo = &image_info;
		write.dstArrayElement = index;
		switch (meta_data._resource_type)
		{
		case ShaderResourceType::SHADER_RESOURCE_TYPE_COMBINED_SAMPLER:
		{
			write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			break;
		}
		}

		switch (resource_scope)
		{


		case ShaderResourceStage::RESOURCE_STAGE_GLOBAL:
		{
			write.dstSet = m_handle.Scene_sets[m_device->m_imageIndex];

			break;
		}
		
		case ShaderResourceStage::RESOURCE_STAGE_INSTANCE:
		{
			write.dstSet = m_handle.Instance_sets[m_device->m_imageIndex];
			break;
		}
		
		case ShaderResourceStage::RESOURCE_STAGE_LOCAL:
		{
			write.dstSet = m_handle.Instance_sets[m_device->m_imageIndex];
			break;
		}


		}

		vkUpdateDescriptorSets(
			m_device->m_device,
			1,
			&write,
			0,
			nullptr
		);
		last_tex_update[m_device->m_imageIndex] = texture;

	}

	void VulkanPipeline::SetMultipleData(ShaderResourceStage resource_scope, void* data, uint32_t size, uint32_t count, uint32_t slot, uint32_t index)
	{
	}

	void VulkanPipeline::SetMultipleTextureData(ShaderResourceStage resource_scope, GTexture* texture, uint32_t count, uint32_t slot, uint32_t index)
	{
	}

	void VulkanPipeline::Bind()
	{
		uint32_t set_count = 0;
		VkDescriptorSet _sets[3];

		if (m_handle.Scene_sets[0])
		{
			_sets[set_count++] = m_handle.Scene_sets[m_device->m_imageIndex];
		}

		if (m_handle.Instance_sets[0])
		{
			_sets[set_count++] = m_handle.Instance_sets[m_device->m_imageIndex];
		}

		if (m_handle.Local_sets[0])
		{
			_sets[set_count++] = m_handle.Local_sets[m_device->m_imageIndex];
		}

		vkCmdBindDescriptorSets(
			m_device->m_graphicsCommandBuffers[m_device->m_imageIndex].m_handle,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			m_handle.m_layout,
			0,
			set_count,
			_sets,
			0,
			nullptr
		);

	}

	void VulkanPipeline::Shutdown()
	{
		vkUnmapMemory(
			m_device->m_device,
			m_handle.Scene_buffers.m_memory
		);
		vkDeviceWaitIdle(m_device->m_device);
		vk::_DestroyPipeline(m_instance, m_device, &m_handle);

		// TODO: Fix bug ___> unknow error source
		//FreeAligned(_mapped_data);

	}


	bool processShaderResources(uint32_t& resource_bindings_count, ShaderResourceBinding* resource_bindings, VulkanPipeline* _pipeline, HashTable<uint32_t>& _hashTable, VKHandle* instance, VKDeviceHandle* device, uint32_t& out_size)
	{
		bool result = false;

		uint32_t total_size_global = 0;
		uint32_t total_size_local = 0;
		_pipeline->Scene_uniforms.resize(resource_bindings_count);

		for (uint32_t i = 0; i < resource_bindings_count; i++)
		{


			uint32_t& hash_id = _hashTable.Get_Ref(resource_bindings[i].resource_name.c_str());
			for (uint32_t j = 0; j < _pipeline->Scene_uniforms.size(); j++)
			{
				if (_pipeline->Scene_uniforms[j]._id == INVALID_ID)
				{

					hash_id = j;
					_pipeline->Scene_uniforms[j]._id = j;
					_pipeline->Scene_uniforms[j]._size = resource_bindings[i].resource_size;
					_pipeline->Scene_uniforms[j]._slot = resource_bindings[i].slot;
					_pipeline->Scene_uniforms[j]._index = resource_bindings[i].index;
					_pipeline->Scene_uniforms[j]._count = resource_bindings[i].count;
					_pipeline->Scene_uniforms[j]._resource_type = resource_bindings[i].resource_type;
					_pipeline->Scene_uniforms[j]._shader_stage = resource_bindings[i].shader_stage;
					if (resource_bindings[i].resource_stage == ShaderResourceStage::RESOURCE_STAGE_LOCAL)
					{
						_pipeline->Scene_uniforms[j]._offset = total_size_local;
						total_size_local += resource_bindings[i].resource_size;
						total_size_local = get_alignment(total_size_local, 16);
						break;
					}
					_pipeline->Scene_uniforms[j]._offset = total_size_global;
					total_size_global += resource_bindings[i].resource_size;
					total_size_global = get_alignment(total_size_global, 16);
					break;
				}
			}



		}

		vk::createBuffer(
			instance,
			device,
			total_size_global,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			_pipeline->m_handle.Scene_buffers.m_handle,
			_pipeline->m_handle.Scene_buffers.m_memory
		);

		VkDescriptorBufferInfo bufs[128];
		for (uint32_t i = 0; i < 3; i++)
		{
			std::vector<VkWriteDescriptorSet> _writes;
			uint32_t block_size = 0;
			uint32_t block_offset = INVALID_ID;

			for (uint32_t j = 0; j < resource_bindings_count; j++)
			{
				VkWriteDescriptorSet write = {};
				write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				write.descriptorCount = 1;
				write.dstArrayElement = resource_bindings[j].index;
				write.dstBinding = resource_bindings[j].slot;
				write.pBufferInfo = nullptr;
				write.pImageInfo = nullptr;
				write.pNext = nullptr;
				write.pTexelBufferView = nullptr;
				if (resource_bindings[j].resource_stage == ShaderResourceStage::RESOURCE_STAGE_GLOBAL)
				{
					
					write.dstSet = _pipeline->m_handle.Scene_sets[i];
				}
				else if (resource_bindings[j].resource_stage == ShaderResourceStage::RESOURCE_STAGE_INSTANCE)
				{
					write.dstSet = _pipeline->m_handle.Instance_sets[i];
				}
				else if (resource_bindings[j].resource_stage == ShaderResourceStage::RESOURCE_STAGE_LOCAL)
				{
					continue;
					//write.dstSet = _pipeline->m_handle.Local_sets[i];
				}

				if (resource_bindings[j].resource_type == ShaderResourceType::SHADER_RESOURCE_TYPE_UNIFORM_BUFFER)
				{
					uint32_t _offset = _pipeline->Scene_uniforms[_hashTable.Get(resource_bindings[j].resource_name)]._offset;
					block_offset = _offset < block_offset ? _offset : block_offset;
					if(j < (resource_bindings_count - 1))
					{ 
						if (resource_bindings[j] == resource_bindings[j+1])
						{
							block_size += _pipeline->Scene_uniforms[_hashTable.Get(resource_bindings[j].resource_name)]._size;
							block_size = get_alignment(block_size, 16);
							continue;
						}
					}
					
					block_size += _pipeline->Scene_uniforms[_hashTable.Get(resource_bindings[j].resource_name)]._size;
					block_size = get_alignment(block_size, 16);
					
					
					bufs[j] = {};
					bufs[j].buffer = _pipeline->m_handle.Scene_buffers.m_handle;
					bufs[j].offset = block_offset;
					bufs[j].range = block_size;
					write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
					write.pBufferInfo = &bufs[j];
					_writes.push_back(write);
					block_offset = INVALID_ID;
					block_size = 0;
				}
				if (resource_bindings[j].resource_type == ShaderResourceType::SHADER_RESOURCE_TYPE_COMBINED_SAMPLER)
				{
					if (resource_bindings[j].resource_data_type == ShaderData::CUSTOM_DATA_TEXTURE && resource_bindings[j].data)
					{
						VulkanTexture* tex = (VulkanTexture*)resource_bindings[j].data;
						VkDescriptorImageInfo img_info = {};
						img_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
						img_info.imageView = tex->m_handle.m_view;
						img_info.sampler = tex->m_sampler;
						write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
						write.pImageInfo = &img_info;
						_writes.push_back(write);
					}
				}
			}

			vkUpdateDescriptorSets(
				device->m_device,
				static_cast<uint32_t>(_writes.size()),
				_writes.data(),
				0,
				nullptr
			);
		}
		out_size = total_size_global;
		result = true;
		return result;
	}

}

namespace vk {

	bool __CreatePipeline(trace::GPipeline* pipeline, trace::PipelineStateDesc description)
	{
		bool result = true;

		TRC_INFO(__FUNCTION__);

		if (!pipeline)
		{
			TRC_ERROR("Please input valid buffer pointer -> {}, Function -> {}", (const void*)pipeline, __FUNCTION__);
			return false;
		}

		if (!pipeline->GetRenderHandle()->m_internalData)
		{
			TRC_ERROR("Invalid render handle, {}, Function -> {}", (const void*)pipeline->GetRenderHandle()->m_internalData, __FUNCTION__);
			return false;
		}

		trace::VKPipeline* _handle = new trace::VKPipeline(); //TODO: Use a custom allocator
		_handle->m_device = &g_VkDevice;
		_handle->m_instance = &g_Vkhandle;
		trace::VKHandle* _instance = (trace::VKHandle*)_handle->m_instance;
		trace::VKDeviceHandle* _device = (trace::VKDeviceHandle*)_handle->m_device;




		return result;
	}
	bool __DestroyPipeline(trace::GPipeline* pipeline)
	{
		bool result = true;

		TRC_INFO(__FUNCTION__);

		if (!pipeline)
		{
			TRC_ERROR("Please input valid buffer pointer -> {}, Function -> {}", (const void*)pipeline, __FUNCTION__);
			return false;
		}

		if (!pipeline->GetRenderHandle()->m_internalData)
		{
			TRC_ERROR("Invalid render handle, {}, Function -> {}", (const void*)pipeline->GetRenderHandle()->m_internalData, __FUNCTION__);
			return false;
		}

		trace::VKPipeline* _handle = (trace::VKPipeline*)pipeline->GetRenderHandle()->m_internalData;
		trace::VKHandle* _instance = (trace::VKHandle*)_handle->m_instance;
		trace::VKDeviceHandle* _device = (trace::VKDeviceHandle*)_handle->m_device;




		return result;
	}
	bool __InitializePipeline(trace::GPipeline* pipeline)
	{
		bool result = true;

		TRC_INFO(__FUNCTION__);

		if (!pipeline)
		{
			TRC_ERROR("Please input valid buffer pointer -> {}, Function -> {}", (const void*)pipeline, __FUNCTION__);
			return false;
		}

		if (!pipeline->GetRenderHandle()->m_internalData)
		{
			TRC_ERROR("Invalid render handle, {}, Function -> {}", (const void*)pipeline->GetRenderHandle()->m_internalData, __FUNCTION__);
			return false;
		}

		trace::VKPipeline* _handle = (trace::VKPipeline*)pipeline->GetRenderHandle()->m_internalData;
		trace::VKHandle* _instance = (trace::VKHandle*)_handle->m_instance;
		trace::VKDeviceHandle* _device = (trace::VKDeviceHandle*)_handle->m_device;




		return result;
	}
	bool __ShutDownPipeline(trace::GPipeline* pipeline)
	{
		bool result = true;

		TRC_INFO(__FUNCTION__);

		if (!pipeline)
		{
			TRC_ERROR("Please input valid buffer pointer -> {}, Function -> {}", (const void*)pipeline, __FUNCTION__);
			return false;
		}

		if (!pipeline->GetRenderHandle()->m_internalData)
		{
			TRC_ERROR("Invalid render handle, {}, Function -> {}", (const void*)pipeline->GetRenderHandle()->m_internalData, __FUNCTION__);
			return false;
		}

		trace::VKPipeline* _handle = (trace::VKPipeline*)pipeline->GetRenderHandle()->m_internalData;
		trace::VKHandle* _instance = (trace::VKHandle*)_handle->m_instance;
		trace::VKDeviceHandle* _device = (trace::VKDeviceHandle*)_handle->m_device;




		return result;
	}
	bool __SetPipelineData(trace::GPipeline* pipeline, const std::string& resource_name, trace::ShaderResourceStage resource_scope, void* data, uint32_t size)
	{
		bool result = true;

		TRC_INFO(__FUNCTION__);

		if (!pipeline)
		{
			TRC_ERROR("Please input valid buffer pointer -> {}, Function -> {}", (const void*)pipeline, __FUNCTION__);
			return false;
		}

		if (!pipeline->GetRenderHandle()->m_internalData)
		{
			TRC_ERROR("Invalid render handle, {}, Function -> {}", (const void*)pipeline->GetRenderHandle()->m_internalData, __FUNCTION__);
			return false;
		}

		trace::VKPipeline* _handle = (trace::VKPipeline*)pipeline->GetRenderHandle()->m_internalData;
		trace::VKHandle* _instance = (trace::VKHandle*)_handle->m_instance;
		trace::VKDeviceHandle* _device = (trace::VKDeviceHandle*)_handle->m_device;




		return result;
	}
	bool __SetPipelineTextureData(trace::const std::string& resource_name, trace::ShaderResourceStage resource_scope, trace::GTexture* texture, uint32_t index = 0)
	{
		bool result = true;

		TRC_INFO(__FUNCTION__);

		if (!pipeline)
		{
			TRC_ERROR("Please input valid buffer pointer -> {}, Function -> {}", (const void*)pipeline, __FUNCTION__);
			return false;
		}

		if (!pipeline->GetRenderHandle()->m_internalData)
		{
			TRC_ERROR("Invalid render handle, {}, Function -> {}", (const void*)pipeline->GetRenderHandle()->m_internalData, __FUNCTION__);
			return false;
		}

		trace::VKPipeline* _handle = (trace::VKPipeline*)pipeline->GetRenderHandle()->m_internalData;
		trace::VKHandle* _instance = (trace::VKHandle*)_handle->m_instance;
		trace::VKDeviceHandle* _device = (trace::VKDeviceHandle*)_handle->m_device;




		return result;
	}
	bool __BindPipeline(trace::GPipeline* pipeline);

}