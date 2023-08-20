#include "pch.h"

#include "VKDevice.h"
#include "VKtypes.h"
#include "VkUtils.h"
#include "core/events/EventsSystem.h"
#include "core/Enums.h"
#include "core/input/Input.h"
#include "core/FileSystem.h"
#include "render/ShaderParser.h"
#include "core/Coretypes.h"
#include "render/Graphics.h"
#include "glm/gtc/matrix_transform.hpp"
#include "VulkanTexture.h"
#include "VulkanRenderPass.h"
#include "VulkanSwapchain.h"
#include "VulkanFramebuffer.h"

extern trace::VKHandle g_Vkhandle;
trace::VKDeviceHandle g_VkDevice;


namespace vk {

	bool __CreateDevice(trace::GDevice* device)
	{
		bool result = true;

		TRC_INFO("Added Vulkan Device Create Function :)");

		if (!device)
		{
			TRC_ERROR("please pass in valid pointer -> {}, Function -> {}", (const void*)device, __FUNCTION__);
			return false;
		}

		if (device->GetRenderHandle()->m_internalData)
		{
			TRC_WARN("These handle is valid can't recreate the device ::Try to destroy and then create, -> {}", (const void*)device->GetRenderHandle()->m_internalData);
			return false;
		}

		device->GetRenderHandle()->m_internalData = &g_VkDevice;

		trace::VKDeviceHandle* _handle = (trace::VKDeviceHandle*)device->GetRenderHandle()->m_internalData;
		// HACK: Find another way to get the vulkan instance
		trace::VKHandle* _instance = &g_Vkhandle;

		VK_ASSERT(vk::_CreateDevice(_handle, _instance));

		TRC_INFO("Created Vulkan Device");

		TRC_TRACE("Creating graphics command buffers");
		vk::_CreateCommandBuffers(_instance, _handle, _handle->m_graphicsCommandPool, _handle->m_graphicsCommandBuffers);
		TRC_INFO("Graphics command buffers created");

		// Sync Objects
		uint32_t frames_in_flight = _handle->frames_in_flight;
		std::vector<VkSemaphore>& image_avaliable_sem = _handle->m_imageAvailableSemaphores;
		std::vector<VkSemaphore>& queue_completed_sem = _handle->m_queueCompleteSemaphores;
		std::array<trace::VKFence, VK_MAX_NUM_FRAMES>& in_flight_fence = _handle->m_inFlightFence;
		std::array<trace::VKFence*, VK_MAX_NUM_FRAMES>& images_fence = _handle->m_imagesFence;

		image_avaliable_sem.resize(frames_in_flight);
		queue_completed_sem.resize(frames_in_flight);

		for (uint32_t i = 0; i < frames_in_flight; i++)
		{
			VkSemaphoreCreateInfo sem_create_info = {};
			sem_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

			VK_ASSERT(vkCreateSemaphore(_handle->m_device, &sem_create_info, _instance->m_alloc_callback, &image_avaliable_sem[i]));
			VK_ASSERT(vkCreateSemaphore(_handle->m_device, &sem_create_info, _instance->m_alloc_callback, &queue_completed_sem[i]));

			vk::_CreateFence(_instance, _handle, &in_flight_fence[i], true);
		}

		for (uint32_t i = 0; i < frames_in_flight; i++)
		{
			images_fence[i] = nullptr;
		}

		VkDescriptorPoolSize pool_sizes[2] = {};
		pool_sizes[0].descriptorCount = KB;
		pool_sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;

		pool_sizes[1].descriptorCount = KB;
		pool_sizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

		VkDescriptorPoolCreateInfo frame_pool = {};
		frame_pool.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		frame_pool.maxSets = KB * 2;
		frame_pool.pNext = nullptr;
		frame_pool.poolSizeCount = 2;
		frame_pool.pPoolSizes = pool_sizes;

		for (uint32_t i = 0; i < frames_in_flight; i++)
		{
			vkCreateDescriptorPool(
				_handle->m_device,
				&frame_pool,
				_instance->m_alloc_callback,
				&_handle->m_frameDescriptorPool[i]
			);
		}

		return result;
	}

	bool __DestroyDevice(trace::GDevice* device)
	{
		bool result = true;

		

		if (!device)
		{
			TRC_ERROR("please pass in valid pointer -> {}, Function -> {}", (const void*)device, __FUNCTION__);
			return false;
		}

		if (!device->GetRenderHandle()->m_internalData)
		{
			TRC_ERROR("These render handle is invalid -> {}, Function -> {}", (const void*)device->GetRenderHandle()->m_internalData, __FUNCTION__);
			return false;
		}

		trace::VKDeviceHandle* _handle = (trace::VKDeviceHandle*)device->GetRenderHandle()->m_internalData;
		// HACK: Find another way to get the vulkan instance
		trace::VKHandle* _instance = &g_Vkhandle;


		vkDeviceWaitIdle(_handle->m_device);

		//Render Graph Memory
		if (_handle->mem_flush)
		{
			vkFreeMemory(
				_handle->m_device,
				_handle->mem_flush,
				_instance->m_alloc_callback
			);
			_handle->mem_flush = VK_NULL_HANDLE;
		}

		vkFreeMemory(
			_handle->m_device,
			_handle->frame_memory,
			_instance->m_alloc_callback
		);
		_handle->frame_memory = VK_NULL_HANDLE;

		// Sync objects
		for (uint32_t i = 0; i < _handle->frames_in_flight; i++)
		{
			vkDestroySemaphore(_handle->m_device, _handle->m_imageAvailableSemaphores[i], _instance->m_alloc_callback);
			vkDestroySemaphore(_handle->m_device, _handle->m_queueCompleteSemaphores[i], _instance->m_alloc_callback);

		}

		for (auto& i : _handle->m_inFlightFence)
		{
			if(i.m_handle)
				vk::_DestroyFence(_instance, _handle, &i);
		}

		for (auto& i : _handle->m_frameDescriptorPool)
		{
			if(i)
				vkDestroyDescriptorPool(
					_handle->m_device,
					i,
					_instance->m_alloc_callback
				);
		}


		vk::_DestoryDevice(_handle, _instance);

		device->GetRenderHandle()->m_internalData = nullptr;

		return result;
	}

	bool __DrawElements(trace::GDevice* device, trace::GBuffer* vertex_buffer) 
	{

		bool result = true;

		

		if (!device || !vertex_buffer)
		{
			TRC_ERROR("please pass in valid pointer -> {} || {} , Function -> {}", (const void*)device, (const void*)vertex_buffer, __FUNCTION__);
			return false;
		}

		if (!device->GetRenderHandle()->m_internalData || !vertex_buffer->GetRenderHandle()->m_internalData)
		{
			TRC_ERROR("These render handle is invalid -> {} || {}, Function -> {}", (const void*)device->GetRenderHandle()->m_internalData, (const void*)vertex_buffer->GetRenderHandle()->m_internalData, __FUNCTION__);
			return false;
		}

		trace::VKDeviceHandle* _handle = (trace::VKDeviceHandle*)device->GetRenderHandle()->m_internalData;
		// HACK: Find another way to get the vulkan instance
		trace::VKHandle* _instance = &g_Vkhandle;


		return result;
	}
	bool __DrawInstanceElements(trace::GDevice* device, trace::GBuffer* vertex_buffer, uint32_t instances) 
	{ 
		
		bool result = true;

		

		if (!device)
		{
			TRC_ERROR("please pass in valid pointer -> {}, Function -> {}", (const void*)device, __FUNCTION__);
			return false;
		}

		if (!device->GetRenderHandle()->m_internalData)
		{
			TRC_ERROR("These render handle is invalid -> {}, Function -> {}", (const void*)device->GetRenderHandle()->m_internalData, __FUNCTION__);
			return false;
		}

		trace::VKDeviceHandle* _handle = (trace::VKDeviceHandle*)device->GetRenderHandle()->m_internalData;
		// HACK: Find another way to get the vulkan instance
		trace::VKHandle* _instance = &g_Vkhandle;

		return result; 
	}
	bool __DrawIndexed_(trace::GDevice* device, trace::GBuffer* index_buffer)
	{

		bool result = true;

		

		if (!device)
		{
			TRC_ERROR("please pass in valid pointer -> {}, Function -> {}", (const void*)device, __FUNCTION__);
			return false;
		}

		if (!device->GetRenderHandle()->m_internalData)
		{
			TRC_ERROR("These render handle is invalid -> {}, Function -> {}", (const void*)device->GetRenderHandle()->m_internalData, __FUNCTION__);
			return false;
		}

		trace::VKDeviceHandle* _handle = (trace::VKDeviceHandle*)device->GetRenderHandle()->m_internalData;
		// HACK: Find another way to get the vulkan instance
		trace::VKHandle* _instance = &g_Vkhandle;

		return result;
	}
	bool __DrawInstanceIndexed(trace::GDevice* device, trace::GBuffer* index_buffer, uint32_t instances)
	{

		bool result = true;

		

		if (!device)
		{
			TRC_ERROR("please pass in valid pointer -> {}, Function -> {}", (const void*)device, __FUNCTION__);
			return false;
		}

		if (!device->GetRenderHandle()->m_internalData)
		{
			TRC_ERROR("These render handle is invalid -> {}, Function -> {}", (const void*)device->GetRenderHandle()->m_internalData, __FUNCTION__);
			return false;
		}

		trace::VKDeviceHandle* _handle = (trace::VKDeviceHandle*)device->GetRenderHandle()->m_internalData;
		// HACK: Find another way to get the vulkan instance
		trace::VKHandle* _instance = &g_Vkhandle;

		return result;
	}
	bool __BindViewport(trace::GDevice* device, trace::Viewport view_port)
	{

		bool result = true;

		

		if (!device)
		{
			TRC_ERROR("please pass in valid pointer -> {}, Function -> {}", (const void*)device, __FUNCTION__);
			return false;
		}

		if (!device->GetRenderHandle()->m_internalData)
		{
			TRC_ERROR("These render handle is invalid -> {}, Function -> {}", (const void*)device->GetRenderHandle()->m_internalData, __FUNCTION__);
			return false;
		}

		trace::VKDeviceHandle* _handle = (trace::VKDeviceHandle*)device->GetRenderHandle()->m_internalData;
		// HACK: Find another way to get the vulkan instance
		trace::VKHandle* _instance = &g_Vkhandle;

		trace::VKCommmandBuffer* command_buffer = &_handle->m_graphicsCommandBuffers[_handle->m_imageIndex];

		VkViewport viewport = {};
		viewport.x = view_port.x;
		viewport.y = view_port.y;
		viewport.width = view_port.width;
		viewport.height = view_port.height;
		viewport.minDepth = view_port.minDepth;
		viewport.maxDepth = view_port.maxDepth;


		vkCmdSetViewport(command_buffer->m_handle, 0, 1, &viewport);


		return result;
	}
	bool __BindRect(trace::GDevice* device, trace::Rect2D rect)
	{

		bool result = true;

		

		if (!device)
		{
			TRC_ERROR("please pass in valid pointer -> {}, Function -> {}", (const void*)device, __FUNCTION__);
			return false;
		}

		if (!device->GetRenderHandle()->m_internalData)
		{
			TRC_ERROR("These render handle is invalid -> {}, Function -> {}", (const void*)device->GetRenderHandle()->m_internalData, __FUNCTION__);
			return false;
		}

		trace::VKDeviceHandle* _handle = (trace::VKDeviceHandle*)device->GetRenderHandle()->m_internalData;


		trace::VKCommmandBuffer* command_buffer = &_handle->m_graphicsCommandBuffers[_handle->m_imageIndex];


		VkRect2D scissor = {};
		scissor.offset.x = rect.top;
		scissor.offset.y = rect.left;
		scissor.extent.width = rect.right;
		scissor.extent.height = rect.bottom;

		vkCmdSetScissor(command_buffer->m_handle, 0, 1, &scissor);

		return result;
	}
	bool __BindPipeline(trace::GDevice* device, trace::GPipeline* pipeline)
	{

		bool result = true;

		

		if (!device)
		{
			TRC_ERROR("please pass in valid pointer -> {}, Function -> {}", (const void*)device, __FUNCTION__);
			return false;
		}

		if (!device->GetRenderHandle()->m_internalData)
		{
			TRC_ERROR("These render handle is invalid -> {}, Function -> {}", (const void*)device->GetRenderHandle()->m_internalData, __FUNCTION__);
			return false;
		}

		trace::VKDeviceHandle* _handle = (trace::VKDeviceHandle*)device->GetRenderHandle()->m_internalData;

		device->m_pipeline = pipeline;
		trace::VKPipeline* _pipeline = reinterpret_cast<trace::VKPipeline*>(pipeline->GetRenderHandle()->m_internalData);
		trace::VKCommmandBuffer* command_buffer = &_handle->m_graphicsCommandBuffers[_handle->m_imageIndex];
		vkCmdBindPipeline(command_buffer->m_handle, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline->m_handle);

		return result;
	}
	bool __BindVertexBuffer(trace::GDevice* device, trace::GBuffer* buffer)
	{

		bool result = true;

		

		if (!device || !buffer)
		{
			TRC_ERROR("please pass in valid pointer -> {} || {}, Function -> {}", (const void*)device, (const void*)buffer, __FUNCTION__);
			return false;
		}

		if (!device->GetRenderHandle()->m_internalData || !buffer->GetRenderHandle()->m_internalData)
		{
			TRC_ERROR("These render handle is invalid -> {} || {}, Function -> {}", (const void*)device->GetRenderHandle()->m_internalData, (const void*)!buffer->GetRenderHandle()->m_internalData, __FUNCTION__);
			return false;
		}

		trace::VKDeviceHandle* _handle = (trace::VKDeviceHandle*)device->GetRenderHandle()->m_internalData;


		trace::VKBuffer* buf = reinterpret_cast<trace::VKBuffer*>(buffer->GetRenderHandle()->m_internalData);
		trace::VKCommmandBuffer* command_buffer = &_handle->m_graphicsCommandBuffers[_handle->m_imageIndex];

		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(command_buffer->m_handle, 0, 1, &buf->m_handle, offsets);

		return result;
	}
	bool __BindIndexBuffer(trace::GDevice* device, trace::GBuffer* buffer)
	{

		bool result = true;

		

		if (!device || !buffer)
		{
			TRC_ERROR("please pass in valid pointer -> {} || {}, Function -> {}", (const void*)device, (const void*)buffer, __FUNCTION__);
			return false;
		}

		if (!device->GetRenderHandle()->m_internalData || !buffer->GetRenderHandle()->m_internalData)
		{
			TRC_ERROR("These render handle is invalid -> {} || {}, Function -> {}", (const void*)device->GetRenderHandle()->m_internalData, (const void*)!buffer->GetRenderHandle()->m_internalData, __FUNCTION__);
			return false;
		}

		trace::VKDeviceHandle* _handle = (trace::VKDeviceHandle*)device->GetRenderHandle()->m_internalData;


		trace::VKBuffer* buf = reinterpret_cast<trace::VKBuffer*>(buffer->GetRenderHandle()->m_internalData);
		trace::VKCommmandBuffer* command_buffer = &_handle->m_graphicsCommandBuffers[_handle->m_imageIndex];

		vkCmdBindIndexBuffer(command_buffer->m_handle, buf->m_handle, 0, VK_INDEX_TYPE_UINT32);

		return result;
	}
	bool __Draw(trace::GDevice* device, uint32_t start_vertex, uint32_t count)
	{

		bool result = true;

		

		if (!device)
		{
			TRC_ERROR("please pass in valid pointer -> {}, Function -> {}", (const void*)device, __FUNCTION__);
			return false;
		}

		if (!device->GetRenderHandle()->m_internalData)
		{
			TRC_ERROR("These render handle is invalid -> {}, Function -> {}", (const void*)device->GetRenderHandle()->m_internalData, __FUNCTION__);
			return false;
		}

		trace::VKDeviceHandle* _handle = (trace::VKDeviceHandle*)device->GetRenderHandle()->m_internalData;
		

		trace::VKCommmandBuffer* command_buffer = &_handle->m_graphicsCommandBuffers[_handle->m_imageIndex];

		vkCmdDraw(
			command_buffer->m_handle,
			count,
			1,
			start_vertex,
			0
		);

		return result;
	}
	bool __DrawIndexed(trace::GDevice* device, uint32_t first_index, uint32_t count)
	{

		bool result = true;

		

		if (!device)
		{
			TRC_ERROR("please pass in valid pointer -> {}, Function -> {}", (const void*)device, __FUNCTION__);
			return false;
		}

		if (!device->GetRenderHandle()->m_internalData)
		{
			TRC_ERROR("These render handle is invalid -> {}, Function -> {}", (const void*)device->GetRenderHandle()->m_internalData, __FUNCTION__);
			return false;
		}

		trace::VKDeviceHandle* _handle = (trace::VKDeviceHandle*)device->GetRenderHandle()->m_internalData;
		
		trace::VKCommmandBuffer* command_buffer = &_handle->m_graphicsCommandBuffers[_handle->m_imageIndex];

		vkCmdDrawIndexed(
			command_buffer->m_handle,
			count,
			1,
			first_index,
			0,
			0
		);

		return result;
	}
	bool __BeginRenderPass(trace::GDevice* device, trace::GRenderPass* render_pass, trace::GFramebuffer* frame_buffer)
	{

		bool result = true;

		

		if (!device || !render_pass || !frame_buffer)
		{
			TRC_ERROR("please pass in valid pointer -> {} || {} || {}, Function -> {}", (const void*)device, (const void*)render_pass, (const void*)frame_buffer, __FUNCTION__);
			return false;
		}

		if (!device->GetRenderHandle()->m_internalData || !render_pass->GetRenderHandle()->m_internalData || !frame_buffer->GetRenderHandle()->m_internalData)
		{
			TRC_ERROR("These render handle is invalid -> {} || {} || {}, Function -> {}", (const void*)device->GetRenderHandle()->m_internalData, (const void*)!render_pass->GetRenderHandle()->m_internalData, (const void*)!frame_buffer->GetRenderHandle()->m_internalData, __FUNCTION__);
			return false;
		}

		trace::VKDeviceHandle* _handle = (trace::VKDeviceHandle*)device->GetRenderHandle()->m_internalData;
		// HACK: Find another way to get the vulkan instance
		trace::VKHandle* _instance = &g_Vkhandle;
		
		trace::VKRenderPass* pass = reinterpret_cast<trace::VKRenderPass*>(render_pass->GetRenderHandle()->m_internalData);
		trace::Framebuffer_VK* frameBuffer = reinterpret_cast<trace::Framebuffer_VK*>(frame_buffer->GetRenderHandle()->m_internalData);
		trace::VKCommmandBuffer* command_buffer = &_handle->m_graphicsCommandBuffers[_handle->m_imageIndex];

		VkRenderPassBeginInfo begin_info = {};
		begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		begin_info.renderPass = pass->m_handle;
		begin_info.renderArea.offset.x = pass->render_area->x;
		begin_info.renderArea.offset.y = pass->render_area->y;
		begin_info.renderArea.extent.width = pass->render_area->z;
		begin_info.renderArea.extent.height = pass->render_area->w;
		begin_info.framebuffer = frameBuffer->m_handle[_handle->m_imageIndex].m_handle;

		VkClearValue clear_colors[2] = {};
		clear_colors[0].color.float32[0] = pass->clear_color->r;
		clear_colors[0].color.float32[1] = pass->clear_color->g;
		clear_colors[0].color.float32[2] = pass->clear_color->b;
		clear_colors[0].color.float32[3] = pass->clear_color->a;

		clear_colors[1].depthStencil.depth = *pass->depth_value;
		clear_colors[1].depthStencil.stencil = *pass->stencil_value;

		begin_info.clearValueCount = 2;
		begin_info.pClearValues = clear_colors;

		vkCmdBeginRenderPass(command_buffer->m_handle, &begin_info, VK_SUBPASS_CONTENTS_INLINE);
		command_buffer->m_state = trace::CommandBufferState::COMMAND_IN_RENDER_PASS;

		return result;
	}
	bool __NextSubpass(trace::GDevice* device, trace::GRenderPass* render_pass)
	{

		bool result = true;

		

		if (!device)
		{
			TRC_ERROR("please pass in valid pointer -> {}, Function -> {}", (const void*)device, __FUNCTION__);
			return false;
		}

		if (!device->GetRenderHandle()->m_internalData)
		{
			TRC_ERROR("These render handle is invalid -> {}, Function -> {}", (const void*)device->GetRenderHandle()->m_internalData, __FUNCTION__);
			return false;
		}

		trace::VKDeviceHandle* _handle = (trace::VKDeviceHandle*)device->GetRenderHandle()->m_internalData;
		// HACK: Find another way to get the vulkan instance
		trace::VKHandle* _instance = &g_Vkhandle;

		return result;
	}
	bool __EndRenderPass(trace::GDevice* device, trace::GRenderPass* render_pass)
	{

		bool result = true;

		

		if (!device || !render_pass)
		{
			TRC_ERROR("please pass in valid pointer -> {} || {}, Function -> {}", (const void*)device, (const void*)render_pass, __FUNCTION__);
			return false;
		}

		if (!device->GetRenderHandle()->m_internalData || !render_pass->GetRenderHandle()->m_internalData)
		{
			TRC_ERROR("These render handle is invalid -> {} || {}, Function -> {}", (const void*)device->GetRenderHandle()->m_internalData, (const void*)render_pass->GetRenderHandle()->m_internalData, __FUNCTION__);
			return false;
		}

		trace::VKDeviceHandle* _handle = (trace::VKDeviceHandle*)device->GetRenderHandle()->m_internalData;
		// HACK: Find another way to get the vulkan instance
		trace::VKHandle* _instance = &g_Vkhandle;

		trace::VKCommmandBuffer* command_buffer = &_handle->m_graphicsCommandBuffers[_handle->m_imageIndex];

		vk::_EndRenderPass(_instance, _handle, command_buffer);

		return result;
	}
	bool __BeginFrame(trace::GDevice* device, trace::GSwapchain* swapchain)
	{

		bool result = true;

		

		if (!device || !swapchain)
		{
			TRC_ERROR("please pass in valid pointer -> {} || {}, Function -> {}", (const void*)device, (const void*)swapchain, __FUNCTION__);
			return false;
		}

		if (!device->GetRenderHandle()->m_internalData || !swapchain->GetRenderHandle()->m_internalData)
		{
			TRC_ERROR("These render handle is invalid -> {} || {}, Function -> {}", (const void*)device->GetRenderHandle()->m_internalData, (const void*)swapchain->GetRenderHandle()->m_internalData, __FUNCTION__);
			return false;
		}

		trace::VKDeviceHandle* _handle = (trace::VKDeviceHandle*)device->GetRenderHandle()->m_internalData;
		// HACK: Find another way to get the vulkan instance
		trace::VKHandle* _instance = &g_Vkhandle;

		trace::VKSwapChain* swap_chain = reinterpret_cast<trace::VKSwapChain*>(swapchain->GetRenderHandle()->m_internalData);

		if (swap_chain->m_recreating)
		{
			vkDeviceWaitIdle(_handle->m_device);

			if(_recreate_swapchain(swapchain))
			{
				TRC_ERROR("Unable to recreate swapchain");
				return false;
			}
			TRC_INFO("Recreating Swapchain...");
			return false;
		}

		uint32_t frame_offset = uint32_t(MB / (_handle->frames_in_flight)) * _handle->m_imageIndex;
		_handle->m_bufCurrentOffset = frame_offset;


		if (!vk::_WaitFence(_instance, _handle, &_handle->m_inFlightFence[_handle->m_currentFrame], UINT64_MAX))
		{
			TRC_WARN("Fence timeout or wait failure");
			return false;
		}

		if (!vk::_AcquireSwapchainImage(_instance, _handle, swap_chain, _handle->m_imageAvailableSemaphores[_handle->m_currentFrame], nullptr, &_handle->m_imageIndex, UINT64_MAX))
		{
			return false;
		}

		trace::VKCommmandBuffer* command_buffer = &_handle->m_graphicsCommandBuffers[_handle->m_imageIndex];
		vk::_CommandBuffer_Reset(command_buffer);
		trace::CommandBufferUsage command_use = trace::CommandBufferUsage::NO_USE;
		vk::_BeginCommandBuffer(command_buffer, command_use);

		return result;
	}
	bool __EndFrame(trace::GDevice* device)
	{

		bool result = true;

		

		if (!device)
		{
			TRC_ERROR("please pass in valid pointer -> {}, Function -> {}", (const void*)device, __FUNCTION__);
			return false;
		}

		if (!device->GetRenderHandle()->m_internalData)
		{
			TRC_ERROR("These render handle is invalid -> {}, Function -> {}", (const void*)device->GetRenderHandle()->m_internalData, __FUNCTION__);
			return false;
		}

		trace::VKDeviceHandle* _handle = (trace::VKDeviceHandle*)device->GetRenderHandle()->m_internalData;
		// HACK: Find another way to get the vulkan instance
		trace::VKHandle* _instance = &g_Vkhandle;


		trace::VKCommmandBuffer* command_buffer = &_handle->m_graphicsCommandBuffers[_handle->m_imageIndex];

		uint32_t frame_offset = uint32_t(MB / (_handle->frames_in_flight)) * _handle->m_imageIndex;
		char* from = _handle->m_bufferData + frame_offset;
		char* to = (char*)_handle->m_bufferPtr + frame_offset;

		memcpy(to, from, uint32_t(MB / (_handle->frames_in_flight)));

		vk::_EndCommandBuffer(command_buffer);

		if (_handle->m_imagesFence[_handle->m_imageIndex] != nullptr)
		{
			vk::_WaitFence(_instance, _handle, _handle->m_imagesFence[_handle->m_imageIndex], UINT64_MAX);
		}

		_handle->m_imagesFence[_handle->m_imageIndex] = &_handle->m_inFlightFence[_handle->m_currentFrame];

		vk::_FenceReset(_handle, &_handle->m_inFlightFence[_handle->m_currentFrame]);

		VkSubmitInfo sub_info = {};
		sub_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		sub_info.commandBufferCount = 1;
		sub_info.pCommandBuffers = &command_buffer->m_handle;
		sub_info.waitSemaphoreCount = 1;
		sub_info.pWaitSemaphores = &_handle->m_imageAvailableSemaphores[_handle->m_currentFrame];
		sub_info.signalSemaphoreCount = 1;
		sub_info.pSignalSemaphores = &_handle->m_queueCompleteSemaphores[_handle->m_currentFrame];

		VkPipelineStageFlags pipeline_flag[1] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		sub_info.pWaitDstStageMask = pipeline_flag;

		VkResult _result = vkQueueSubmit(_handle->m_graphicsQueue, 1, &sub_info, _handle->m_inFlightFence[_handle->m_currentFrame].m_handle);

		if (_result != VK_SUCCESS)
		{
			TRC_ERROR("Unable to submit command buffer");
			return false;
		}

		vk::_CommandBufferSubmitted(command_buffer);

		//vk::_PresentSwapchainImage(m_instance, m_handle, &m_handle->m_swapChain, m_handle->m_graphicsQueue, m_handle->m_presentQueue, m_handle->m_queueCompleteSemaphores[m_handle->m_currentFrame], &m_handle->m_imageIndex);

		return result;
	}

	bool __OnDrawStart(trace::GDevice* device, trace::GPipeline* pipeline)
	{
		bool result = true;



		if (!device || !pipeline)
		{
			TRC_ERROR("please pass in valid pointer -> {} || {}, Function -> {}", (const void*)device, (const void*)pipeline, __FUNCTION__);
			return false;
		}

		if (!device->GetRenderHandle()->m_internalData || !pipeline->GetRenderHandle()->m_internalData)
		{
			TRC_ERROR("These render handle is invalid -> {} || {}, Function -> {}", (const void*)device->GetRenderHandle()->m_internalData, (const void*)pipeline->GetRenderHandle()->m_internalData, __FUNCTION__);
			return false;
		}

		trace::VKDeviceHandle* _handle = (trace::VKDeviceHandle*)device->GetRenderHandle()->m_internalData;
		// HACK: Find another way to get the vulkan instance
		trace::VKHandle* _instance = &g_Vkhandle;
		trace::VKPipeline* pipe_handle = reinterpret_cast<trace::VKPipeline*>(pipeline->GetRenderHandle()->m_internalData);

		trace::VKCommmandBuffer* command_buffer = &_handle->m_graphicsCommandBuffers[_handle->m_imageIndex];
		

		for (auto& stct : pipeline->Scence_struct)
		{
			stct.second = _handle->m_bufCurrentOffset;
			_handle->m_bufCurrentOffset += stct.first;
		}

		return result;
	}

	bool __OnDrawEnd(trace::GDevice* device, trace::GPipeline* pipeline)
	{
		bool result = true;

		if (!device || !pipeline)
		{
			TRC_ERROR("please pass in valid pointer -> {} || {}, Function -> {}", (const void*)device, (const void*)pipeline, __FUNCTION__);
			return false;
		}

		if (!device->GetRenderHandle()->m_internalData || !pipeline->GetRenderHandle()->m_internalData)
		{
			TRC_ERROR("These render handle is invalid -> {} || {}, Function -> {}", (const void*)device->GetRenderHandle()->m_internalData, (const void*)pipeline->GetRenderHandle()->m_internalData, __FUNCTION__);
			return false;
		}

		trace::VKDeviceHandle* _handle = (trace::VKDeviceHandle*)device->GetRenderHandle()->m_internalData;
		// HACK: Find another way to get the vulkan instance
		trace::VKHandle* _instance = &g_Vkhandle;
		trace::VKPipeline* pipe_handle = reinterpret_cast<trace::VKPipeline*>(pipeline->GetRenderHandle()->m_internalData);

		trace::VKCommmandBuffer* command_buffer = &_handle->m_graphicsCommandBuffers[_handle->m_imageIndex];

		for (auto& stct : pipeline->Scence_struct)
		{
			stct.second = INVALID_ID;
		}

		return result;
	}

}