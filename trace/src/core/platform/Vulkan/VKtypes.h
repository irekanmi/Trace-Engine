#pragma once

#include "core/Enums.h"
#include "core/io/Logging.h"
#include "vulkan/vulkan.h"
#include "EASTL/vector.h"
#include "glm/glm.hpp"


namespace trace {

#ifdef TRC_DEBUG_BUILD
#define VK_ASSERT(func) TRC_ASSERT(func == VK_SUCCESS, #func);
#else
#define VK_ASSERT(func) func
#endif

#define VK_NO_FLAGS 0

	struct Setlayout
	{
		VkDescriptorSetLayout m_layout;
		bool m_vaild;
	};

	struct SwapchainInfo
	{
		VkSurfaceCapabilitiesKHR surface_capabilities;
		uint32_t format_count;
		eastl::vector<VkSurfaceFormatKHR> formats;
		uint32_t present_count;
		eastl::vector<VkPresentModeKHR> present_modes;
	};

	struct QueueFamilyIndices
	{
		uint32_t graphics_queue = -1;
		uint32_t present_queue = -1;
		uint32_t transfer_queue = -1;
		uint32_t compute_queue = -1;
	};

	enum CommandBufferUsage
	{
		NO_USE,
		SINGLE_USE,
		RENDER_PASS_CONTINUE,
		SIMULTANEOUS_USE
	};

	enum CommandBufferState
	{
		COMMAND_READY,
		COMMAND_RECORDING,
		COMMAND_IN_RENDER_PASS,
		COMMAND_RECORDING_ENDED,
		COMMAND_SUBMMITED,
		COMMAND_NOT_ALLOCATED
	};

	struct VKCommmandBuffer
	{
		VkCommandBuffer m_handle;
		CommandBufferState m_state;
//		CommandBufferUsage m_usage;
	};

	enum RenderPassState
	{
		READY,
		RECORDING,
		IN_RENDER_PASS,
		RECORDING_ENDED,
		SUBMMITED,
		NOT_ALLOCATED
	};

	struct VKRenderPass
	{
		VkRenderPass m_handle;

		glm::vec4 clear_color;
		glm::vec4 render_area;

		float depth_value;
		uint32_t stencil_value;

		RenderPassState m_state;
	};

	struct VKBuffer
	{
		VkBuffer m_handle = VK_NULL_HANDLE;
		VkDeviceMemory m_memory = VK_NULL_HANDLE;
	};

	struct VKPipeline
	{
		VkPipeline m_handle = VK_NULL_HANDLE;
		VkPipelineLayout m_layout = VK_NULL_HANDLE;

		VkDescriptorSetLayoutBinding Scene_bindings[16] = {};
		VKBuffer Scene_buffers[16] = {};
		VkDescriptorSet Scene_sets[3] = {};
		VkDescriptorSetLayout Scene_layout = VK_NULL_HANDLE;
		VkDescriptorPool Scene_pool = VK_NULL_HANDLE;

	};

	struct VKShader
	{
		VkShaderModule m_module = VK_NULL_HANDLE;
		VkPipelineShaderStageCreateInfo create_info = {};
		// TODO 
		std::vector<uint32_t> m_code = {};
	};

	struct VKImage
	{
		VkImage m_handle = VK_NULL_HANDLE;
		VkImageView m_view = VK_NULL_HANDLE;
		VkDeviceMemory m_mem = VK_NULL_HANDLE;
		uint32_t m_width = 0;
		uint32_t m_height = 0;
	};

	struct VKFrameBuffer
	{
		uint32_t m_width = 0;
		uint32_t m_height = 0;
		uint32_t m_attachmentCount = 0;
		VkFramebuffer m_handle = VK_NULL_HANDLE;
		VKRenderPass m_renderPass = {};
		eastl::vector<VkImageView> m_attachments = {};
	};

	struct VKFence
	{
		VkFence m_handle = VK_NULL_HANDLE;
		bool m_isSignaled = false;
	};

	struct VKSwapChain
	{
		VkSwapchainKHR m_handle;
		VkSurfaceFormatKHR m_format;
		VKImage m_depthimage;

		eastl::vector<VkImage> m_images;
		eastl::vector<VkImageView> m_imageViews;

		uint32_t image_count;

		eastl::vector<VKFrameBuffer> m_frameBuffers;

	};

	struct VKHandle
	{
		VkInstance m_instance;
		VkAllocationCallbacks* m_alloc_callback = nullptr;
		VkSurfaceKHR m_surface;
#ifdef TRC_DEBUG_BUILD
		VkDebugUtilsMessengerEXT m_debugutils;
#endif
	};

	struct VKDeviceHandle
	{
		VkPhysicalDevice m_physicalDevice;
		VkDevice m_device;

		VkQueue m_graphicsQueue;
		VkQueue m_presentQueue;
		VkQueue m_transferQueue;

		VkFormat m_depthFormat;

		eastl::vector<VKCommmandBuffer> m_graphicsCommandBuffers;
		VkCommandPool m_graphicsCommandPool;

		bool m_recreatingSwapcahin = false;

		VKSwapChain m_swapChain;
		VKRenderPass m_renderPass;


		eastl::vector<VkSemaphore> m_imageAvailableSemaphores;
		eastl::vector<VkSemaphore> m_queueCompleteSemaphores;

		uint32_t m_numInFlightFence;
		eastl::vector<VKFence> m_inFlightFence;

		eastl::vector<VKFence*> m_imagesFence;

		uint32_t m_frameBufferWidth;
		uint32_t m_frameBufferHeight;

		uint32_t frames_in_flight;
		uint32_t m_currentFrame = 0;
		uint32_t m_imageIndex;

		QueueFamilyIndices m_queues;
		VkPhysicalDeviceProperties m_properties;
		VkPhysicalDeviceFeatures m_features;
		SwapchainInfo m_swapchainInfo;
	};



}
