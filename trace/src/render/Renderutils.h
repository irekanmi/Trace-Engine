#pragma once

#include "Graphics.h"

namespace trace {

	class GHandle;
	class GDevice;
	class GContext;
	class GBuffer;
	class GSwapchain;
	class GPipeline;
	class GFramebuffer;
	struct BufferInfo;

	void AutoFillPipelineDesc(PipelineStateDesc& desc, bool input_layout = true, bool raterizer_state = true, bool depth_sten_state = true, bool color_blend_state = true, bool view_port = true, bool scissor = true, bool render_pass = true, bool primitive_topology = true);
	bool operator ==(ShaderResourceBinding lhs, ShaderResourceBinding rhs);


	typedef bool (*__CreateContext)(GContext*);
	typedef bool (*__DestroyContext)(GContext*);


	// Device Fuctions --------------
	typedef bool (*__CreateDevice)(GDevice*);
	typedef bool (*__DestroyDevice)(GDevice*);
	typedef bool (*__DrawElements)(GDevice*,GBuffer*);
	typedef bool (*__DrawInstanceElements)(GDevice*,GBuffer*, uint32_t);
	typedef bool (*__DrawIndexed_)(GDevice*,GBuffer*);
	typedef bool (*__DrawInstanceIndexed)(GDevice*,GBuffer*, uint32_t);
	typedef bool (*__BindViewport)(GDevice*,Viewport);
	typedef bool (*__BindRect)(GDevice*,Rect2D);
	typedef bool (*__BindPipeline)(GDevice*,GPipeline*);
	typedef bool (*__BindVertexBuffer)(GDevice*,GBuffer*);
	typedef bool (*__BindIndexBuffer)(GDevice*,GBuffer*);
	typedef bool (*__Draw)(GDevice*,uint32_t, uint32_t);
	typedef bool (*__DrawIndexed)(GDevice*,uint32_t, uint32_t);
	typedef bool (*__BeginRenderPass)(GDevice*,GRenderPass*, GFramebuffer*);
	typedef bool (*__NextSubpass)(GDevice*,GRenderPass*);
	typedef bool (*__EndRenderPass)(GDevice*,GRenderPass*);
	typedef bool (*__BeginFrame)(GDevice*,GSwapchain*);
	typedef bool (*__EndFrame)(GDevice*);
	// -----------------------

	// Buffers ----------------
	typedef bool (*__CreateBuffer)(GBuffer*, BufferInfo);
	typedef bool (*__DestroyBuffer)(GBuffer*);
	typedef bool (*__SetBufferData)(GBuffer*, void*, uint32_t);
	// -----------------

	// Framebuffers ----------------
	typedef bool (*__CreateFramebuffer)(GFramebuffer*, uint32_t, GTexture**, GRenderPass*, uint32_t, uint32_t, uint32_t, GSwapchain*);
	typedef bool (*__DestroyFramebuffer)(GFramebuffer*);
	// ----------------------------

	typedef bool (*__ValidateHandle)(GHandle*);

	class TRACE_API RenderFuncLoader
	{
	public:
		static bool LoadVulkanRenderFunctions();
	private:
	protected:
	};

	class TRACE_API RenderFunc
	{

	public:
		static bool CreateContext(GContext* context);
		static bool DestroyContext(GContext* context);

		static bool CreateDevice(GDevice* device);
		static bool DestroyDevice(GDevice* device);
		static bool DrawElements(GDevice* device, GBuffer* vertex_buffer);
		static bool DrawInstanceElements(GDevice* device, GBuffer* vertex_buffer, uint32_t instances);
		static bool DrawIndexed_(GDevice* device, GBuffer* index_buffer);
		static bool DrawInstanceIndexed(GDevice* device, GBuffer* index_buffer, uint32_t instances);
		static bool BindViewport(GDevice* device, Viewport view_port);
		static bool BindRect(GDevice* device, Rect2D rect);
		static bool BindPipeline(GDevice* device, GPipeline* pipeline);
		static bool BindVertexBuffer(GDevice* device, GBuffer* buffer);
		static bool BindIndexBuffer(GDevice* device, GBuffer* buffer);
		static bool Draw(GDevice* device, uint32_t start_vertex, uint32_t count);
		static bool DrawIndexed(GDevice* device, uint32_t first_index, uint32_t count);
		static bool BeginRenderPass(GDevice* device, GRenderPass* render_pass, GFramebuffer* frame_buffer);
		static bool NextSubpass(GDevice* device, GRenderPass* render_pass);
		static bool EndRenderPass(GDevice* device, GRenderPass* render_pass);
		static bool BeginFrame(GDevice* device, GSwapchain* swapchain);
		static bool EndFrame(GDevice* device);


		static bool CreateBuffer(GBuffer* buffer, BufferInfo buffer_info);
		static bool DestroyBuffer(GBuffer* buffer);
		static bool SetBufferData(GBuffer* buffer, void* data, uint32_t size);


		static bool CreateFramebuffer(GFramebuffer* framebuffer, uint32_t num_attachment, GTexture** attachments, GRenderPass* render_pass, uint32_t width, uint32_t height, uint32_t swapchain_image_index, GSwapchain* swapchain);
		static bool DestroyFramebuffer(GFramebuffer* framebuffer);

		static bool ValidateHandle(GHandle* handle);

	private:
		static __CreateContext _createContext;
		static __DestroyContext _destroyContext;

		static __CreateDevice _createDevice;
		static __DestroyDevice _destroyDevice;
		static __DrawElements _drawElements;
		static __DrawInstanceElements _drawInstancedElements;
		static __DrawIndexed_ _drawIndexed_;
		static __DrawInstanceIndexed _drawInstancedIndexed;
		static __BindViewport _bindViewport;
		static __BindRect _bindRect;
		static __BindPipeline _bindPipeline;
		static __BindVertexBuffer _bindVertexBuffer;
		static __BindIndexBuffer _bindIndexBuffer;
		static __Draw _draw;
		static __DrawIndexed _drawIndexed;
		static __BeginRenderPass _beginRenderPass;
		static __NextSubpass _nextSubpass;
		static __EndRenderPass _endRenderPass;
		static __BeginFrame _beginFrame;
		static __EndFrame _endFrame;

		static __CreateBuffer _createBuffer;
		static __DestroyBuffer _destroyBuffer;
		static __SetBufferData _setBufferData;

		static __CreateFramebuffer _createFramebuffer;
		static __DestroyFramebuffer _destroyFramebuffer;

		static __ValidateHandle _validateHandle;

	protected:
		friend RenderFuncLoader;
	};

}
