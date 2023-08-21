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
		GPipeline*_pipeline,
		HashTable<uint32_t>& _hashTable,
		VKHandle* instance,
		VKDeviceHandle* device,
		uint32_t& out_size
		);



	bool processShaderResources(uint32_t& resource_bindings_count, ShaderResourceBinding* resource_bindings, GPipeline* _pipeline, HashTable<uint32_t>& _hashTable, VKHandle* instance, VKDeviceHandle* device, uint32_t& out_size)
	{
		bool result = false;

		VKPipeline* _handle = (VKPipeline*)_pipeline->GetRenderHandle()->m_internalData;

		uint32_t total_size_global = 0;
		uint32_t total_size_local = 0;
		_pipeline->Scene_uniforms.resize(128);// TODO: Fix Hard Coded value

		uint32_t z = 0;
		for (auto& i : _pipeline->m_desc.resources.resources)
		{
			bool is_struct = i.def == trace::ShaderDataDef::STRUCTURE;
			bool is_array = i.def == trace::ShaderDataDef::ARRAY;
			bool is_varible = i.def == trace::ShaderDataDef::VARIABLE;
			bool is_sArray = i.def == trace::ShaderDataDef::STRUCT_ARRAY;

			trace::ShaderResourceStage res_stage = trace::ShaderResourceStage::RESOURCE_STAGE_NONE;
			if (is_struct) res_stage = i._struct.resource_stage;
			else if (is_array) res_stage = i._array.resource_stage;
			else if (is_varible) res_stage = i._variable.resource_stage;
			else if (is_sArray) res_stage = i._array.resource_stage;

			if (is_struct)
			{
				uint32_t struct_size = 0;
				for (auto& mem : i._struct.members)
				{
					uint32_t& hash_id = _hashTable.Get_Ref(mem.resource_name);
					for (uint32_t j = 0; j < _pipeline->Scene_uniforms.size(); j++)
					{
						if (_pipeline->Scene_uniforms[j]._id == INVALID_ID)
						{

							hash_id = j;
							_pipeline->Scene_uniforms[j]._id = j;
							_pipeline->Scene_uniforms[j]._size = mem.resource_size;
							_pipeline->Scene_uniforms[j]._slot = i._struct.slot;
							_pipeline->Scene_uniforms[j]._index = i._struct.index;
							_pipeline->Scene_uniforms[j]._count = i._struct.count;
							_pipeline->Scene_uniforms[j]._resource_type = i._struct.resource_type;
							_pipeline->Scene_uniforms[j]._shader_stage = i._struct.shader_stage;
							_pipeline->Scene_uniforms[j]._offset = struct_size;
							struct_size += mem.resource_size;
							struct_size = get_alignment(struct_size, 16);
							if (i._struct.resource_stage == ShaderResourceStage::RESOURCE_STAGE_LOCAL)
							{
								_pipeline->Scene_uniforms[j]._offset = total_size_local;
								total_size_local += mem.resource_size;
								total_size_local = get_alignment(total_size_local, 16);
								break;
							}
							_pipeline->Scene_uniforms[j]._struct_index = z;
							if (i._struct.resource_stage == ShaderResourceStage::RESOURCE_STAGE_GLOBAL)
							{
								_pipeline->Scene_uniforms[j]._offset = total_size_global;
								total_size_global += mem.resource_size;
								total_size_global = get_alignment(total_size_global, 16);
							}

							break;
						}
					}
				}
				if (i._struct.resource_stage == trace::ShaderResourceStage::RESOURCE_STAGE_INSTANCE)
				{
					_pipeline->Scence_struct.push_back({ struct_size ,  INVALID_ID });
					z++;
				}
			}

			if (is_array)
			{
				for (auto& mem : i._array.members)
				{
					uint32_t& hash_id = _hashTable.Get_Ref(mem.resource_name);
					for (uint32_t j = 0; j < _pipeline->Scene_uniforms.size(); j++)
					{
						if (_pipeline->Scene_uniforms[j]._id == INVALID_ID)
						{

							hash_id = j;
							_pipeline->Scene_uniforms[j]._id = j;
							_pipeline->Scene_uniforms[j]._size = i._array.resource_size;
							_pipeline->Scene_uniforms[j]._slot = i._array.slot;
							_pipeline->Scene_uniforms[j]._index = mem.index;
							_pipeline->Scene_uniforms[j]._count = i._array.count;
							_pipeline->Scene_uniforms[j]._resource_type = i._array.resource_type;
							_pipeline->Scene_uniforms[j]._shader_stage = i._array.shader_stage;
							if (i._struct.resource_stage == ShaderResourceStage::RESOURCE_STAGE_LOCAL)
							{
								_pipeline->Scene_uniforms[j]._offset = total_size_local;
								total_size_local += i._array.resource_size;
								total_size_local = get_alignment(total_size_local, 16);
								break;
							}
							//_pipeline->Scene_uniforms[j]._offset = total_size_global;
							//total_size_global += i._array.resource_size;
							//total_size_global = get_alignment(total_size_global, 16);
							break;
						}
					}
				}
			}

			if (is_sArray)
			{
				uint32_t& hash_id = _hashTable.Get_Ref(i._array.name);
				for (uint32_t j = 0; j < _pipeline->Scene_uniforms.size(); j++)
				{
					if (_pipeline->Scene_uniforms[j]._id == INVALID_ID)
					{

						hash_id = j;
						_pipeline->Scene_uniforms[j]._id = j;
						_pipeline->Scene_uniforms[j]._size = i._array.resource_size * i._array.count;
						_pipeline->Scene_uniforms[j]._slot = i._array.slot;
						_pipeline->Scene_uniforms[j]._index = 0;
						_pipeline->Scene_uniforms[j]._count = i._array.count;
						_pipeline->Scene_uniforms[j]._resource_type = i._array.resource_type;
						_pipeline->Scene_uniforms[j]._shader_stage = i._array.shader_stage;
						_pipeline->Scene_uniforms[j]._offset = 0;
						_pipeline->Scene_uniforms[j]._struct_index = z;
						if (i._array.resource_stage == ShaderResourceStage::RESOURCE_STAGE_LOCAL)
						{
							_pipeline->Scene_uniforms[j]._offset = total_size_local;
							total_size_local += i._array.resource_size;
							total_size_local = get_alignment(total_size_local, 16);
							break;
						}
						if (i._array.resource_stage == ShaderResourceStage::RESOURCE_STAGE_GLOBAL)
						{
							_pipeline->Scene_uniforms[j]._offset = total_size_global;
							total_size_global += (i._array.resource_size * i._array.count);
							total_size_global = get_alignment(total_size_global, 16);
						}
						break;
					}
				}
				if (i._array.resource_stage == trace::ShaderResourceStage::RESOURCE_STAGE_LOCAL)
				{
					_pipeline->Scence_struct.push_back({ i._array.resource_size * i._array.count ,  INVALID_ID });
					z++;
				}
			}
		}
		
		if (total_size_global > 0)
		{
			vk::createBuffer(
				instance,
				device,
				total_size_global * device->frames_in_flight,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				_handle->Scene_buffers.m_handle,
				_handle->Scene_buffers.m_memory
			);
		}


		VkDescriptorBufferInfo bufs[128];
		uint32_t t_size = 0;
		for (uint32_t i = 0; i < device->frames_in_flight; i++)
		{
			std::vector<VkWriteDescriptorSet> _writes;
			uint32_t block_size = 0;
			uint32_t block_offset = INVALID_ID;

			uint32_t k = 0;
			uint32_t k_off = 0;
			for (auto& _i : _pipeline->m_desc.resources.resources)
			{
				bool is_struct = _i.def == trace::ShaderDataDef::STRUCTURE;
				bool is_array = _i.def == trace::ShaderDataDef::ARRAY;
				bool is_varible = _i.def == trace::ShaderDataDef::VARIABLE;
				bool is_sArray = _i.def == trace::ShaderDataDef::STRUCT_ARRAY;

				trace::ShaderResourceStage res_stage = trace::ShaderResourceStage::RESOURCE_STAGE_NONE;
				if (is_struct) res_stage = _i._struct.resource_stage;
				else if (is_array) res_stage = _i._array.resource_stage;
				else if (is_varible) res_stage = _i._variable.resource_stage;
				else if (is_sArray) res_stage = _i._array.resource_stage;

				if (is_struct)
				{
					uint32_t struct_size = 0;
					for (auto& mem : _i._struct.members)
					{
						struct_size += mem.resource_size;
						struct_size = get_alignment(struct_size, 16);
					}
					VkWriteDescriptorSet write = {};
					write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
					write.descriptorCount = 1;
					write.dstArrayElement = _i._struct.index;
					write.dstBinding = _i._struct.slot;
					write.pBufferInfo = nullptr;
					write.pImageInfo = nullptr;
					write.pNext = nullptr;
					write.pTexelBufferView = nullptr;
					bufs[k] = {};
					if (_i._struct.resource_stage == ShaderResourceStage::RESOURCE_STAGE_GLOBAL)
					{

						write.dstSet = _handle->Scene_sets[i];
						write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
						bufs[k].offset = k_off + ( t_size * i);
						bufs[k].buffer = _handle->Scene_buffers.m_handle;
						k_off += struct_size;
						k_off = get_alignment(k_off, 16);
					}
					else if (_i._struct.resource_stage == ShaderResourceStage::RESOURCE_STAGE_INSTANCE)
					{
						bufs[k].offset = 0;
						write.dstSet = _handle->Instance_sets[i];
						write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
						bufs[k].buffer = device->m_frameDescriptorBuffer.m_handle;
					}
					else if (_i._struct.resource_stage == ShaderResourceStage::RESOURCE_STAGE_LOCAL)
					{
						continue;
					}
					bufs[k].range = struct_size;
					write.pBufferInfo = &bufs[k];
					_writes.push_back(write);
				}
				if (is_array)
				{
					VkWriteDescriptorSet write = {};
					write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
					write.descriptorCount = 1;
					write.dstBinding = _i._array.slot;
					write.pBufferInfo = nullptr;
					write.pImageInfo = nullptr;
					write.pNext = nullptr;
					write.pTexelBufferView = nullptr;
					if (_i._array.resource_stage == ShaderResourceStage::RESOURCE_STAGE_GLOBAL)
					{

						write.dstSet = _handle->Scene_sets[i];
					}
					else if (_i._array.resource_stage == ShaderResourceStage::RESOURCE_STAGE_INSTANCE)
					{
						write.dstSet = _handle->Instance_sets[i];
					}
					for (auto& mem : _i._array.members)
					{
						write.dstArrayElement = mem.index;
						if (_i._array.resource_type == ShaderResourceType::SHADER_RESOURCE_TYPE_COMBINED_SAMPLER)
						{
							VkDescriptorImageInfo img_info = {};
							img_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
							img_info.imageView = device->nullImage.m_view;
							img_info.sampler = device->nullImage.m_sampler;
							write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
							write.pImageInfo = &img_info;
							_writes.push_back(write);
							//if (mem.resource_data_type == ShaderData::CUSTOM_DATA_TEXTURE && mem.data)
							//{
							//	VKImage* tex = (VKImage*)mem.data;
							//	VkDescriptorImageInfo img_info = {};
							//	img_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
							//	img_info.imageView = tex->m_view;
							//	img_info.sampler = tex->m_sampler;
							//	write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
							//	write.pImageInfo = &img_info;
							//	_writes.push_back(write);
							//}
						}
					}
				}
				if (is_sArray)
				{
					uint32_t struct_size = _i._array.resource_size;
					VkWriteDescriptorSet write = {};
					write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
					write.descriptorCount = 1;
					write.dstBinding = _i._array.slot;
					write.pBufferInfo = nullptr;
					write.pImageInfo = nullptr;
					write.pNext = nullptr;
					write.pTexelBufferView = nullptr;
					for (uint32_t r = 0; r < _i._array.count; r++)
					{
						write.dstArrayElement = r;
						if (_i._array.resource_stage == ShaderResourceStage::RESOURCE_STAGE_GLOBAL)
						{
							write.dstSet = _handle->Scene_sets[i];
							write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
							bufs[k].offset = k_off + (t_size * i);
							bufs[k].buffer = _handle->Scene_buffers.m_handle;
							k_off += struct_size;
							k_off = get_alignment(k_off, 16);
						}
						else if (_i._array.resource_stage == ShaderResourceStage::RESOURCE_STAGE_INSTANCE)
						{
							bufs[k].offset = 0;
							write.dstSet = _handle->Instance_sets[i];
							write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
							bufs[k].buffer = device->m_frameDescriptorBuffer.m_handle;
						}
						else if (_i._array.resource_stage == ShaderResourceStage::RESOURCE_STAGE_LOCAL)
						{
							continue;
						}
						bufs[k].range = struct_size;
						write.pBufferInfo = &bufs[k];
						_writes.push_back(write);
						k++;
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
		_handle->cache_size = t_size;
		out_size = total_size_global;
		result = true;
		return result;
	}

}

namespace vk {

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

		pipeline->m_desc = desc;


		pipeline->_hashTable.Init(512);// TODO: let number be configurable or more dynamic

		uint32_t _ids = INVALID_ID;
		pipeline->_hashTable.Fill(_ids);

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


		if (_handle->Scene_buffers.m_memory)
		{
			vkUnmapMemory(
				_device->m_device,
				_handle->Scene_buffers.m_memory
			);
		}
		vkDeviceWaitIdle(_device->m_device);
		vk::_DestroyPipeline(_instance, _device, _handle);

		// TODO: Fix bug ___> unknow error source
		//FreeAligned(_mapped_data);

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


		uint32_t total_size_global = 0;


		uint32_t map_data_size;


		result = trace::processShaderResources(
			map_data_size,
			nullptr,
			pipeline,
			pipeline->_hashTable,
			_instance,
			_device,
			map_data_size
		);

		//_mapped_data = (char*)AllocAligned(map_data_size, m_device->m_properties.limits.minMemoryMapAlignment);

		if (_handle->Scene_buffers.m_memory)
		{
			vkMapMemory(
				_device->m_device,
				_handle->Scene_buffers.m_memory,
				0,
				map_data_size,
				0,
				(void**)&_handle->cache_data
			);
		}


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

		if (_handle->Scene_buffers.m_memory)
		{
			vkUnmapMemory(
				_device->m_device,
				_handle->Scene_buffers.m_memory
			);
		}
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

		uint32_t hash_id = pipeline->_hashTable.Get(resource_name);

		if (hash_id == INVALID_ID)
		{
			TRC_CRITICAL("Can't set value for an invalid resource. please check if pipeline has been initialized {}", resource_name);
			return false;
		}


		trace::UniformMetaData& meta_data = pipeline->Scene_uniforms[hash_id];
		if (size > meta_data._size || size < meta_data._size)
			TRC_ERROR("Please ensure data size is not greater than resource data");
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
			char* data_point = (char*)_handle->cache_data;
			uint32_t location =  meta_data._offset + (_handle->cache_size * _device->m_imageIndex);

			void* map_point = data_point + location;
			memcpy(map_point, data, meta_data._size);
			return true;
		}



		char* data_point = _device->m_bufferData;
		uint32_t location = pipeline->Scence_struct[meta_data._struct_index].second + meta_data._offset;

		void* map_point = data_point + location;
		memcpy(map_point, data, meta_data._size);


		return result;
	}
	bool __SetPipelineTextureData(trace::GPipeline* pipeline, const std::string& resource_name, trace::ShaderResourceStage resource_scope, trace::GTexture* texture, uint32_t index)
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



		uint32_t hash_id = pipeline->_hashTable.Get(resource_name.c_str());

		if (hash_id == INVALID_ID)
		{
			TRC_CRITICAL("Can't set value for an invalid resource. please check if pipeline has been initialized");
			return false;
		}

		if (_handle->last_tex_update[_device->m_imageIndex] == texture)
			return false;

		trace::VKImage* _tex = reinterpret_cast<trace::VKImage*>(texture->GetRenderHandle()->m_internalData);


		VkWriteDescriptorSet write = {};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;


		trace::UniformMetaData& meta_data = pipeline->Scene_uniforms[hash_id];
		VkDescriptorImageInfo image_info = {};
		image_info.sampler = _tex->m_sampler;
		image_info.imageView = _tex->m_view;
		image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		write.descriptorCount = 1; //HACK: Fix
		write.dstBinding = meta_data._slot;
		write.pImageInfo = &image_info;
		write.dstArrayElement = index;
		switch (meta_data._resource_type)
		{
		case trace::ShaderResourceType::SHADER_RESOURCE_TYPE_COMBINED_SAMPLER:
		{
			write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			break;
		}
		}

		switch (resource_scope)
		{


		case trace::ShaderResourceStage::RESOURCE_STAGE_GLOBAL:
		{
			write.dstSet = _handle->Scene_sets[_device->m_imageIndex];

			break;
		}

		case trace::ShaderResourceStage::RESOURCE_STAGE_INSTANCE:
		{
			write.dstSet = _handle->Instance_sets[_device->m_imageIndex];
			break;
		}

		case trace::ShaderResourceStage::RESOURCE_STAGE_LOCAL:
		{
			write.dstSet = _handle->Instance_sets[_device->m_imageIndex];
			break;
		}


		}

		vkUpdateDescriptorSets(
			_device->m_device,
			1,
			&write,
			0,
			nullptr
		);
		_handle->last_tex_update[_device->m_imageIndex] = texture;


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

		if (_handle->Scene_sets[0])
		{
			_sets[set_count++] = _handle->Scene_sets[_device->m_imageIndex];
		}

		if (_handle->Instance_sets[0])
		{
			_sets[set_count++] = _handle->Instance_sets[_device->m_imageIndex];
		}

		if (_handle->Local_sets[0])
		{
			_sets[set_count++] = _handle->Local_sets[_device->m_imageIndex];
		}

		uint32_t offset_count = 0;
		uint32_t offsets[12] = {};

		for (auto& stct : pipeline->Scence_struct)
		{
			offsets[offset_count++] = stct.second;
		}

		vkCmdBindDescriptorSets(
			_device->m_graphicsCommandBuffers[_device->m_imageIndex].m_handle,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			_handle->m_layout,
			0,
			set_count,
			_sets,
			offset_count,
			offsets
		);

		return result;
		
	}

}