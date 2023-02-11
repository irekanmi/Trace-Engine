#include "pch.h"

#include "VKDevice.h"
#include "VKtypes.h"
#include "VkUtils.h"
#include "core/Application.h"
#include "core/events/EventsSystem.h"
#include "core/Enums.h"
#include "core/input/Input.h"
#include "core/FileSystem.h"
#include "render/ShaderParser.h"
#include "core/Coretypes.h"
#include "render/Graphics.h"
#include "glm/gtc/matrix_transform.hpp"
#include "VulkanTexture.h"

extern trace::VKHandle g_Vkhandle;
trace::VKDeviceHandle g_VkDevice;
namespace trace {


	VKDevice::VKDevice()
	{
		m_instance = &g_Vkhandle;
		m_handle = &g_VkDevice;
	}

	VKDevice::~VKDevice()
	{
	}

	bool VKDevice::Init()
	{
		VK_ASSERT(vk::_CreateDevice(m_handle, m_instance));

		TRC_INFO("Created Vulkan Device");

		TRC_TRACE("Creating Swapchain...");
		VK_ASSERT(vk::_CreateSwapchain(m_instance, m_handle, &m_handle->m_swapChain, Application::get_instance()->GetWindow()->GetWidth(), Application::get_instance()->GetWindow()->GetHeight()));

		TRC_INFO("Vulkan SwapChain Created");

		TRC_TRACE("Creating Render Pass...");
		vk::_CreateRenderPass(
			m_instance,
			m_handle,
			&m_handle->m_renderPass,
			glm::vec4(0.0f, 0.6f, .2f, 1.0f),
			glm::vec4(0, 0, Application::get_instance()->GetWindow()->GetWidth(), Application::get_instance()->GetWindow()->GetHeight()),
			1.0f,
			0,
			&m_handle->m_swapChain
		);
		TRC_INFO("Vulkan Render Pass Created");

		vk::_RegenerateFrameBuffers(m_instance, m_handle, &m_handle->m_swapChain, &m_handle->m_renderPass);
		TRC_INFO("Generated render frame buffers");

		TRC_TRACE("Creating graphics command buffers");
		vk::_CreateCommandBuffers(m_instance, m_handle, m_handle->m_graphicsCommandPool, m_handle->m_graphicsCommandBuffers);
		TRC_INFO("Graphics command buffers created");

		// Sync Objects
		uint32_t frames_in_flight = m_handle->frames_in_flight;
		eastl::vector<VkSemaphore>& image_avaliable_sem = m_handle->m_imageAvailableSemaphores;
		eastl::vector<VkSemaphore>& queue_completed_sem = m_handle->m_queueCompleteSemaphores;
		eastl::vector<VKFence>& in_flight_fence = m_handle->m_inFlightFence;
		eastl::vector<VKFence*>& images_fence = m_handle->m_imagesFence;

		image_avaliable_sem.resize(frames_in_flight);
		queue_completed_sem.resize(frames_in_flight);
		in_flight_fence.resize(frames_in_flight);
		images_fence.resize(m_handle->m_frameBufferHeight);
		
		for (uint32_t i = 0; i < frames_in_flight; i++)
		{
			VkSemaphoreCreateInfo sem_create_info = {};
			sem_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
			
			VK_ASSERT(vkCreateSemaphore(m_handle->m_device,&sem_create_info, m_instance->m_alloc_callback, &image_avaliable_sem[i]));
			VK_ASSERT(vkCreateSemaphore(m_handle->m_device, &sem_create_info, m_instance->m_alloc_callback, &queue_completed_sem[i]));

			vk::_CreateFence(m_instance, m_handle, &in_flight_fence[i], true);
		}

		for (uint32_t i = 0; i < m_handle->m_frameBufferHeight; i++)
		{
			images_fence[i] = nullptr;
		}
		
		EventsSystem::get_instance()->AddEventListener(EventType::TRC_WND_RESIZE, BIND_EVENT_FN(VKDevice::OnEvent));
		EventsSystem::get_instance()->AddEventListener(EventType::TRC_MOUSE_MOVE, BIND_EVENT_FN(VKDevice::OnEvent));



		return true;
	}

	void VKDevice::DrawElements(GBuffer* vertex_buffer)
	{
	}

	void VKDevice::DrawInstanceElements(GBuffer* vertex_buffer, uint32_t instances)
	{
	}

	void VKDevice::DrawIndexed(GBuffer* index_buffer)
	{
	}

	void VKDevice::DrawInstanceIndexed(GBuffer* index_buffer, uint32_t instances)
	{
	}

	void VKDevice::ShutDown()
	{
		vkDeviceWaitIdle(m_handle->m_device);




		// Sync objects
		for (uint32_t i = 0; i < m_handle->frames_in_flight; i++)
		{
			vkDestroySemaphore(m_handle->m_device, m_handle->m_imageAvailableSemaphores[i], m_instance->m_alloc_callback);
			vkDestroySemaphore(m_handle->m_device, m_handle->m_queueCompleteSemaphores[i], m_instance->m_alloc_callback);

		}

		for (auto& i : m_handle->m_inFlightFence)
		{
			vk::_DestroyFence(m_instance, m_handle, &i);
		}

		// Destroy frame Buffers
		for (VKFrameBuffer& i : m_handle->m_swapChain.m_frameBuffers)
		{
			vk::_DestoryFrameBuffer(m_instance, m_handle, &i);
		}

		vk::_DestroyRenderPass(m_instance, m_handle, &m_handle->m_renderPass);
		vk::_DestroySwapchain(m_instance, m_handle, &m_handle->m_swapChain);
		vk::_DestoryDevice(m_handle, m_instance);
	}

	void VKDevice::UpdateSceneGlobalData(void* data, uint32_t size, uint32_t slot, uint32_t index)
	{
		VulkanPipeline* pipeline = reinterpret_cast<VulkanPipeline*>(m_pipeline);

		VKPipeline& _pipe = pipeline->m_handle;

		VkWriteDescriptorSet write = {};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;

		switch (_pipe.Scene_bindings[slot].descriptorType)
		{
		case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
		{
			void* _data;
			vkMapMemory(m_handle->m_device, _pipe.Scene_buffers[slot].m_memory, 0, size, 0, &_data);
			memcpy(_data, data, size);
			vkUnmapMemory(m_handle->m_device, _pipe.Scene_buffers[slot].m_memory);

			VkDescriptorBufferInfo buf_info = {};
			buf_info.offset = 0;
			buf_info.range = size;
			buf_info.buffer = _pipe.Scene_buffers[slot].m_handle;

			write.descriptorCount = _pipe.Scene_bindings[slot].descriptorCount;
			write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			write.dstBinding = _pipe.Scene_bindings[slot].binding;
			write.dstSet = _pipe.Scene_sets[m_handle->m_imageIndex];
			write.pBufferInfo = &buf_info;

			break;
		}
		}

		vkUpdateDescriptorSets(m_handle->m_device, 1, &write, 0, nullptr);

	}

	void VKDevice::UpdateSceneGlobalData(SceneGlobals data, uint32_t slot, uint32_t index)
	{
		VulkanPipeline* pipeline = reinterpret_cast<VulkanPipeline*>(m_pipeline);

		VKPipeline& _pipe = pipeline->m_handle;

		VkWriteDescriptorSet write = {};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;

		switch (_pipe.Scene_bindings[slot].descriptorType)
		{
		case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
		{
			void* _data;
			vkMapMemory(m_handle->m_device, _pipe.Scene_buffers[slot].m_memory, 0, sizeof(SceneGlobals), 0, &_data);
			memcpy(_data, &data, sizeof(SceneGlobals));
			vkUnmapMemory(m_handle->m_device, _pipe.Scene_buffers[slot].m_memory);

			VkDescriptorBufferInfo buf_info = {};
			buf_info.offset = 0;
			buf_info.range = sizeof(SceneGlobals);
			buf_info.buffer = _pipe.Scene_buffers[slot].m_handle;

			write.descriptorCount = _pipe.Scene_bindings[slot].descriptorCount;
			write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			write.dstBinding = _pipe.Scene_bindings[slot].binding;
			write.dstSet = _pipe.Scene_sets[m_handle->m_imageIndex];
			write.pBufferInfo = &buf_info;

			break;
		}
		}

		vkUpdateDescriptorSets(m_handle->m_device, 1, &write, 0, nullptr);

	}

	void VKDevice::UpdateSceneGlobalTexture(GTexture* texture, uint32_t slot, uint32_t index)
	{
		VulkanPipeline* pipeline = reinterpret_cast<VulkanPipeline*>(m_pipeline);
		VulkanTexture* _tex = reinterpret_cast<VulkanTexture*>(texture);

		VKPipeline& _pipe = pipeline->m_handle;

		VkWriteDescriptorSet write = {};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;

		switch (_pipe.Scene_bindings[slot].descriptorType)
		{
		case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
		{


			VkDescriptorImageInfo image_info = {};
			image_info.sampler = _tex->m_sampler;
			image_info.imageView = _tex->m_handle.m_view;
			image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			write.descriptorCount = _pipe.Scene_bindings[slot].descriptorCount;
			write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			write.dstBinding = _pipe.Scene_bindings[slot].binding;
			write.dstSet = _pipe.Scene_sets[m_handle->m_imageIndex];
			write.pImageInfo = &image_info;

			break;
		}
		}

		vkUpdateDescriptorSets(m_handle->m_device, 1, &write, 0, nullptr);

	}

	void VKDevice::UpdateSceneGlobalTextures(GTexture* texture, uint32_t count, uint32_t slot, uint32_t index)
	{
	}

	void VKDevice::BindPipeline(GPipeline* pipeline)
	{
		m_pipeline = pipeline;

		VulkanPipeline* _pipeline = reinterpret_cast<VulkanPipeline*>(m_pipeline);


		VKCommmandBuffer* command_buffer = &m_handle->m_graphicsCommandBuffers[m_handle->m_imageIndex];


		vkCmdBindPipeline(command_buffer->m_handle, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline->m_handle.m_handle);

		vkCmdBindDescriptorSets(
			command_buffer->m_handle,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			_pipeline->m_handle.m_layout,
			0,
			1,
			&_pipeline->m_handle.Scene_sets[m_handle->m_imageIndex],
			0,
			nullptr
		);

	}

	void VKDevice::BindVertexBuffer(GBuffer* buffer)
	{
		VulkanBuffer* buf = reinterpret_cast<VulkanBuffer*>(buffer);
		VKCommmandBuffer* command_buffer = &m_handle->m_graphicsCommandBuffers[m_handle->m_imageIndex];

		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(command_buffer->m_handle, 0, 1, &buf->m_handle.m_handle, offsets);

	}

	void VKDevice::BindIndexBuffer(GBuffer* buffer)
	{
		VulkanBuffer* buf = reinterpret_cast<VulkanBuffer*>(buffer);
		VKCommmandBuffer* command_buffer = &m_handle->m_graphicsCommandBuffers[m_handle->m_imageIndex];

		vkCmdBindIndexBuffer(command_buffer->m_handle, buf->m_handle.m_handle, 0, VK_INDEX_TYPE_UINT32);
	}

	void VKDevice::Draw(uint32_t start_vertex, uint32_t count)
	{
		VKCommmandBuffer* command_buffer = &m_handle->m_graphicsCommandBuffers[m_handle->m_imageIndex];

		vkCmdDraw(
			command_buffer->m_handle,
			count,
			1,
			start_vertex,
			0
		);
	}

	void VKDevice::DrawIndexed(uint32_t first_index, uint32_t count)
	{
		VKCommmandBuffer* command_buffer = &m_handle->m_graphicsCommandBuffers[m_handle->m_imageIndex];

		vkCmdDrawIndexed(
			command_buffer->m_handle,
			count,
			1,
			first_index,
			0,
			0
		);
	}

	bool VKDevice::BeginFrame()
	{
		
		if (m_handle->m_recreatingSwapcahin)
		{
			vkDeviceWaitIdle(m_handle->m_device);

			if (!recreateSwapchain())
			{
				TRC_ERROR("Unable to recreate swapchain");
				return false;
			}
			TRC_INFO("Recreating Swapchain...");
			return false;
		}



		if (!vk::_WaitFence(m_instance, m_handle, &m_handle->m_inFlightFence[m_handle->m_currentFrame], UINT64_MAX))
		{
			TRC_WARN("Fence timeout or wait failure");
			return false;
		}

		if (!vk::_AcquireSwapchainImage(m_instance, m_handle, &m_handle->m_swapChain, m_handle->m_imageAvailableSemaphores[m_handle->m_currentFrame], nullptr, &m_handle->m_imageIndex, UINT64_MAX))
		{
			return false;
		}

		VKCommmandBuffer* command_buffer = &m_handle->m_graphicsCommandBuffers[m_handle->m_imageIndex];
		vk::_CommandBuffer_Reset(command_buffer);
		CommandBufferUsage command_use = CommandBufferUsage::NO_USE;
		vk::_BeginCommandBuffer(command_buffer, command_use);

		VkViewport viewport = {};
		viewport.x = 0;
		viewport.y = m_handle->m_frameBufferHeight;
		viewport.width = m_handle->m_frameBufferWidth;
		viewport.height = -(float)m_handle->m_frameBufferHeight;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor = {};
		scissor.offset.x = scissor.offset.y = 0;
		scissor.extent.width = m_handle->m_frameBufferWidth;
		scissor.extent.height = m_handle->m_frameBufferHeight;

		vkCmdSetViewport(command_buffer->m_handle, 0, 1, &viewport);
		vkCmdSetScissor(command_buffer->m_handle, 0, 1, &scissor);

		vk::_BeginRenderPass(m_instance, m_handle, &m_handle->m_renderPass, command_buffer, m_handle->m_swapChain.m_frameBuffers[m_handle->m_imageIndex].m_handle);

		m_handle->m_renderPass.render_area.z = m_handle->m_frameBufferWidth;
		m_handle->m_renderPass.render_area.w = m_handle->m_frameBufferHeight;




		return true;
	}

	void VKDevice::EndFrame()
	{


		VKCommmandBuffer* command_buffer = &m_handle->m_graphicsCommandBuffers[m_handle->m_imageIndex];

		vk::_EndRenderPass(m_instance, m_handle, command_buffer);

		vk::_EndCommandBuffer(command_buffer);

		if (m_handle->m_imagesFence[m_handle->m_imageIndex] != nullptr)
		{
			vk::_WaitFence(m_instance, m_handle, m_handle->m_imagesFence[m_handle->m_imageIndex], UINT64_MAX);
		}

		m_handle->m_imagesFence[m_handle->m_imageIndex] = &m_handle->m_inFlightFence[m_handle->m_currentFrame];

		vk::_FenceReset(m_handle, &m_handle->m_inFlightFence[m_handle->m_currentFrame]);

		VkSubmitInfo sub_info = {};
		sub_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		sub_info.commandBufferCount = 1;
		sub_info.pCommandBuffers = &command_buffer->m_handle;
		sub_info.waitSemaphoreCount = 1;
		sub_info.pWaitSemaphores = &m_handle->m_imageAvailableSemaphores[m_handle->m_currentFrame];
		sub_info.signalSemaphoreCount = 1;
		sub_info.pSignalSemaphores = &m_handle->m_queueCompleteSemaphores[m_handle->m_currentFrame];
		
		VkPipelineStageFlags pipeline_flag[1] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		sub_info.pWaitDstStageMask = pipeline_flag;

		VkResult result = vkQueueSubmit(m_handle->m_graphicsQueue, 1, &sub_info, m_handle->m_inFlightFence[m_handle->m_currentFrame].m_handle);

		if (result != VK_SUCCESS)
		{
			TRC_ERROR("Unable to submit command buffer");
			return;
		}

		vk::_CommandBufferSubmitted(command_buffer);

		vk::_PresentSwapchainImage(m_instance, m_handle, &m_handle->m_swapChain, m_handle->m_graphicsQueue, m_handle->m_presentQueue, m_handle->m_queueCompleteSemaphores[m_handle->m_currentFrame], &m_handle->m_imageIndex);


		m_handle->m_currentFrame = (m_handle->m_currentFrame + 1) % m_handle->frames_in_flight;
		

	}

	void VKDevice::OnEvent(Event* p_Event)
	{

		switch (p_Event->m_type)
		{
		case EventType::TRC_WND_RESIZE:
		{
			WindowResize* resize = reinterpret_cast<WindowResize*>(p_Event);
			m_handle->m_recreatingSwapcahin = true;
			m_handle->m_frameBufferWidth = resize->m_width;
			m_handle->m_frameBufferHeight = resize->m_height;
			break;
		}

		case EventType::TRC_MOUSE_MOVE:
		{
			MouseMove* move = reinterpret_cast<MouseMove*>(p_Event);
			
			bool control_key = InputSystem::get_instance()->GetKeyState(Keys::KEY_LCONTROL) == KeyState::KEY_HELD || InputSystem::get_instance()->GetKeyState(Keys::KEY_RCONTROL) == KeyState::KEY_HELD || InputSystem::get_instance()->GetKeyState(Keys::KEY_CONTROL) == KeyState::KEY_HELD;
			if (control_key)
			{

				float r = (float)(move->m_x / m_handle->m_frameBufferWidth);
				float g = (float)(move->m_y / m_handle->m_frameBufferHeight);
				float b = (g + r) / 2;
				float a = (r + g + b) / 3;

				m_handle->m_renderPass.clear_color.r = r;
				m_handle->m_renderPass.clear_color.g = g;
				m_handle->m_renderPass.clear_color.b = b;
				m_handle->m_renderPass.clear_color.a = a;

			}

			break;
		}
		}


	}

	bool VKDevice::recreateSwapchain()
	{


		if (m_handle->m_frameBufferWidth == 0 || m_handle->m_frameBufferHeight == 0)
		{
			return false;
		}

		eastl::vector<VKFence*>& images_fence = m_handle->m_imagesFence;


		for (uint32_t i = 0; i < m_handle->m_swapChain.image_count; i++)
		{
			images_fence[i] = nullptr;
		}

		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_handle->m_physicalDevice, m_instance->m_surface, &m_handle->m_swapchainInfo.surface_capabilities);

		vk::_RecreateSwapchain(m_instance, m_handle, &m_handle->m_swapChain, m_handle->m_frameBufferWidth, m_handle->m_frameBufferHeight);

		m_handle->m_renderPass.render_area.z = m_handle->m_frameBufferWidth;
		m_handle->m_renderPass.render_area.w = m_handle->m_frameBufferHeight;

		for (VKFrameBuffer& i : m_handle->m_swapChain.m_frameBuffers)
		{
			vk::_DestoryFrameBuffer(m_instance, m_handle, &i);
		}

		vk::_RegenerateFrameBuffers(m_instance, m_handle, &m_handle->m_swapChain, &m_handle->m_renderPass);

		m_handle->m_recreatingSwapcahin = false;

		return true;
	}

}