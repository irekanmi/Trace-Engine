#include "pch.h"

#include "Renderutils.h"
#include "core/platform/Vulkan/VulkanRenderFunc.h"

#define RENDER_FUNC_IS_VALID(function)                           \
	if(!function)                                                \
	{                                                            \
		result = false;                                          \
		TRC_ERROR(                                               \
	"{} is not available, please check for any errors"           \
		, #function);                                           \
		return result;                                           \
	}
	

namespace trace {



	void AutoFillPipelineDesc(
		PipelineStateDesc& desc,
		bool input_layout, 
		bool raterizer_state, 
		bool depth_sten_state, 
		bool color_blend_state, 
		bool view_port, 
		bool scissor,
		bool render_pass,
		bool primitive_topology
	)
	{

		if (input_layout)
		{
			InputLayout _layout = Vertex::get_input_layout();
			desc.input_layout = _layout;
		}

		if (raterizer_state)
		{
			RaterizerState rs = {};
			rs.cull_mode = CullMode::BACK;
			rs.fill_mode = FillMode::SOLID;

			desc.rateriser_state = rs;
		}

		if (depth_sten_state)
		{
			DepthStencilState dss = {};
			dss.depth_test_enable = true;
			dss.minDepth = 0.0f;
			dss.maxDepth = 1.0f;
			dss.stencil_test_enable = false;
			
			desc.depth_sten_state = dss;
		}

		if (color_blend_state)
		{
			ColorBlendState blds = {};
			blds.alpha_to_blend_coverage = true;

			desc.blend_state = blds;
		}

		if (view_port)
		{
			Viewport vp = {};
			vp.x = vp.y = 0.0f;
			vp.width = 800.0f;
			vp.height = 600.0f;
			vp.minDepth = 0.0f;
			vp.maxDepth = 1.0f;
			
			desc.view_port = vp;
		}

		if (scissor)
		{
			Rect2D rect;
			rect.top = rect.left = 0;
			rect.right = 800;
			rect.bottom = 600;

		}
		
		if (render_pass)
		{
			desc._renderPass = RENDERPASS::MAIN_PASS;
			desc.subpass_index = 0;
		}
		if (primitive_topology)
		{
			desc.topology = PrimitiveTopology::TRIANGLE_LIST;
		}
	}

	bool operator==(ShaderResourceBinding lhs, ShaderResourceBinding rhs)
	{
		bool result = (lhs.count == rhs.count) &&
			(lhs.resource_stage == rhs.resource_stage) &&
			(lhs.resource_type == rhs.resource_type) &&
			(lhs.slot == rhs.slot);
		return result;
	}

	bool RenderFuncLoader::LoadVulkanRenderFunctions()
	{
		RenderFunc::_createContext = vk::__CreateContext;
		RenderFunc::_destroyContext = vk::__DestroyContext;
		RenderFunc::_createDevice = vk::__CreateDevice;
		RenderFunc::_destroyDevice = vk::__DestroyDevice;
		return false;
	}

	__CreateContext RenderFunc::_createContext = nullptr;
	__DestroyContext RenderFunc::_destroyContext = nullptr;

	__CreateDevice RenderFunc::_createDevice = nullptr;
	__DestroyDevice RenderFunc::_destroyDevice = nullptr;
	__DrawElements RenderFunc::_drawElements = nullptr;
	__DrawInstanceElements RenderFunc::_drawInstancedElements = nullptr;
	__DrawIndexed_ RenderFunc::_drawIndexed_ = nullptr;
	__DrawInstanceIndexed RenderFunc::_drawInstancedIndexed = nullptr;
	__BindViewport RenderFunc::_bindViewport = nullptr;
	__BindRect RenderFunc::_bindRect = nullptr;
	__BindPipeline RenderFunc::_bindPipeline = nullptr;
	__BindVertexBuffer RenderFunc::_bindVertexBuffer = nullptr;
	__BindIndexBuffer RenderFunc::_bindIndexBuffer = nullptr;
	__Draw RenderFunc::_draw = nullptr;
	__DrawIndexed RenderFunc::_drawIndexed = nullptr;
	__BeginRenderPass RenderFunc::_beginRenderPass = nullptr;
	__NextSubpass RenderFunc::_nextSubpass = nullptr;
	__EndRenderPass RenderFunc::_endRenderPass = nullptr;
	__BeginFrame RenderFunc::_beginFrame;
	__EndFrame RenderFunc::_endFrame = nullptr;

	__CreateBuffer RenderFunc::_createBuffer = nullptr;
	__DestroyBuffer RenderFunc::_destroyBuffer = nullptr;
	__SetBufferData RenderFunc::_setBufferData = nullptr;

	__CreateFramebuffer RenderFunc::_createFramebuffer = nullptr;
	__DestroyFramebuffer RenderFunc::_destroyFramebuffer = nullptr;

	__ValidateHandle RenderFunc::_validateHandle = nullptr;

	bool RenderFunc::CreateContext(GContext* context)
	{
		bool result = true;

		RENDER_FUNC_IS_VALID(_createContext);
		result = _createContext(context);

		return result;
	}

	bool RenderFunc::DestroyContext(GContext* context)
	{
		bool result = true;

		RENDER_FUNC_IS_VALID(_destroyContext);
		result = _destroyContext(context);

		return result;
	}

	bool RenderFunc::CreateDevice(GDevice* device)
	{
		bool result = true;

		RENDER_FUNC_IS_VALID(_createDevice);
		result = _createDevice(device);

		return result;
	}

	bool RenderFunc::DestroyDevice(GDevice* device)
	{
		bool result = true;

		RENDER_FUNC_IS_VALID(_destroyDevice);
		result = _destroyDevice(device);

		return result;
	}

	bool RenderFunc::DrawElements(GDevice* device, GBuffer* vertex_buffer)
	{
		bool result = true;

		RENDER_FUNC_IS_VALID(_drawElements);
		result = _drawElements(device,vertex_buffer);

		return result;
	}

	bool RenderFunc::DrawInstanceElements(GDevice* device, GBuffer* vertex_buffer, uint32_t instances)
	{
		bool result = true;

		RENDER_FUNC_IS_VALID(_drawInstancedElements);
		result = _drawInstancedElements(device, vertex_buffer, instances);

		return result;
	}

	bool RenderFunc::DrawIndexed_(GDevice* device, GBuffer* index_buffer)
	{
		bool result = true;

		RENDER_FUNC_IS_VALID(_drawIndexed_);
		result = _drawIndexed_(device, index_buffer);

		return result;
	}

	bool RenderFunc::DrawInstanceIndexed(GDevice* device, GBuffer* index_buffer, uint32_t instances)
	{
		bool result = true;

		RENDER_FUNC_IS_VALID(_drawInstancedIndexed);
		result = _drawInstancedIndexed(device, index_buffer, instances);

		return result;
	}

	bool RenderFunc::BindViewport(GDevice* device, Viewport view_port)
	{
		bool result = true;

		RENDER_FUNC_IS_VALID(_bindViewport);
		result = _bindViewport(device, view_port);

		return result;
	}

	bool RenderFunc::CreateBuffer(GBuffer* buffer, BufferInfo buffer_info)
	{
		bool result = true;

		RENDER_FUNC_IS_VALID(_createBuffer);
		result = _createBuffer(buffer, buffer_info);

		return result;
	}

	bool RenderFunc::DestroyBuffer(GBuffer* buffer)
	{
		bool result = true;

		RENDER_FUNC_IS_VALID(_destroyBuffer);
		result = _destroyBuffer(buffer);

		return result;
	}

	bool RenderFunc::SetBufferData(GBuffer* buffer, void* data, uint32_t size)
	{
		bool result = true;

		RENDER_FUNC_IS_VALID(_setBufferData);
		result = _setBufferData(buffer, data, size);

		return result;
	}

	bool RenderFunc::CreateFramebuffer(GFramebuffer* framebuffer, uint32_t num_attachment, GTexture** attachments, GRenderPass* render_pass, uint32_t width, uint32_t height, uint32_t swapchain_image_index, GSwapchain* swapchain)
	{
		bool result = true;

		RENDER_FUNC_IS_VALID(_createFramebuffer);
		result = _createFramebuffer(framebuffer, num_attachment, attachments, render_pass, width, height, swapchain_image_index, swapchain);

		return result;
	}

	bool RenderFunc::DestroyFramebuffer(GFramebuffer* framebuffer)
	{
		bool result = true;

		RENDER_FUNC_IS_VALID(_destroyFramebuffer);
		result = _destroyFramebuffer(framebuffer);

		return result;
	}

	bool RenderFunc::ValidateHandle(GHandle* handle)
	{
		bool result = true;

		RENDER_FUNC_IS_VALID(_validateHandle);
		result = _validateHandle(handle);

		return result;
	}

	bool RenderFunc::BindRect(GDevice* device, Rect2D rect)
	{
		bool result = true;

		RENDER_FUNC_IS_VALID(_bindRect);
		result = _bindRect(device, rect);

		return result;
	}

	bool RenderFunc::BindPipeline(GDevice* device, GPipeline* pipeline)
	{
		bool result = true;

		RENDER_FUNC_IS_VALID(_bindPipeline);
		result = _bindPipeline(device, pipeline);

		return result;
	}

	bool RenderFunc::BindVertexBuffer(GDevice* device, GBuffer* buffer)
	{
		bool result = true;

		RENDER_FUNC_IS_VALID(_bindVertexBuffer);
		result = _bindVertexBuffer(device, buffer);

		return result;
	}

	bool RenderFunc::BindIndexBuffer(GDevice* device, GBuffer* buffer)
	{
		bool result = true;

		RENDER_FUNC_IS_VALID(_bindIndexBuffer);
		result = _bindIndexBuffer(device, buffer);

		return result;
	}

	bool RenderFunc::Draw(GDevice* device, uint32_t start_vertex, uint32_t count)
	{
		bool result = true;

		RENDER_FUNC_IS_VALID(_draw);
		result = _draw(device, start_vertex, count);

		return result;
	}

	bool RenderFunc::DrawIndexed(GDevice* device, uint32_t first_index, uint32_t count)
	{
		bool result = true;

		RENDER_FUNC_IS_VALID(_drawIndexed);
		result = _drawIndexed(device, first_index, count);

		return result;
	}

	bool RenderFunc::BeginRenderPass(GDevice* device, GRenderPass* render_pass, GFramebuffer* frame_buffer)
	{
		bool result = true;

		RENDER_FUNC_IS_VALID(_beginRenderPass);
		result = _beginRenderPass(device, render_pass, frame_buffer);

		return result;
	}

	bool RenderFunc::NextSubpass(GDevice* device, GRenderPass* render_pass)
	{
		bool result = true;

		RENDER_FUNC_IS_VALID(_nextSubpass);
		result = _nextSubpass(device, render_pass);

		return result;
	}

	bool RenderFunc::EndRenderPass(GDevice* device, GRenderPass* render_pass)
	{
		bool result = true;

		RENDER_FUNC_IS_VALID(_endRenderPass);
		result = _endRenderPass(device, render_pass);

		return result;
	}

	bool RenderFunc::BeginFrame(GDevice* device, GSwapchain* swapchain)
	{
		bool result = true;

		RENDER_FUNC_IS_VALID(_beginFrame);
		result = _beginFrame(device, swapchain);

		return result;
	}

	bool RenderFunc::EndFrame(GDevice* device)
	{
		bool result = true;

		RENDER_FUNC_IS_VALID(_endFrame);
		result = _endFrame(device);

		return result;
	}

}