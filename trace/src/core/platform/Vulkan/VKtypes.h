#pragma once

#include "core/Enums.h"
#include "core/io/Logging.h"
#include "vulkan/vulkan.h"
#include "EASTL/vector.h"
#include "glm/glm.hpp"
#include <unordered_map>



namespace trace {

#ifdef TRC_DEBUG_BUILD
#define VK_ASSERT(func) TRC_ASSERT(func == VK_SUCCESS, #func);
#else
#define VK_ASSERT(func) func
#endif

#define VK_NO_FLAGS 0
#define VK_MAX_NUM_FRAMES 3

	class RenderGraphPass;
	class RenderGraph;
	struct RenderGraphResource;
	struct SwapchainInfo
	{
		VkSurfaceCapabilitiesKHR surface_capabilities;
		uint32_t format_count;
		std::vector<VkSurfaceFormatKHR> formats;
		uint32_t present_count;
		std::vector<VkPresentModeKHR> present_modes;
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

		glm::vec4* clear_color;
		glm::vec4* render_area;

		float* depth_value;
		uint32_t* stencil_value;

		RenderPassState m_state;

		void* m_instance = nullptr;
		void* m_device = nullptr;
	};
	
	struct VKShader
	{
		VkShaderModule m_module = VK_NULL_HANDLE;
		VkPipelineShaderStageCreateInfo create_info = {};
		// TODO: Determine if "m_code" will be vulkan specific member variakble
		std::vector<uint32_t> m_code = {};

		void* m_instance = nullptr;
		void* m_device = nullptr;
	};

	struct VKImage
	{
		VkImage m_handle = VK_NULL_HANDLE;
		VkImageView m_view = VK_NULL_HANDLE;
		VkDeviceMemory m_mem = VK_NULL_HANDLE;
		uint32_t m_width = 0;
		uint32_t m_height = 0;

		VkSampler m_sampler;

		void* m_instance = nullptr;
		void* m_device = nullptr;
	};

	

	struct VKFence
	{
		VkFence m_handle = VK_NULL_HANDLE;
		bool m_isSignaled = false;
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

	struct VKSwapChain
	{
		VkSwapchainKHR m_handle;
		VkSurfaceFormatKHR m_format;
		VKImage m_depthimage;
		VkFormat m_depthFormat;

		std::vector<VkImage> m_images;
		std::vector<VkImageView> m_imageViews;

		uint32_t image_count;
		uint32_t m_currentImageIndex;
		uint32_t m_width;
		uint32_t m_height;
		bool m_recreating;

		//HACK: Find another way to solve problem of get a swapchain image
		VKImage tempColor;


		void* m_instance = nullptr;
		void* m_device = nullptr;

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

		std::vector<VkSemaphore> m_imageAvailableSemaphores;
		std::vector<VkSemaphore> m_queueCompleteSemaphores;

		uint32_t m_numInFlightFence;
		std::array<VKFence, VK_MAX_NUM_FRAMES> m_inFlightFence;

		std::array<VKFence*, VK_MAX_NUM_FRAMES> m_imagesFence;

		std::array<VkDescriptorPool, VK_MAX_NUM_FRAMES> m_frameDescriptorPool;

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

	struct VKBuffer
	{
		VkBuffer m_handle = VK_NULL_HANDLE;
		VkDeviceMemory m_memory = VK_NULL_HANDLE;
		VKDeviceHandle* m_device = nullptr;
		VKHandle* m_instance = nullptr;
	};

	struct VKPipeline
	{
		VkPipeline m_handle = VK_NULL_HANDLE;
		VkPipelineLayout m_layout = VK_NULL_HANDLE;

		VKBuffer Scene_buffers = {};
		VkDescriptorSet Scene_sets[3] = {};
		VkDescriptorSetLayout Scene_layout = VK_NULL_HANDLE;
		VkDescriptorPool Scene_pool = VK_NULL_HANDLE;


		VkDescriptorSet Instance_sets[3] = {};
		VkDescriptorSetLayout Instance_layout = VK_NULL_HANDLE;
		VkDescriptorPool Instance_pool = VK_NULL_HANDLE;

		VkDescriptorSet Local_sets[3] = {};
		VkDescriptorSetLayout Local_layout = VK_NULL_HANDLE;
		VkDescriptorPool Local_pool = VK_NULL_HANDLE;

		VKHandle* m_instance = nullptr;
		VKDeviceHandle* m_device = nullptr;
		char* cache_data = nullptr;
		void* last_tex_update[3] = {};

	};

	struct VKFrameBuffer
	{
		uint32_t m_width = 0;
		uint32_t m_height = 0;
		uint32_t m_attachmentCount = 0;
		VkFramebuffer m_handle = VK_NULL_HANDLE;
		VKRenderPass m_renderPass = {};
		eastl::vector<VkImageView> m_attachments = {};
		VKHandle* m_instance = nullptr;
		VKDeviceHandle* m_device = nullptr;

	};

	//Temp : fix the frame buffer structure, to prevent creating multiple strucuters the to hold the same type
	struct Framebuffer_VK
	{
		std::vector<VKFrameBuffer> m_handle;
		VKHandle* m_instance = nullptr;
		VKDeviceHandle* m_device = nullptr;
	};

	struct VKMaterialData
	{
		VkDescriptorSet m_sets[3] = {};

		void* m_instance;
		void* m_device;
	};

	struct VKDrawData
	{
		VkDescriptorSet sets[3];
		VkWriteDescriptorSet writes;
	};

	struct VKEvntPair
	{
		RenderGraphPass* pass = nullptr;
		RenderGraphResource* resource = nullptr;
		VkEvent evnt = VK_NULL_HANDLE;
	};

	struct VKRenderGraph
	{
		void* m_device = nullptr;
		void* m_instance = nullptr;
		std::vector<VKEvntPair> events;
	};

	struct VKRenderGraphPass
	{
		std::vector<VkEvent> wait_events;
		std::vector<VkEvent> signal_events;
		VKRenderPass physical_pass;
		std::vector<VkImageMemoryBarrier> invalidate_image_barriers[VK_MAX_NUM_FRAMES];
		std::vector<VkImageMemoryBarrier> flush_image_barriers[VK_MAX_NUM_FRAMES];
		VkFramebuffer frame_buffers[VK_MAX_NUM_FRAMES];
	};

	struct VKRenderGraphResource
	{
		struct {
			VKImage texture;
			VKBuffer buffer;
		} resource[VK_MAX_NUM_FRAMES];
	};





}
