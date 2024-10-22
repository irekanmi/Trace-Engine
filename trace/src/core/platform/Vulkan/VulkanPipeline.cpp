#include "pch.h"

#include "VulkanPipeline.h"
#include "VkUtils.h"
#include "VulkanRenderPass.h"
#include "VulkanTexture.h"
#include "VulkanBuffer.h"
#include "core/memory/memory.h"
#include "backends/Renderutils.h"
#include "render/GShader.h"

	extern trace::VKHandle g_Vkhandle;
	extern trace::VKDeviceHandle g_VkDevice;

	bool generate_pipeline_resources(trace::VKDeviceHandle* device, trace::GPipeline* pipeline, trace::VKPipeline* handle);
	uint32_t get_type_alignment_std430(trace::ShaderData type);
	uint32_t get_type_alignment_std140(trace::ShaderData type);
	

namespace trace {

	
	

	void update_data_indices(GPipeline* _pipeline, HashTable<uint32_t>& _hashTable, VKPipeline* pipe)
	{
		int count = 0;
		auto lambda = [&](GShader* shader)
		{
			for (auto& i : shader->GetDataIndex())
			{
				int slot = i.second >> 16;
				int index = ((0x0000FFFF) & i.second);
				uint32_t& hash_id = _hashTable.Get_Ref(i.first);
				uint32_t current_id = _pipeline->GetSceneUniforms().size();
				_pipeline->GetSceneUniforms().push_back(trace::UniformMetaData());
				hash_id = current_id;
				_pipeline->GetSceneUniforms()[current_id]._id = current_id;
				_pipeline->GetSceneUniforms()[current_id]._slot = slot;
				_pipeline->GetSceneUniforms()[current_id]._index = index;
				_pipeline->GetSceneUniforms()[current_id]._count = 1;
				_pipeline->GetSceneUniforms()[current_id].data_type = ShaderData::CUSTOM_DATA_TEXTURE;
				_pipeline->GetSceneUniforms()[current_id]._resource_type = ShaderResourceType::SHADER_RESOURCE_TYPE_COMBINED_SAMPLER;
				
				//count++;
				pipe->bindless_2d_tex_count[slot]++;
			}
		};

		PipelineStateDesc& desc = _pipeline->GetDesc();
		if (desc.vertex_shader)
		{
			lambda(desc.vertex_shader);
		}
		if (desc.pixel_shader)
		{
			lambda(desc.pixel_shader);
		}

		//pipe->bindless_2d_tex_count = count;
	}

	bool processShaderResources(uint32_t& resource_bindings_count, GPipeline* _pipeline, HashTable<uint32_t>& _hashTable, VKHandle* instance, VKDeviceHandle* device, uint32_t& out_size)
	{
		bool result = false;

		VKPipeline* _handle = (VKPipeline*)_pipeline->GetRenderHandle()->m_internalData;

		uint32_t total_size_global = 0;
		uint32_t total_size_instance = 0;
		uint32_t total_size_local = 0;

		uint32_t offset_alignment = device->m_properties.limits.minUniformBufferOffsetAlignment;


		uint32_t z = 0;
		for (auto& i : _pipeline->GetDesc().resources.resources)
		{
			bool is_structure = i.def == trace::ShaderDataDef::STRUCTURE;
			bool is_image = i.def == trace::ShaderDataDef::IMAGE;
			bool is_storage_buffer = i.resource_type == trace::ShaderResourceType::SHADER_RESOURCE_TYPE_STORAGE_BUFFER;

			
			trace::ShaderResourceStage res_stage = i.resource_stage;


			if (is_structure)
			{
				
				uint32_t struct_size = 0;
				uint32_t max_alignment = 0;
				uint32_t temp_globals_size = total_size_global;
				for (auto& mem : i.members)
				{
					uint32_t alignment = get_type_alignment_std140(mem.resource_data_type);
					struct_size = get_alignment(struct_size, alignment);
					max_alignment = alignment > max_alignment ? alignment : max_alignment;

					uint32_t& hash_id = _hashTable.Get_Ref(mem.resource_name);
					uint32_t current_id = _pipeline->GetSceneUniforms().size();
					_pipeline->GetSceneUniforms().push_back(trace::UniformMetaData());
					hash_id = current_id;
					_pipeline->GetSceneUniforms()[current_id]._id = current_id;
					_pipeline->GetSceneUniforms()[current_id]._size = mem.resource_size;
					_pipeline->GetSceneUniforms()[current_id]._slot = i.slot;
					_pipeline->GetSceneUniforms()[current_id].meta_id = ( (int)res_stage << 16 ) | i.slot;
					_pipeline->GetSceneUniforms()[current_id]._index = i.index;
					_pipeline->GetSceneUniforms()[current_id]._count = i.count;
					_pipeline->GetSceneUniforms()[current_id]._resource_type = i.resource_type;
					_pipeline->GetSceneUniforms()[current_id]._shader_stage = i.shader_stage;
					_pipeline->GetSceneUniforms()[current_id].data_type = mem.resource_data_type;
					_pipeline->GetSceneUniforms()[current_id]._offset = mem.offset;
					

					//struct_size = get_alignment(struct_size, offset_alignment);
					if (i.resource_stage == ShaderResourceStage::RESOURCE_STAGE_LOCAL)
					{
						_pipeline->GetSceneUniforms()[current_id]._offset = total_size_local;
						total_size_local += mem.resource_size;
						total_size_local = get_alignment(total_size_local, offset_alignment);
						break;
					}
					_pipeline->GetSceneUniforms()[current_id]._index = z;


					if (i.resource_stage == ShaderResourceStage::RESOURCE_STAGE_INSTANCE)
					{
						_pipeline->GetSceneUniforms()[current_id]._offset = struct_size;
						total_size_instance += mem.resource_size;
						total_size_instance = get_alignment(total_size_instance, offset_alignment);
					}
					struct_size += mem.resource_size;

				}
				if (i.resource_stage == trace::ShaderResourceStage::RESOURCE_STAGE_INSTANCE)
				{
					uint32_t& hash_id = _hashTable.Get_Ref(i.resource_name);
					uint32_t current_id = _pipeline->GetSceneUniforms().size();
					_pipeline->GetSceneUniforms().push_back(trace::UniformMetaData());
					hash_id = current_id;
					_pipeline->GetSceneUniforms()[current_id]._id = current_id;
					_pipeline->GetSceneUniforms()[current_id]._size = get_alignment(struct_size, max_alignment);
					_pipeline->GetSceneUniforms()[current_id]._slot = i.slot;
					_pipeline->GetSceneUniforms()[current_id].meta_id = ((int)res_stage << 16) | i.slot;
					_pipeline->GetSceneUniforms()[current_id]._index = i.index;
					_pipeline->GetSceneUniforms()[current_id]._count = i.count;
					_pipeline->GetSceneUniforms()[current_id]._resource_type = i.resource_type;
					_pipeline->GetSceneUniforms()[current_id]._shader_stage = i.shader_stage;

					_pipeline->GetSceneStructs().push_back({_hashTable.Get_Ref(i.resource_name) ,  INVALID_ID});
					z++;
				}
			}

			if (is_image)
			{
				uint32_t& hash_id = _hashTable.Get_Ref(i.resource_name);
				uint32_t current_id = _pipeline->GetSceneUniforms().size();
				_pipeline->GetSceneUniforms().push_back(trace::UniformMetaData());
				hash_id = current_id;
				_pipeline->GetSceneUniforms()[current_id]._id = current_id;
				_pipeline->GetSceneUniforms()[current_id]._size = i.resource_size;
				_pipeline->GetSceneUniforms()[current_id]._slot = i.slot;
				_pipeline->GetSceneUniforms()[current_id].meta_id = ((int)res_stage << 16) | i.slot;
				_pipeline->GetSceneUniforms()[current_id]._index = 0;
				_pipeline->GetSceneUniforms()[current_id]._count = i.count;
				_pipeline->GetSceneUniforms()[current_id]._resource_type = i.resource_type;
				_pipeline->GetSceneUniforms()[current_id]._shader_stage = i.shader_stage;
				
			}

			
		}
		
		update_data_indices(_pipeline, _hashTable, _handle);

		
		generate_pipeline_resources(device, _pipeline, _handle);

		VkDescriptorBufferInfo* bufs = new VkDescriptorBufferInfo[2048];// TODO: Use custom allocator and find a better way to update buffers
		uint32_t t_size = 0;
		
		for (uint32_t i = 0; i < device->frames_in_flight; i++)
		{
			std::vector<VkWriteDescriptorSet> _writes;
			uint32_t block_size = 0;
			uint32_t block_offset = INVALID_ID;

			uint32_t k = 0;
			uint32_t k_off = 0;
			for (auto& _i : _pipeline->GetDesc().resources.resources)
			{
				bool is_structure = _i.def == trace::ShaderDataDef::STRUCTURE;
				bool is_image = _i.def == trace::ShaderDataDef::IMAGE;

				trace::ShaderResourceStage res_stage = _i.resource_stage;
				uint32_t meta_id = ((int)res_stage << 16) | _i.slot;

				if (is_structure)
				{
					uint32_t struct_size = 0;
					for (auto& mem : _i.members)
					{
						struct_size += mem.resource_size;
						struct_size = get_alignment(struct_size, offset_alignment);
					}
					VkWriteDescriptorSet write = {};
					write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
					write.descriptorCount = 1;
					write.dstArrayElement = _i.index;
					write.dstBinding = _i.slot;
					write.pBufferInfo = nullptr;
					write.pImageInfo = nullptr;
					write.pNext = nullptr;
					write.pTexelBufferView = nullptr;
					bufs[k] = {};
					if (_i.resource_stage == ShaderResourceStage::RESOURCE_STAGE_GLOBAL)
					{

						write.dstSet = _handle->Scene_sets[i];
						write.descriptorType = vk::convertDescriptorType(_i.resource_type);
						bufs[k].offset = 0;
						bufs[k].buffer = _handle->buffer_resources[meta_id].resource[i].m_handle;
						k_off += struct_size;
						k_off = get_alignment(k_off, offset_alignment);
					}
					else if (_i.resource_stage == ShaderResourceStage::RESOURCE_STAGE_INSTANCE)
					{
						bufs[k].offset = 0;
						write.dstSet = _handle->Instance_sets[i];
						write.descriptorType = vk::convertDescriptorType(_i.resource_type);
						bufs[k].buffer = _handle->buffer_resources[meta_id].resource[i].m_handle;
					}
					else if (_i.resource_stage == ShaderResourceStage::RESOURCE_STAGE_LOCAL)
					{
						continue;
					}
					bufs[k].range = VK_WHOLE_SIZE;
					write.pBufferInfo = &bufs[k];
					_writes.push_back(write);

				}
				if (is_image)
				{
					VkWriteDescriptorSet write = {};
					write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
					write.descriptorCount = 1;
					write.dstBinding = _i.slot;
					write.pBufferInfo = nullptr;
					write.pImageInfo = nullptr;
					write.pNext = nullptr;
					write.pTexelBufferView = nullptr;
					if (_i.resource_stage == ShaderResourceStage::RESOURCE_STAGE_GLOBAL)
					{

						write.dstSet = _handle->Scene_sets[i];
					}
					else if (_i.resource_stage == ShaderResourceStage::RESOURCE_STAGE_INSTANCE)
					{
						write.dstSet = _handle->Instance_sets[i];
					}
					for (uint32_t index = 0; index < _i.count; index++)
					{
						write.dstArrayElement = index;
						VkDescriptorImageInfo img_info = {};
						img_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
						img_info.imageView = device->nullImage.m_view;
						img_info.sampler = device->nullImage.m_sampler;
						write.descriptorType = vk::convertDescriptorType(_i.resource_type);
						write.pImageInfo = &img_info;
						_writes.push_back(write);
						
					}
				}
				k++;


			}
			t_size = k_off;

			vkUpdateDescriptorSets(
				device->m_device,
				static_cast<uint32_t>(_writes.size()),
				_writes.data(),
				0,
				nullptr
			);
		}
		delete[] bufs;// TODO: Use custom allocator and find a better way to update buffers
		out_size = total_size_global;
		result = true;
		return result;
	}


}

bool generate_pipeline_resources(trace::VKDeviceHandle* device, trace::GPipeline* pipeline, trace::VKPipeline* handle)
{
	uint32_t offset_alignment = device->m_properties.limits.minUniformBufferOffsetAlignment;

	for (trace::ShaderResource& resource : pipeline->GetDesc().resources.resources)
	{
		if (resource.resource_stage == trace::ShaderResourceStage::RESOURCE_STAGE_LOCAL)
		{
			continue;
		}

		bool is_structure = resource.def == trace::ShaderDataDef::STRUCTURE;
		bool is_image = resource.def == trace::ShaderDataDef::IMAGE;

		uint32_t id = 0;
		uint32_t stage = (uint32_t)resource.resource_stage;
		id = (stage << 16) | (resource.slot);


		if (is_structure)
		{
			uint32_t total_size = 0;
			uint32_t max_alignment = 0;
			for (trace::ShaderResource::Member& member : resource.members)
			{
				if (resource.resource_stage == trace::ShaderResourceStage::RESOURCE_STAGE_INSTANCE)
				{
					uint32_t alignment = get_type_alignment_std140(member.resource_data_type);
					max_alignment = alignment > max_alignment ? alignment : max_alignment;
					total_size = get_alignment(total_size, alignment);
				}
				total_size += member.resource_size;
				if (resource.resource_stage == trace::ShaderResourceStage::RESOURCE_STAGE_GLOBAL)
				{
					total_size = get_alignment(total_size, offset_alignment);
				}
			}
			if (resource.resource_stage == trace::ShaderResourceStage::RESOURCE_STAGE_INSTANCE)
			{
				total_size = get_alignment(total_size, max_alignment);
			}

			trace::BufferBindingInfo& info = handle->buffer_resources[id];
			info.current_frame_offset = 0;
			
			uint32_t buffer_size = total_size;
			VkBufferUsageFlags usage_flag;
			uint32_t bind_flag = 0;

			bool is_uniform_buffer = resource.resource_type == trace::ShaderResourceType::SHADER_RESOURCE_TYPE_UNIFORM_BUFFER;
			bool is_storage_buffer = resource.resource_type == trace::ShaderResourceType::SHADER_RESOURCE_TYPE_STORAGE_BUFFER;


			if (is_uniform_buffer)
			{
				usage_flag = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
				bind_flag |= trace::BindFlag::CONSTANT_BUFFER_BIT;
			}
			if (is_storage_buffer)
			{
				usage_flag = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
				bind_flag |= trace::BindFlag::UNORDERED_RESOURCE_BIT;

				buffer_size = total_size + KB;
			}

			if (resource.resource_stage == trace::ShaderResourceStage::RESOURCE_STAGE_INSTANCE && !is_storage_buffer)
			{
				buffer_size = total_size + (2 * KB);
			}

			for (uint32_t i = 0; i < device->frames_in_flight; i++)
			{

				trace::BufferInfo create_info;
				create_info.m_flag = (trace::BindFlag)bind_flag;
				create_info.m_size = buffer_size;
				create_info.m_usageFlag = trace::UsageFlag::UPLOAD;

				vk::_CreateBuffer(device->instance, device, &info.resource[i], create_info);
			}
		}
	}

	return true;
}

uint32_t get_type_alignment_std430(trace::ShaderData type)
{
	switch (type)
	{
	case trace::ShaderData::CUSTOM_DATA_BOOL:
	{
		return 4;
	}
	case trace::ShaderData::CUSTOM_DATA_FLOAT:
	{
		return 4;
	}
	case trace::ShaderData::CUSTOM_DATA_INT:
	{
		return 4;
	}
	case trace::ShaderData::CUSTOM_DATA_IVEC2:
	{
		return 8;
	}
	case trace::ShaderData::CUSTOM_DATA_IVEC3:
	{
		return 4;
	}
	case trace::ShaderData::CUSTOM_DATA_IVEC4:
	{
		return 16;
	}
	case trace::ShaderData::CUSTOM_DATA_MAT2:
	{
		return 8;
	}
	case trace::ShaderData::CUSTOM_DATA_MAT3:
	{
		return 16;
	}
	case trace::ShaderData::CUSTOM_DATA_MAT4:
	{
		return 16;
	}
	case trace::ShaderData::CUSTOM_DATA_VEC2:
	{
		return 8;
	}
	case trace::ShaderData::CUSTOM_DATA_VEC3:
	{
		return 4;
	}
	case trace::ShaderData::CUSTOM_DATA_VEC4:
	{
		return 16;
	}
	}

	return 0;
}

uint32_t get_type_alignment_std140(trace::ShaderData type)
{
	switch (type)
	{
	case trace::ShaderData::CUSTOM_DATA_BOOL:
	{
		return 4;
	}
	case trace::ShaderData::CUSTOM_DATA_FLOAT:
	{
		return 4;
	}
	case trace::ShaderData::CUSTOM_DATA_INT:
	{
		return 4;
	}
	case trace::ShaderData::CUSTOM_DATA_IVEC2:
	{
		return 8;
	}
	case trace::ShaderData::CUSTOM_DATA_IVEC3:
	{
		return 16;
	}
	case trace::ShaderData::CUSTOM_DATA_IVEC4:
	{
		return 16;
	}
	case trace::ShaderData::CUSTOM_DATA_MAT2:
	{
		return 16;
	}
	case trace::ShaderData::CUSTOM_DATA_MAT3:
	{
		return 16;
	}
	case trace::ShaderData::CUSTOM_DATA_MAT4:
	{
		return 16;
	}
	case trace::ShaderData::CUSTOM_DATA_VEC2:
	{
		return 8;
	}
	case trace::ShaderData::CUSTOM_DATA_VEC3:
	{
		return 16;
	}
	case trace::ShaderData::CUSTOM_DATA_VEC4:
	{
		return 16;
	}
	}

	return 0;
}

namespace vk {

	void update_pipeline_instance_sets(trace::GPipeline* pipeline, trace::VKPipeline* _handle, trace::VKBuffer& buffer,uint32_t new_size)
	{
		trace::VKDeviceHandle* _device = (trace::VKDeviceHandle*)_handle->m_device;
		trace::VKHandle* _instance = (trace::VKHandle*)_handle->m_instance;


		_ResizeBuffer(
			_instance,
			_device,
			buffer,
			new_size
			);

	}

	void resize_buffer_and_update_set(trace::GPipeline* pipeline, trace::VKPipeline* _handle, trace::UniformMetaData& meta_data, trace::VKBuffer& buffer, uint32_t new_size, VkDescriptorSet& set)
	{
		trace::VKDeviceHandle* _device = (trace::VKDeviceHandle*)_handle->m_device;
		trace::VKHandle* _instance = (trace::VKHandle*)_handle->m_instance;


		_ResizeBuffer( _instance, _device, buffer, new_size);

		VkWriteDescriptorSet write = {};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.descriptorCount = 1;
		write.dstArrayElement = meta_data._index;
		write.dstBinding = meta_data._slot;
		write.pBufferInfo = nullptr;
		write.pImageInfo = nullptr;
		write.pNext = nullptr;
		write.pTexelBufferView = nullptr;
		write.dstSet = set;

		VkDescriptorBufferInfo buffer_info = {};
		buffer_info.buffer = buffer.m_handle;
		buffer_info.offset = 0;
		buffer_info.range = VK_WHOLE_SIZE;

		write.pBufferInfo = &buffer_info;
		write.descriptorType = vk::convertDescriptorType(meta_data._resource_type);

		vkUpdateDescriptorSets(_device->m_device, 1, &write, 0, nullptr);

	}

	bool __CreatePipeline(trace::GPipeline* pipeline, trace::PipelineStateDesc desc)
	{
		bool result = true;

		

		if (!pipeline)
		{
			TRC_ERROR("Please input valid pointer -> {}, Function -> {}", (const void*)pipeline, __FUNCTION__);
			return false;
		}

		if (pipeline->GetRenderHandle()->m_internalData)
		{
			TRC_WARN("This pipeline handle is valid, {}, Function -> {}", (const void*)pipeline->GetRenderHandle()->m_internalData, __FUNCTION__);
			return false;
		}

		trace::VKPipeline* _handle = new trace::VKPipeline(); //TODO: Use a custom allocator
		_handle->m_device = &g_VkDevice;
		_handle->m_instance = &g_Vkhandle;
		trace::VKHandle* _instance = (trace::VKHandle*)_handle->m_instance;
		trace::VKDeviceHandle* _device = (trace::VKDeviceHandle*)_handle->m_device;
		pipeline->GetRenderHandle()->m_internalData = _handle;

		pipeline->SetDesc(desc);


		pipeline->GetHashTable().Init(1063);// TODO: let number be configurable or more dynamic

		uint32_t _ids = INVALID_ID;
		pipeline->GetHashTable().Fill(_ids);

		VkViewport viewport = {};
		viewport.x = 0;
		viewport.y = -desc.view_port.height;
		viewport.width = desc.view_port.width;
		viewport.height = desc.view_port.height;
		viewport.minDepth = desc.view_port.minDepth;
		viewport.maxDepth = desc.view_port.maxDepth;

		VkRect2D scissor = {};
		scissor.offset.x = scissor.offset.y = 0;
		scissor.extent.width = static_cast<uint32_t>(desc.view_port.width);
		scissor.extent.height = static_cast<uint32_t>(desc.view_port.height);

		trace::VKRenderPass* pass = reinterpret_cast<trace::VKRenderPass*>(desc.render_pass->GetRenderHandle()->m_internalData);

		VkResult pipeline_result = vk::_CreatePipeline(
			_instance,
			_device,
			1,
			&viewport,
			1,
			&scissor,
			desc,
			_handle,
			pass,
			desc.subpass_index
		);

		if (pipeline_result == VK_SUCCESS)
		{
			TRC_INFO("Pipeline created  |__// ...");
		}
		else
		{
			delete _handle;
			pipeline->GetRenderHandle()->m_internalData = nullptr;
			result = false;
		}

		

		return result;
	}
	bool __DestroyPipeline(trace::GPipeline* pipeline)
	{
		bool result = true;

		

		if (!pipeline)
		{
			TRC_ERROR("Please input valid pointer -> {}, Function -> {}", (const void*)pipeline, __FUNCTION__);
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




		for (auto& i : _handle->buffer_resources)
		{
			for (uint32_t j = 0; j < _device->frames_in_flight; j++)
			{
				_DestoryBuffer(_instance, _device, &i.second.resource[j]);
			}
		}

		

		vkDeviceWaitIdle(_device->m_device);
		vk::_DestroyPipeline(_instance, _device, _handle);

		

		delete pipeline->GetRenderHandle()->m_internalData;
		pipeline->GetRenderHandle()->m_internalData = nullptr;

		return result;
	}
	bool __InitializePipeline(trace::GPipeline* pipeline)
	{
		bool result = true;

		

		if (!pipeline)
		{
			TRC_ERROR("Please input valid pointer -> {}, Function -> {}", (const void*)pipeline, __FUNCTION__);
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


		uint32_t map_data_size;


		result = trace::processShaderResources(
			map_data_size,
			pipeline,
			pipeline->GetHashTable(),
			_instance,
			_device,
			map_data_size
		);

		return result;
	}
	bool __ShutDownPipeline(trace::GPipeline* pipeline)
	{
		bool result = true;

		

		if (!pipeline)
		{
			TRC_ERROR("Please input valid pointer -> {}, Function -> {}", (const void*)pipeline, __FUNCTION__);
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

		vkDeviceWaitIdle(_device->m_device);
		vk::_DestroyPipeline(_instance, _device, _handle);

		// TODO: Fix bug ___> unknow error source
		//FreeAligned(_mapped_data);

		delete pipeline->GetRenderHandle()->m_internalData;
		pipeline->GetRenderHandle()->m_internalData = nullptr;

		return result;
	}
	bool __SetPipelineData(trace::GPipeline* pipeline, const std::string& resource_name, trace::ShaderResourceStage resource_scope, void* data, uint32_t size)
	{
		bool result = true;		

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

		uint32_t hash_id = pipeline->GetHashTable().Get(resource_name);

		if (hash_id == INVALID_ID)
		{
			TRC_CRITICAL("Can't set value for an invalid resource. please check if pipeline has been initialized {}", resource_name);
			return false;
		}


		trace::UniformMetaData& meta_data = pipeline->GetSceneUniforms()[hash_id];

		return __SetPipelineData_Meta(pipeline, meta_data, resource_scope, data, size);
	}
	bool __SetPipelineInstanceData(trace::GPipeline* pipeline, const std::string& resource_name, trace::ShaderResourceStage resource_scope, void* data, uint32_t size, uint32_t count)
	{
		bool result = true;

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

		uint32_t hash_id = pipeline->GetHashTable().Get(resource_name);

		if (hash_id == INVALID_ID)
		{
			TRC_CRITICAL("Can't set value for an invalid resource. please check if pipeline has been initialized {}", resource_name);
			return false;
		}

		trace::UniformMetaData& meta_data = pipeline->GetSceneUniforms()[hash_id];

		//if (resource_scope == trace::ShaderResourceStage::RESOURCE_STAGE_INSTANCE)
		//{
		//	//NOTE: Removing buffer info added at OnDrawStart
		//	uint32_t r_index = (_handle->instance_buffer_infos.size() - meta_data._struct_index)  - 1;
		//	auto it = _handle->instance_buffer_infos.begin() + r_index;
		//	_handle->instance_buffer_infos.erase(it);

		//	trace::VKBuffer& instance_buffer = _handle->Instance_buffers[_device->m_imageIndex];

		//	uint32_t buf_offset = _handle->instance_buffer_offset;

		//	if ((buf_offset + size) >= instance_buffer.m_info.m_size)
		//	{
		//		update_pipeline_instance_sets(pipeline, _handle, buf_offset + size);

		//	}

		//	void* data_point;

		//	vkMapMemory(
		//		_device->m_device,
		//		instance_buffer.m_memory,
		//		0,
		//		VK_WHOLE_SIZE,
		//		VK_NO_FLAGS,
		//		&data_point
		//	);

		//	uint32_t location = buf_offset;

		//	void* map_point = (char*)data_point + location;
		//	memcpy(map_point, data, size);

		//	vkUnmapMemory(_device->m_device, instance_buffer.m_memory);
		//	_handle->instance_buffer_offset += size;

		//	uint32_t current_offset = buf_offset;
		//	for (uint32_t i = 0; i < count; i++)
		//	{
		//		VkDescriptorBufferInfo buf_info = {};
		//		buf_info.buffer = _handle->Instance_buffers[_device->m_imageIndex].m_handle;
		//		buf_info.offset = current_offset;
		//		buf_info.range = meta_data._size;

		//		_handle->instance_buffer_infos.insert(it ,buf_info);
		//		it++;

		//		current_offset += meta_data._size;
		//	}

		//}



		return result;

	}
	bool __SetPipelineData_Meta(trace::GPipeline* pipeline, trace::UniformMetaData& meta_data, trace::ShaderResourceStage resource_scope, void* data, uint32_t size)
	{
		bool result = true;



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

		bool is_storage_buffer = meta_data._resource_type == trace::ShaderResourceType::SHADER_RESOURCE_TYPE_STORAGE_BUFFER;

		
		if (size > meta_data._size)
		{
			TRC_ERROR("Please ensure data size is not greater than resource data");
		}
		if (resource_scope == trace::ShaderResourceStage::RESOURCE_STAGE_LOCAL)
		{
			vkCmdPushConstants(
				_device->m_graphicsCommandBuffers[_device->m_imageIndex].m_handle,
				_handle->m_layout,
				vk::convertShaderStage(meta_data._shader_stage),
				meta_data._offset,
				meta_data._size,
				data
			);
			return true;
		}

		if (resource_scope == trace::ShaderResourceStage::RESOURCE_STAGE_GLOBAL)
		{
			

			void* data_point;
			trace::VKBuffer& buffer = _handle->buffer_resources[meta_data.meta_id].resource[_device->m_imageIndex];
			VkDescriptorSet& set = _handle->Scene_sets[_device->m_imageIndex];

			if (size > buffer.m_info.m_size)
			{
				uint32_t new_size = buffer.m_info.m_size * 2;
				resize_buffer_and_update_set(pipeline, _handle, meta_data, buffer, new_size, set);
			}

			vkMapMemory(_device->m_device, buffer.m_memory, 0, buffer.m_info.m_size, VK_NO_FLAGS, &data_point);

			uint32_t location = meta_data._offset;

			void* map_point = (char*)data_point + location;
			memcpy(map_point, data, size);
			vkUnmapMemory(_device->m_device, buffer.m_memory);

			return true;
		}

		if (resource_scope == trace::ShaderResourceStage::RESOURCE_STAGE_INSTANCE)
		{


			trace::VKBuffer& buffer = _handle->buffer_resources[meta_data.meta_id].resource[_device->m_imageIndex];

			uint32_t buffer_index = (_handle->instance_buffer_infos[meta_data._slot].size() - 1);
			trace::BufferDescriptorInfo& buf_info = _handle->instance_buffer_infos[meta_data._slot][buffer_index];

			uint32_t buf_offset = buf_info.offset + meta_data._offset;

			if (buf_offset + meta_data._size >= buffer.m_info.m_size)
			{
				if (is_storage_buffer)
				{					
					VkDescriptorSet& set = _handle->Instance_sets[_device->m_imageIndex];
					uint32_t new_size = buffer.m_info.m_size * 2;
					resize_buffer_and_update_set(pipeline, _handle, meta_data, buffer, new_size, set);
				}
				else
				{
					update_pipeline_instance_sets(pipeline, _handle, buffer, buffer.m_info.m_size * 4);
					
				}


			}

			buf_info.is_bindless = is_storage_buffer ? false : true;

			void* data_point;

			vkMapMemory( _device->m_device, buffer.m_memory, 0, VK_WHOLE_SIZE, VK_NO_FLAGS, &data_point);

			uint32_t location = buf_offset;

			void* map_point = (char*)data_point + location;
			memcpy(map_point, data, size);

			vkUnmapMemory(_device->m_device, buffer.m_memory);
		}
		


		return result;
	}
	bool __SetPipelineTextureData(trace::GPipeline* pipeline, const std::string& resource_name, trace::ShaderResourceStage resource_scope, trace::GTexture* texture, uint32_t index)
	{
		bool result = true;

		if (!pipeline || !texture)
		{
			TRC_ERROR("Please input valid pointer -> {}, tex ->{}, Function -> {}", (const void*)pipeline, __FUNCTION__);
			return false;
		}

		if (!pipeline->GetRenderHandle()->m_internalData)
		{
			TRC_ERROR("Invalid pipeline handle, {}, Function -> {}", (const void*)pipeline->GetRenderHandle()->m_internalData, __FUNCTION__);
			return false;
		}

		if (!texture->GetRenderHandle()->m_internalData)
		{
			TRC_ERROR("Invalid texture handle, {}, Function -> {}", texture->GetName(), __FUNCTION__);
		}

		trace::VKPipeline* _handle = (trace::VKPipeline*)pipeline->GetRenderHandle()->m_internalData;
		trace::VKHandle* _instance = (trace::VKHandle*)_handle->m_instance;
		trace::VKDeviceHandle* _device = (trace::VKDeviceHandle*)_handle->m_device;
		trace::VKImage* _tex = texture->GetRenderHandle()->m_internalData ? reinterpret_cast<trace::VKImage*>(texture->GetRenderHandle()->m_internalData) : &_device->nullImage;

		uint32_t hash_id = pipeline->GetHashTable().Get(resource_name.c_str());

		if (hash_id == INVALID_ID)
		{
			TRC_CRITICAL("Can't set value for an invalid resource. please check if pipeline has been initialized");
			return false;
		}



		trace::UniformMetaData& meta_data = pipeline->GetSceneUniforms()[hash_id];

		return __SetPipelineTextureData_Meta(pipeline, meta_data, resource_scope, _tex, index);
	}
	bool __SetPipelineTextureData_Meta(trace::GPipeline* pipeline, trace::UniformMetaData& meta_data, trace::ShaderResourceStage resource_scope, trace::VKImage* texture, uint32_t index)
	{
		bool result = true;

		if (!pipeline)
		{
			TRC_ERROR("Please input valid pointer -> {}, tex ->{}, Function -> {}", (const void*)pipeline, __FUNCTION__);
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
		trace::VKImage* _tex = texture;

		VkWriteDescriptorSet write = {};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		switch (meta_data._resource_type)
		{
		case trace::ShaderResourceType::SHADER_RESOURCE_TYPE_COMBINED_SAMPLER:
		{
			write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			break;
		}
		}

		VkDescriptorImageInfo image_info = {};
		image_info.sampler = _tex->m_sampler;
		image_info.imageView = _tex->m_view;
		image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		write.descriptorCount = 1; //HACK: Fix
		write.dstBinding = meta_data._slot;
		write.pImageInfo = &image_info;
		write.dstArrayElement = index;

		bool _updated = true;
		if (meta_data._frame_index != _device->m_imageIndex)
		{
			meta_data._frame_index = _device->m_imageIndex;
			meta_data._num_frame_update = 0;
			_updated = false;
		}

		uint32_t set_index = (_device->m_imageIndex * VK_MAX_DESCRIPTOR_SET_PER_FRAME) + meta_data._num_frame_update;
		++meta_data._num_frame_update;

		if (resource_scope == trace::ShaderResourceStage::RESOURCE_STAGE_GLOBAL)
		{
			write.dstSet = _handle->Scene_set;

			vkUpdateDescriptorSets(
				_device->m_device,
				1,
				&write,
				0,
				nullptr
			);
		}

		if (resource_scope == trace::ShaderResourceStage::RESOURCE_STAGE_INSTANCE)
		{
			std::vector<trace::TextureDescriptorInfo>& tex_infos = _handle->instance_texture_infos[meta_data._slot];
			int index = ((int)tex_infos.size() - _handle->bindless_2d_tex_count[meta_data._slot]) + (int)meta_data._index;
			tex_infos[index].texture = _tex;
		}


		return result;
	}
	bool __BindPipeline_(trace::GPipeline* pipeline)
	{
		bool result = true;

		

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


		uint32_t set_count = 0;
		VkDescriptorSet _sets[3];
		uint32_t set_index = (_device->m_imageIndex * VK_MAX_DESCRIPTOR_SET_PER_FRAME);
		if (_handle->Scene_sets[0])
		{
			_sets[set_count++] = _handle->Scene_set ? _handle->Scene_set : _handle->Scene_sets[set_index];
		}

		if (_handle->Instance_sets[0])
		{
			_sets[set_count++] = _handle->Instance_set ? _handle->Instance_set : _handle->Instance_sets[set_index];
		}



		uint32_t offset_count = 0;
		uint32_t offsets[12] = {};

		for (auto& stct : pipeline->GetSceneStructs())
		{
			offsets[offset_count++] = stct.second;
		}


		offset_count = 0;

		glm::ivec4 draw_inst(_handle->frame_update - 1, _handle->frame_update - 1/*_handle->instance_texture_infos.size() - _handle->bindless_2d_tex_count*/, 0, 0);
		if (_handle->bindless)
		{
			__SetPipelineData(pipeline, "draw_instance_index", trace::ShaderResourceStage::RESOURCE_STAGE_LOCAL, &draw_inst, sizeof(glm::ivec4));
		}

		vkCmdBindDescriptorSets( _device->m_graphicsCommandBuffers[_device->m_imageIndex].m_handle, VK_PIPELINE_BIND_POINT_GRAPHICS, _handle->m_layout, 0, set_count, _sets, offset_count, offsets);

		return result;
		
	}

}