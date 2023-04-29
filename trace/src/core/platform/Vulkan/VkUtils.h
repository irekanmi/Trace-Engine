#pragma once
#include "vulkan/vulkan.h"
#include "VKtypes.h"
#include "EASTL/vector.h"
#include "core/Window.h"
#include "render/Graphics.h"
#include "EASTL/map.h"
#include <map>
#include "render/GTexture.h"


namespace vk {

	// Vulkan Instance
	VkResult _CreateInstance(trace::VKHandle* instance);
	void _DestroyInstance(trace::VKHandle* instance);

	bool _FindLayer(const char* layer, eastl::vector<VkLayerProperties>& layers);
	bool _FindExtension(const char* extension, eastl::vector<VkExtensionProperties>& extensions);

	// Validation layers
	void EnableValidationlayers(trace::VKHandle* instance);
	void DisableValidationlayers(trace::VKHandle* instance);

	// Logical device
	VkResult _CreateDevice(trace::VKDeviceHandle* device, trace::VKHandle* instance);
	void _DestoryDevice(trace::VKDeviceHandle* device, trace::VKHandle* instance);

	// Surfaces
	VkResult _CreateSurface(trace::VKHandle* instance);
	void _DestorySurface(trace::VKHandle* instance);

	VkPhysicalDevice GetPhysicalDevice(trace::VKHandle* instance);
	void RatePhysicalDevice(VkPhysicalDevice phy_device, uint32_t& score);

	// Swapchain
	VkResult _CreateSwapchain(trace::VKHandle* instance, trace::VKDeviceHandle* device, trace::VKSwapChain* swapchain, uint32_t width, uint32_t height);
	void _DestroySwapchain(trace::VKHandle* instance, trace::VKDeviceHandle* device, trace::VKSwapChain* swapchain);
	VkResult _RecreateSwapchain(trace::VKHandle* instance, trace::VKDeviceHandle* device, trace::VKSwapChain* swapchain, uint32_t width, uint32_t height);
	bool _AcquireSwapchainImage(trace::VKHandle* instance, trace::VKDeviceHandle* device, trace::VKSwapChain* swapchain, VkSemaphore image_acquired_sem, trace::VKFence* fence, uint32_t* image_index, uint32_t timeout_ns);
	void _PresentSwapchainImage(trace::VKHandle* instance, trace::VKDeviceHandle* device, trace::VKSwapChain* swapchain, VkQueue graphics_queue, VkQueue present_queue, VkSemaphore render_complete, uint32_t* present_image_index);

	// Images
	VkResult _CreateImage(trace::VKHandle* instance,trace::VKDeviceHandle* device, trace::VKImage* out_image, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags memory_prop , VkImageAspectFlags aspect_flags, VkImageCreateFlags flags,  VkImageType image_type, uint32_t width, uint32_t height, uint32_t layers, uint32_t mip_levels);
	VkResult _CreateImageView(trace::VKHandle* instance, trace::VKDeviceHandle* device, VkImageView* out_image_view, VkFormat format, VkImageAspectFlags aspect_flags, VkImage* image);
	void _DestroyImageView(trace::VKHandle* instance, trace::VKDeviceHandle* device, VkImageView* image_view);
	void _DestroyImage(trace::VKHandle* instance, trace::VKDeviceHandle* device, trace::VKImage* image);
	void _CopyBufferToImage(trace::VKHandle* instance, trace::VKDeviceHandle* device, trace::VKCommmandBuffer* command_buffer, trace::VKImage* image, trace::VKBuffer* buffer);
	bool _TransitionImageLayout(trace::VKHandle* instance, trace::VKDeviceHandle* device, trace::VKCommmandBuffer* command_buffer, trace::VKImage* image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout, VkImageSubresourceRange sub_resource_range);
	bool _GenerateMipLevels(trace::VKHandle* instance, trace::VKDeviceHandle* device, trace::VKImage* image, VkFormat format, uint32_t width, uint32_t height, uint32_t mip_levels, uint32_t layer_index);

	// Samplers
	VkResult _CreateSampler(trace::VKHandle* instance, trace::VKDeviceHandle* device, trace::TextureDesc& desc, VkSampler& sampler, float max_lod);
	void _DestroySampler(trace::VKHandle* instance, trace::VKDeviceHandle* device, VkSampler& sampler);

	uint32_t FindMemoryIndex(trace::VKDeviceHandle* device, uint32_t memory_type, VkMemoryPropertyFlags mem_prop);

	// Command Buffer
	void _AllocateCommandBuffer(trace::VKDeviceHandle* device, VkCommandPool command_pool, trace::VKCommmandBuffer* out_command_buffer, bool is_primary = true);
	void _FreeCommandBuffer(trace::VKDeviceHandle* device, VkCommandPool command_pool, trace::VKCommmandBuffer* command_buffer);

	void _BeginCommandBuffer(trace::VKCommmandBuffer* command_buffer, trace::CommandBufferUsage usage);
	void _BeginCommandBufferSingleUse(trace::VKDeviceHandle* device, VkCommandPool command_pool, trace::VKCommmandBuffer* command_buffer);
	void _EndCommandBuffer(trace::VKCommmandBuffer* command_buffer);
	void _EndCommandBufferSingleUse(trace::VKDeviceHandle* device, VkCommandPool command_pool, VkQueue queue, trace::VKCommmandBuffer* command_buffer);
	void _CommandBufferSubmitted(trace::VKCommmandBuffer* command_buffer);
	void _CommandBuffer_Reset(trace::VKCommmandBuffer* command_buffer);

	void _CreateCommandBuffers(trace::VKHandle* instance, trace::VKDeviceHandle* device, VkCommandPool command_pool, eastl::vector<trace::VKCommmandBuffer> &buffers);

	// FrameBuffers
	VkResult _CreateFrameBuffer(trace::VKHandle* instance, trace::VKDeviceHandle* device, trace::VKFrameBuffer* frame_buffer,const eastl::vector<VkImageView>& attachments, trace::VKRenderPass* render_pass, uint32_t width, uint32_t height, uint32_t attachment_count);
	void _DestoryFrameBuffer(trace::VKHandle* instance, trace::VKDeviceHandle* device, trace::VKFrameBuffer* frame_buffer);

	// Fences
	void _CreateFence(trace::VKHandle* instance, trace::VKDeviceHandle* device, trace::VKFence* out_fence, bool signaled);
	bool _WaitFence(trace::VKHandle* instance, trace::VKDeviceHandle* device, trace::VKFence* out_fence, uint64_t timeout_ns);
	void _DestroyFence(trace::VKHandle* instance, trace::VKDeviceHandle* device, trace::VKFence* out_fence);
	void _FenceReset(trace::VKDeviceHandle* device, trace::VKFence* fence);
	

	// RenderPass
	// There is tendency that the render pass create and dectroy function would change
	VkResult _CreateRenderPass(trace::VKHandle* instance, trace::VKDeviceHandle* device, trace::VKRenderPass* render_pass, glm::vec4 clear_color, glm::vec4 render_area, float depth_value, uint32_t stencil_value, trace::VKSwapChain* swapchain);
	void _DestroyRenderPass(trace::VKHandle* instance, trace::VKDeviceHandle* device, trace::VKRenderPass* render_pass);
	void _BeginRenderPass(trace::VKHandle* instance, trace::VKDeviceHandle* device, trace::VKRenderPass* render_pass, trace::VKCommmandBuffer* command_buffer, VkFramebuffer framebuffer);
	void _EndRenderPass(trace::VKHandle* instance, trace::VKDeviceHandle* device, trace::VKCommmandBuffer* command_buffer);

	// Pipeline
	VkResult _CreatePipeline(trace::VKHandle* instance, trace::VKDeviceHandle* device, uint32_t view_port_count, VkViewport* view_ports, uint32_t scissor_count, VkRect2D* scissors, trace::PipelineStateDesc desc, trace::VKPipeline* out_pipeline, trace::VKRenderPass* render_pass, uint32_t subpass_index);
	void _DestroyPipeline(trace::VKHandle* instance, trace::VKDeviceHandle* device, trace::VKPipeline* pipeline);

	// Shaders
	VkResult _CreateShader(trace::VKHandle* instance, trace::VKDeviceHandle* device, trace::VKShader* out_shader, trace::ShaderStage stage, std::vector<uint32_t>& code);
	void _DestoryShader(trace::VKHandle* instance, trace::VKDeviceHandle* device, trace::VKShader* shader);

	// Buffers
	VkResult _CreateBuffer(trace::VKHandle* instance, trace::VKDeviceHandle* device, trace::VKBuffer* out_buffer, trace::BufferInfo buffer_info);
	void _DestoryBuffer(trace::VKHandle* instance, trace::VKDeviceHandle* device, trace::VKBuffer* buffer);
	void _BindBuffer(trace::VKHandle* instance, trace::VKDeviceHandle* device, trace::VKBuffer* buffer);
	void _BindBufferMem(trace::VKHandle* instance, trace::VKDeviceHandle* device, VkBuffer buffer, VkDeviceMemory device_mem, uint32_t offset);
	void _MapMemory(trace::VKDeviceHandle* device,void* data, VkDeviceMemory memory, uint32_t offset, uint32_t size, uint32_t flags = 0);
	void _UnMapMemory(trace::VKDeviceHandle* device, VkDeviceMemory memory);
	void _CopyBuffer(trace::VKHandle* instance, trace::VKDeviceHandle* device, trace::VKBuffer* src, trace::VKBuffer* dst, uint32_t size, uint32_t offset);


	// Utils
	bool _ResultIsSuccess(VkResult result);
	const char* _GetResultString(VkResult result);
	void parseInputLayout(trace::InputLayout& layout, VkVertexInputBindingDescription& binding, eastl::vector<VkVertexInputAttributeDescription>& attrs);
	void parseRasterizerState(trace::RaterizerState& raterizer, VkPipelineRasterizationStateCreateInfo& create_info);
	void parseMultiState(VkPipelineMultisampleStateCreateInfo& create_info);
	void parseDepthStenState(trace::DepthStencilState& state, VkPipelineDepthStencilStateCreateInfo& create_info);
	void parseColorBlendState(trace::ColorBlendState& state, VkPipelineColorBlendStateCreateInfo& create_info, VkPipelineColorBlendAttachmentState& colorBlendAttachment);
	void parsePipelineLayout(trace::VKHandle* instance, trace::VKDeviceHandle* device, trace::PipelineStateDesc& desc, VkPipelineLayoutCreateInfo& create_info, trace::VKPipeline* pipeline, VkDescriptorSetLayout* _layouts, std::vector<VkPushConstantRange>& ranges);

	VkFormat convertFmt(trace::Format format);
	VkImageViewType convertImageViewType(trace::ImageType image_type);
	VkImageType convertImageType(trace::ImageType image_type);
	VkPrimitiveTopology convertTopology(trace::PrimitiveTopology topology);
	VkPolygonMode convertPolygonMode(trace::FillMode fillmode);
	VkShaderStageFlagBits convertShaderStage(trace::ShaderStage stage);
	VkSamplerAddressMode convertAddressMode(trace::AddressMode address_mode);
	VkFilter convertFilter(trace::FilterMode filter);
	VkImageLayout convertTextureFmt(trace::TextureFormat fmt);
	VkAttachmentLoadOp convertAttachmentLoadOp(trace::AttachmentLoadOp op);
	VkAttachmentStoreOp convertAttachmentStoreOp(trace::AttachmentStoreOp op);
	VkCullModeFlagBits convertCullMode(trace::CullMode mode);
	std::vector<VkPushConstantRange> processShaderLocalData(std::vector<trace::ShaderResourceBinding>& bindings);
	void createBuffer(trace::VKHandle* instance, trace::VKDeviceHandle* device, VkDeviceSize size, VkBufferUsageFlags usage_flags, VkMemoryPropertyFlags prop_flags, VkBuffer& buffer, VkDeviceMemory& buffer_mem);

	//Processing
	bool operator ==(VkDescriptorSetLayoutBinding lhs, VkDescriptorSetLayoutBinding rhs);

}
