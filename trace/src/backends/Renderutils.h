#pragma once

#include "render/Graphics.h"
#include "resource/Ref.h"

#include <any>


namespace trace {

	class GHandle;
	class GDevice;
	class GContext;
	class GBuffer;
	class GSwapchain;
	class GPipeline;
	class GFramebuffer;
	class MaterialInstance;
	class RenderGraph;
	class RenderGraphPass;
	struct RenderGraphResource;
	struct Material;
	struct BufferInfo;
	struct RenderPassDescription;


	void AutoFillPipelineDesc(PipelineStateDesc& desc, bool input_layout = true, bool raterizer_state = true, bool depth_sten_state = true, bool color_blend_state = true, bool view_port = true, bool scissor = true, bool render_pass = true, bool primitive_topology = true);
	std::unordered_map<std::string, std::pair<std::any, uint32_t>> GetPipelineMaterialData(Ref<GPipeline> pipeline);


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
	typedef bool (*__BindLineWidth)(GDevice*,float);
	typedef bool (*__BindPipeline)(GDevice*,GPipeline*);
	typedef bool (*__BindVertexBuffer)(GDevice*,GBuffer*);
	typedef bool (*__BindVertexBufferBatch)(GDevice*,GBuffer*);
	typedef bool (*__BindIndexBuffer)(GDevice*,GBuffer*);
	typedef bool (*__BindIndexBufferBatch)(GDevice*,GBuffer*);
	typedef bool (*__Draw)(GDevice*,uint32_t, uint32_t);
	typedef bool (*__DrawIndexed)(GDevice*,uint32_t, uint32_t);
	typedef bool (*__BeginRenderPass)(GDevice*,GRenderPass*, GFramebuffer*);
	typedef bool (*__NextSubpass)(GDevice*,GRenderPass*);
	typedef bool (*__EndRenderPass)(GDevice*,GRenderPass*);
	typedef bool (*__BeginFrame)(GDevice*,GSwapchain*);
	typedef bool (*__EndFrame)(GDevice*);
	typedef bool (*__OnDrawStart)(GDevice*, GPipeline*);
	typedef bool (*__OnDrawEnd)(GDevice*, GPipeline*);
	// -----------------------

	// Buffers ----------------
	typedef bool (*__CreateBuffer)(GBuffer*, BufferInfo);
	typedef bool (*__DestroyBuffer)(GBuffer*);
	typedef bool (*__SetBufferData)(GBuffer*, void*, uint32_t);
	typedef bool (*__SetBufferDataOffset)(GBuffer*, void*, uint32_t, uint32_t);
	// -----------------

	// Framebuffers ----------------
	typedef bool (*__CreateFramebuffer)(GFramebuffer*, uint32_t, GTexture**, GRenderPass*, uint32_t, uint32_t, uint32_t, GSwapchain*);
	typedef bool (*__DestroyFramebuffer)(GFramebuffer*);
	// ----------------------------

	// Materials -------------------
	typedef bool (*__InitializeMaterial)(MaterialInstance*, Ref<GPipeline>);
	typedef bool (*__DestroyMaterial)(MaterialInstance*);
	typedef bool (*__PostInitializeMaterial)(MaterialInstance*, Ref<GPipeline>);
	typedef bool (*__ApplyMaterial)(MaterialInstance*);
	//-----------------------------

	// Pipeline --------------------
	typedef bool (*__CreatePipeline)(GPipeline*, PipelineStateDesc);
	typedef bool (*__DestroyPipeline)(GPipeline*);
	typedef bool (*__InitializePipeline)(GPipeline*);
	typedef bool (*__ShutDownPipeline)(GPipeline*);
	typedef bool (*__SetPipelineData)(GPipeline*, const std::string&, ShaderResourceStage, void*, uint32_t );
	typedef bool (*__SetPipelineTextureData)(GPipeline*, const std::string&, ShaderResourceStage, GTexture*, uint32_t);
	typedef bool (*__BindPipeline_)(GPipeline*);
	//------------------------------

	// RenderPass -------------------
	typedef bool (*__CreateRenderPass)(GRenderPass*, RenderPassDescription);
	typedef bool (*__DestroyRenderPass)(GRenderPass*);
	//-------------------------------

	// Shader ----------------------
	typedef bool (*__CreateShader)(GShader* , const std::string& , ShaderStage);
	typedef bool (*__CreateShader_)(GShader* , std::vector<uint32_t>& , ShaderStage);
	typedef bool (*__DestroyShader)(GShader*);
	//------------------------------

	// Swapchain ------------------
	typedef bool (*__CreateSwapchain)(GSwapchain* , GDevice* , GContext* );
	typedef bool (*__DestroySwapchain)(GSwapchain* );
	typedef bool (*__ResizeSwapchain)(GSwapchain* , uint32_t , uint32_t );
	typedef bool (*__PresentSwapchain)(GSwapchain* );
	typedef bool (*__GetSwapchainColorBuffer)(GSwapchain* , GTexture* );
	typedef bool (*__GetSwapchainDepthBuffer)(GSwapchain* , GTexture* );
	//-----------------------------

	// Textures -------------------
	typedef bool (*__CreateTexture)(GTexture* , TextureDesc );
	typedef bool (*__DestroyTexture)(GTexture* );
	typedef bool (*__GetTextureNativeHandle)(GTexture*, void*& );
	typedef bool (*__GetTextureData)(GTexture*, void*&);
	//----------------------------

	// RenderGraph ----------------
	typedef bool (*__BuildRenderGraph)(GDevice*, RenderGraph*);
	typedef bool (*__DestroyRenderGraph)(GDevice*, RenderGraph*);
	typedef bool (*__BeginRenderGraphPass)(RenderGraph*, RenderGraphPass* );
	typedef bool (*__EndRenderGraphPass)(RenderGraph*, RenderGraphPass* );
	typedef bool (*__BeginRenderGraph)(RenderGraph* );
	typedef bool (*__EndRenderGraph)(RenderGraph* );
	typedef bool (*__BindRenderGraphTexture)(RenderGraph*, GPipeline*, const std::string&, ShaderResourceStage, RenderGraphResource*, uint32_t);
	typedef bool (*__BindRenderGraphBuffer)(RenderGraph*, GPipeline*, const std::string&, ShaderResourceStage, RenderGraphResource*, uint32_t);
	//------------------------------

	// Batching -----------------------
	typedef bool (*__CreateBatchBuffer)(GBuffer*, BufferInfo);
	typedef bool (*__DestroyBatchBuffer)(GBuffer*);
	typedef bool(*__FlushBatchBuffer)(GBuffer*, void*, uint32_t);
	//---------------------------------------

	typedef bool (*__ValidateHandle)(GHandle*);

	class RenderFuncLoader
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
		static bool BindLineWidth(GDevice* device, float value);
		static bool BindPipeline(GDevice* device, GPipeline* pipeline);
		static bool BindVertexBuffer(GDevice* device, GBuffer* buffer);
		static bool BindVertexBufferBatch(GDevice* device, GBuffer* buffer);
		static bool BindIndexBuffer(GDevice* device, GBuffer* buffer);
		static bool BindIndexBufferBatch(GDevice* device, GBuffer* buffer);
		static bool Draw(GDevice* device, uint32_t start_vertex, uint32_t count);
		static bool DrawIndexed(GDevice* device, uint32_t first_index, uint32_t count);
		static bool BeginRenderPass(GDevice* device, GRenderPass* render_pass, GFramebuffer* frame_buffer);
		static bool NextSubpass(GDevice* device, GRenderPass* render_pass);
		static bool EndRenderPass(GDevice* device, GRenderPass* render_pass);
		static bool BeginFrame(GDevice* device, GSwapchain* swapchain);
		static bool EndFrame(GDevice* device);
		static bool OnDrawStart(GDevice* device, GPipeline* pipeline);
		static bool OnDrawEnd(GDevice* device, GPipeline* pipeline);


		static bool CreateBuffer(GBuffer* buffer, BufferInfo buffer_info);
		static bool DestroyBuffer(GBuffer* buffer);
		static bool SetBufferData(GBuffer* buffer, void* data, uint32_t size);
		static bool SetBufferDataOffset(GBuffer* buffer, void* data, uint32_t offset, uint32_t size);


		static bool CreateFramebuffer(GFramebuffer* framebuffer, uint32_t num_attachment, GTexture** attachments, GRenderPass* render_pass, uint32_t width, uint32_t height, uint32_t swapchain_image_index, GSwapchain* swapchain);
		static bool DestroyFramebuffer(GFramebuffer* framebuffer);

		static bool InitializeMaterial(MaterialInstance* mat_instance, Ref<GPipeline> pipeline);
		static bool DestroyMaterial(MaterialInstance* mat_instance);
		static bool PostInitializeMaterial(MaterialInstance* mat_instance, Ref<GPipeline> pipeline);
		static bool ApplyMaterial(MaterialInstance* mat_instance);

		static bool CreatePipeline(GPipeline* pipeline, PipelineStateDesc desc);
		static bool DestroyPipeline(GPipeline* pipeline);
		static bool InitializePipeline(GPipeline* pipeline);
		static bool ShutDownPipeline(GPipeline* pipeline);
		static bool SetPipelineData(GPipeline* pipeline, const std::string& resource_name, ShaderResourceStage resource_scope, void* data, uint32_t size);
		static bool SetPipelineTextureData(GPipeline* pipeline, const std::string& resource_name, ShaderResourceStage resource_scope, GTexture* texture, uint32_t index = 0);
		static bool BindPipeline_(GPipeline* pipeline);

		static bool CreateRenderPass(GRenderPass* render_pass, RenderPassDescription desc);
		static bool DestroyRenderPass(GRenderPass* render_pass);

		static bool CreateShader(GShader* shader, const std::string& src, ShaderStage stage);
		static bool CreateShader(GShader* shader, std::vector<uint32_t> & src, ShaderStage stage);
		static bool DestroyShader(GShader* shader);

		static bool CreateSwapchain(GSwapchain* swapchain, GDevice* device, GContext* context);
		static bool DestroySwapchain(GSwapchain* swapchain);
		static bool ResizeSwapchain(GSwapchain* swapchain, uint32_t width, uint32_t height);
		static bool PresentSwapchain(GSwapchain* swapchain);
		static bool GetSwapchainColorBuffer(GSwapchain* swapchain, GTexture* out_texture);
		static bool GetSwapchainDepthBuffer(GSwapchain* swapchain, GTexture* out_texture);

		static bool CreateTexture(GTexture* texture, TextureDesc desc);
		static bool DestroyTexture(GTexture* texture);
		static bool GetTextureNativeHandle(GTexture* texture, void*& out_handle);
		//NOTE: Ensure that the memory passed in is big enough to collect texture data
		static bool GetTextureData(GTexture* texture, void*& out_data);

		static bool BuildRenderGraph(GDevice* device, RenderGraph* render_graph);
		static bool DestroyRenderGraph(GDevice* device, RenderGraph* render_graph);
		static bool BeginRenderGraphPass(RenderGraph* render_graph, RenderGraphPass* pass);
		static bool EndRenderGraphPass(RenderGraph* render_graph, RenderGraphPass* pass);
		static bool BeginRenderGraph(RenderGraph* render_graph);
		static bool EndRenderGraph(RenderGraph* render_graph);
		static bool BindRenderGraphTexture(RenderGraph* render_graph, GPipeline* pipeline, const std::string& bind_name, ShaderResourceStage resource_stage, RenderGraphResource* resource, uint32_t index = 0);
		static bool BindRenderGraphBuffer(RenderGraph* render_graph, GPipeline* pipeline, const std::string& bind_name, ShaderResourceStage resource_stage, RenderGraphResource* resource, uint32_t index = 0);

		static bool CreateBatchBuffer(GBuffer* buffer, BufferInfo create_info);
		static bool DestroyBatchBuffer(GBuffer* buffer);
		static bool FlushBatchBuffer(GBuffer* buffer, void* data, uint32_t size);


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
		static __BindLineWidth _bindLineWidth;
		static __BindPipeline _bindPipeline;
		static __BindVertexBuffer _bindVertexBuffer;
		static __BindVertexBufferBatch _bindVertexBufferBatch;
		static __BindIndexBuffer _bindIndexBuffer;
		static __BindIndexBufferBatch _bindIndexBufferBatch;
		static __Draw _draw;
		static __DrawIndexed _drawIndexed;
		static __BeginRenderPass _beginRenderPass;
		static __NextSubpass _nextSubpass;
		static __EndRenderPass _endRenderPass;
		static __BeginFrame _beginFrame;
		static __EndFrame _endFrame;
		static __OnDrawStart _onDrawStart;
		static __OnDrawEnd _onDrawEnd;

		static __CreateBuffer _createBuffer;
		static __DestroyBuffer _destroyBuffer;
		static __SetBufferData _setBufferData;
		static __SetBufferDataOffset _setBufferDataOffset;

		static __CreateFramebuffer _createFramebuffer;
		static __DestroyFramebuffer _destroyFramebuffer;

		static __InitializeMaterial _initializeMaterial;
		static __DestroyMaterial _destroyMaterial;
		static __PostInitializeMaterial _postInitializeMaterial;
		static __ApplyMaterial _applyMaterial;

		static  __CreatePipeline _createPipeline;
		static  __DestroyPipeline _destoryPipeline;
		static  __InitializePipeline _initializePipeline;
		static  __ShutDownPipeline _shutDownPipeline;
		static  __SetPipelineData _setPipelineData;
		static  __SetPipelineTextureData _setPipelineTextureData;
		static  __BindPipeline_ _bindPipeline_;

		static __CreateRenderPass _createRenderPass;
		static __DestroyRenderPass _destroyRenderPass;

		static __CreateShader _createShader;
		static __CreateShader_ _createShader_;
		static __DestroyShader _destroyShader;

		static __CreateSwapchain _createSwapchain;
		static __DestroySwapchain _destroySwapchain;
		static __ResizeSwapchain _resizeSwapchain;
		static __PresentSwapchain _presentSwapchain;
		static __GetSwapchainColorBuffer _getSwapchainColorBuffer;
		static __GetSwapchainDepthBuffer _getSwapchainDepthBuffer;

		static __CreateTexture _createTexture;
		static __DestroyTexture _destroyTexture;
		static __GetTextureNativeHandle _getTextureNativeHandle;
		static __GetTextureData _getTextureData;

		static __BuildRenderGraph _buildRenderGraph;
		static __DestroyRenderGraph _destroyRenderGraph;
		static __BeginRenderGraphPass _beginRenderGraphPass;
		static __EndRenderGraphPass _endRenderGraphPass;
		static __BeginRenderGraph _beginRenderGraph;
		static __EndRenderGraph _endRenderGraph;
		static __BindRenderGraphTexture _bindRenderGraphTexture;
		static __BindRenderGraphBuffer _bindRenderGraphBuffer;

		static __CreateBatchBuffer _createBatchBuffer;
		static __DestroyBatchBuffer _destroyBatchBuffer;
		static __FlushBatchBuffer _flushBatchBuffer;


		static __ValidateHandle _validateHandle;

	protected:
		friend RenderFuncLoader;
	};

}
