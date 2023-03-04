#include "pch.h"

#include "VulkanPipeline.h"
#include "VkUtils.h"
#include "VulkanRenderPass.h"
#include "VulkanTexture.h"
#include "core/memory/memory.h"

	extern trace::VKHandle g_Vkhandle;
	extern trace::VKDeviceHandle g_VkDevice;
namespace trace {

	bool processShaderBindings(
		uint32_t& resource_bindings_count,
		ShaderResourceBinding* resource_bindings,
		VKPipeline& _pipeline,
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

		uint32_t _ids = INVAILD_ID;
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
		scissor.extent.width = desc.view_port.width;
		scissor.extent.height = desc.view_port.height;

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
	}

	bool VulkanPipeline::Initialize()
	{
		uint32_t total_size_global = 0;
		uint32_t& resource_bindings_count = m_desc.resource_bindings_count;

		ShaderResourceBinding* resource_bindings = m_desc.resource_bindings;
		
		uint32_t map_data_size;

		bool result = processShaderBindings(
			resource_bindings_count,
			resource_bindings,
			m_handle,
			_hashTable,
			m_instance,
			m_device,
			map_data_size
		);

		//_mapped_data = new char[map_data_size];
		_mapped_data = (char*)AllocAligned(map_data_size, m_device->m_properties.limits.minMemoryMapAlignment);

		cache_data = _mapped_data;
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

	void VulkanPipeline::SetData(const eastl::string resource_name, ShaderResourceStage resource_scope, void* data, uint32_t size)
	{

		uint32_t hash_id = _hashTable.Get(resource_name.c_str());

		if (hash_id == INVAILD_ID)
		{
			TRC_CRITICAL("Can't set value for an invalid resource. please check if pipeline has been initialized");
			return;
		}

		UniformMetaData& meta_data = m_handle.Scene_uniforms[hash_id];
		void* map_point = cache_data + meta_data._offset;

		if (size > meta_data._size)
			TRC_ERROR("Please ensure data size is not greater than resource data");

		memcpy(map_point, data, meta_data._size);



	}

	void VulkanPipeline::SetTextureData(const eastl::string resource_name, ShaderResourceStage resource_scope, GTexture* texture, uint32_t index)
	{

		uint32_t hash_id = _hashTable.Get(resource_name.c_str());

		if (hash_id == INVAILD_ID)
		{
			TRC_CRITICAL("Can't set value for an invalid resource. please check if pipeline has been initialized");
			return;
		}
		VulkanTexture* _tex = reinterpret_cast<VulkanTexture*>(texture);


		VkWriteDescriptorSet write = {};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;


		UniformMetaData& meta_data = m_handle.Scene_uniforms[hash_id];
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

	}

	void VulkanPipeline::SetMultipleData(ShaderResourceStage resource_scope, void* data, uint32_t size, uint32_t count, uint32_t slot, uint32_t index)
	{
	}

	void VulkanPipeline::SetMultipleTextureData(ShaderResourceStage resource_scope, GTexture* texture, uint32_t count, uint32_t slot, uint32_t index)
	{
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
		FreeAligned(_mapped_data);

	}

	bool processShaderBindings(uint32_t& resource_bindings_count, ShaderResourceBinding* resource_bindings, VKPipeline& _pipeline, HashTable<uint32_t>& _hashTable, VKHandle* instance, VKDeviceHandle* device, uint32_t& out_size)
	{
		bool result = false;

		uint32_t total_size_global = 0;
		_pipeline.Scene_uniforms.resize(resource_bindings_count);

		for (uint32_t i = 0; i < resource_bindings_count; i++)
		{


			uint32_t& hash_id = _hashTable.Get_Ref(resource_bindings[i].resource_name.c_str());
			for (uint32_t j = 0; j < _pipeline.Scene_uniforms.size(); j++)
			{
				if (_pipeline.Scene_uniforms[j]._id == INVAILD_ID)
				{
					hash_id = j;
					_pipeline.Scene_uniforms[j]._id = j;
					_pipeline.Scene_uniforms[j]._size = resource_bindings[i].resource_size;
					_pipeline.Scene_uniforms[j]._offset = total_size_global;
					_pipeline.Scene_uniforms[j]._slot = resource_bindings[i].slot;
					_pipeline.Scene_uniforms[j]._index = resource_bindings[i].index;
					_pipeline.Scene_uniforms[j]._count = resource_bindings[i].count;
					_pipeline.Scene_uniforms[j]._resource_type = resource_bindings[i].resource_type;
					total_size_global += resource_bindings[i].resource_size;
					total_size_global = get_alignment(total_size_global, device->m_properties.limits.minMemoryMapAlignment);
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
			_pipeline.Scene_buffers.m_handle,
			_pipeline.Scene_buffers.m_memory
		);


		for (uint32_t j = 0; j < 3; j++)
		{
		eastl::vector<VkWriteDescriptorSet> _writes;


			for (uint32_t i = 0; i < resource_bindings_count; i++)
			{

				switch (resource_bindings[i].resource_stage)
				{


				case ShaderResourceStage::RESOURCE_STAGE_GLOBAL:
				{
					switch (resource_bindings[i].resource_type)
					{
					case ShaderResourceType::SHADER_RESOURCE_TYPE_UNIFORM_BUFFER:
					{
						VkDescriptorBufferInfo buffer_info = {};
						buffer_info.buffer = _pipeline.Scene_buffers.m_handle;
						buffer_info.offset = _pipeline.Scene_uniforms[_hashTable.Get(resource_bindings[i].resource_name.data())]._offset;
						buffer_info.range = _pipeline.Scene_uniforms[_hashTable.Get(resource_bindings[i].resource_name.data())]._size;

						VkWriteDescriptorSet write = {};
						write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
						write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
						write.descriptorCount = resource_bindings[i].count;
						write.dstArrayElement = resource_bindings[i].index;
						write.dstBinding = resource_bindings[i].slot;
						write.dstSet = _pipeline.Scene_sets[j];
						write.pBufferInfo = &buffer_info;

						_writes.push_back(write);

						break;
					}
					}
					break;
				}
				
				case ShaderResourceStage::RESOURCE_STAGE_INSTANCE:
				{
					switch (resource_bindings[i].resource_type)
					{
					case ShaderResourceType::SHADER_RESOURCE_TYPE_UNIFORM_BUFFER:
					{
						VkDescriptorBufferInfo buffer_info = {};
						buffer_info.buffer = _pipeline.Scene_buffers.m_handle;
						buffer_info.offset = _pipeline.Scene_uniforms[_hashTable.Get(resource_bindings[i].resource_name.data())]._offset;
						buffer_info.range = _pipeline.Scene_uniforms[_hashTable.Get(resource_bindings[i].resource_name.data())]._size;

						VkWriteDescriptorSet write = {};
						write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
						write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
						write.descriptorCount = resource_bindings[i].count;
						write.dstArrayElement = resource_bindings[i].index;
						write.dstBinding = resource_bindings[i].slot;
						write.dstSet = _pipeline.Instance_sets[j];
						write.pBufferInfo = &buffer_info;

						_writes.push_back(write);

						break;
					}
					}
					break;
				}
				
				case ShaderResourceStage::RESOURCE_STAGE_LOCAL:
				{
					switch (resource_bindings[i].resource_type)
					{
					case ShaderResourceType::SHADER_RESOURCE_TYPE_UNIFORM_BUFFER:
					{
						VkDescriptorBufferInfo buffer_info = {};
						buffer_info.buffer = _pipeline.Scene_buffers.m_handle;
						buffer_info.offset = _pipeline.Scene_uniforms[_hashTable.Get(resource_bindings[i].resource_name.data())]._offset;
						buffer_info.range = _pipeline.Scene_uniforms[_hashTable.Get(resource_bindings[i].resource_name.data())]._size;

						VkWriteDescriptorSet write = {};
						write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
						write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
						write.descriptorCount = resource_bindings[i].count;
						write.dstArrayElement = resource_bindings[i].index;
						write.dstBinding = resource_bindings[i].slot;
						write.dstSet = _pipeline.Local_sets[j];
						write.pBufferInfo = &buffer_info;

						_writes.push_back(write);

						break;
					}
					}
					break;
				}



				}


				
			}

			vkUpdateDescriptorSets(
				device->m_device,
				_writes.size(),
				_writes.data(),
				0,
				nullptr
			);
			
		}
		result = true;
		out_size = total_size_global;

		return result;
	}

}