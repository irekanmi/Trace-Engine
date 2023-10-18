#include "pch.h"

#include "backends/Renderutils.h"
#include "render/Material.h"
#include "render/GPipeline.h"
#include "core/platform/Vulkan/VulkanRenderFunc.h"
#include "render/render_graph/RenderGraph.h"
#include "resource/TextureManager.h"


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
			RasterizerState rs = {};
			rs.cull_mode = CullMode::BACK;
			rs.fill_mode = FillMode::SOLID;

			desc.rasteriser_state = rs;
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
			blds.alpha_to_blend_coverage = false;

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
			desc.topology = PRIMITIVETOPOLOGY::TRIANGLE_LIST;
		}
	}

	std::unordered_map<std::string, std::pair<std::any, uint32_t>> GetPipelineMaterialData(Ref<GPipeline> pipeline)
	{
		std::unordered_map<std::string, std::pair<std::any, uint32_t>> result;

		PipelineStateDesc& desc = pipeline->GetDesc();

		auto lambda = [&](ShaderData type, std::any& dst)
		{
			switch (type)
			{
			case ShaderData::CUSTOM_DATA_BOOL: { dst = bool(); break; }
			case ShaderData::CUSTOM_DATA_FLOAT: {dst = float(); break; }
			case ShaderData::CUSTOM_DATA_INT: { dst = int(); break; }
			case ShaderData::CUSTOM_DATA_IVEC2: {dst = glm::ivec2(); break; }
			case ShaderData::CUSTOM_DATA_IVEC3: {dst = glm::ivec3(); break; }
			case ShaderData::CUSTOM_DATA_IVEC4: {dst = glm::ivec4(); break; }
			case ShaderData::CUSTOM_DATA_MAT2: {dst = glm::mat2(); break; }
			case ShaderData::CUSTOM_DATA_MAT3: {dst = glm::mat3(); break; }
			case ShaderData::CUSTOM_DATA_MAT4: {dst = glm::mat4(); break; }
			case ShaderData::CUSTOM_DATA_TEXTURE:{	dst = TextureManager::get_instance()->GetDefault("albedo_map"); break; }
			case ShaderData::CUSTOM_DATA_VEC2: {dst = glm::vec2(); break; }
			case ShaderData::CUSTOM_DATA_VEC3: {dst = glm::vec3(); break; }
			case ShaderData::CUSTOM_DATA_VEC4: {dst = glm::vec4(); break; }
			}
		};

		for (auto i : desc.resources.resources)
		{
			bool is_struct = i.def == ShaderDataDef::STRUCTURE;
			bool is_array = i.def == ShaderDataDef::ARRAY;
			bool is_varible = i.def == ShaderDataDef::VARIABLE;
			bool is_sArray = i.def == ShaderDataDef::STRUCT_ARRAY;

			if (is_struct)
			{
				for (auto& mem : i._struct.members)
				{
					if (mem.resource_name[0] == '_') continue;
					std::any a;
					lambda(mem.resource_data_type, a);
					uint32_t hash = pipeline->_hashTable.Get(mem.resource_name);
					result[mem.resource_name] = std::make_pair(a, hash);
				}

			}
			if (is_array)
			{
				for (auto& mem : i._array.members)
				{
					std::any a;
					lambda(mem.data_type, a);
					uint32_t hash = pipeline->_hashTable.Get(mem.resource_name);
					result[mem.resource_name] = std::make_pair(a, hash);
				}

			}
		}

		return result;
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
		RenderFunc::_drawElements = vk::__DrawElements;
		RenderFunc::_drawInstancedElements = vk::__DrawInstanceElements;
		RenderFunc::_drawIndexed_ = vk::__DrawIndexed_;
		RenderFunc::_drawInstancedIndexed = vk::__DrawInstanceIndexed;
		RenderFunc::_bindViewport = vk::__BindViewport;
		RenderFunc::_bindRect = vk::__BindRect;
		RenderFunc::_bindLineWidth = vk::__BindLineWidth;
		RenderFunc::_bindPipeline = vk::__BindPipeline;
		RenderFunc::_bindVertexBuffer = vk::__BindVertexBuffer;
		RenderFunc::_bindVertexBufferBatch = vk::__BindVertexBufferBatch;
		RenderFunc::_bindIndexBuffer = vk::__BindIndexBuffer;
		RenderFunc::_bindIndexBufferBatch = vk::__BindIndexBufferBatch;
		RenderFunc::_draw = vk::__Draw;
		RenderFunc::_drawIndexed = vk::__DrawIndexed;
		RenderFunc::_beginRenderPass = vk::__BeginRenderPass;
		RenderFunc::_nextSubpass = vk::__NextSubpass;
		RenderFunc::_endRenderPass = vk::__EndRenderPass;
		RenderFunc::_beginFrame = vk::__BeginFrame;
		RenderFunc::_endFrame = vk::__EndFrame;
		RenderFunc::_onDrawStart = vk::__OnDrawStart;
		RenderFunc::_onDrawEnd = vk::__OnDrawEnd;

		RenderFunc::_createBuffer = vk::__CreateBuffer;
		RenderFunc::_destroyBuffer = vk::__DestroyBuffer;
		RenderFunc::_setBufferData = vk::__SetBufferData;
		RenderFunc::_setBufferDataOffset = vk::__SetBufferDataOffset;

		RenderFunc::_createFramebuffer = vk::__CreateFrameBuffer;
		RenderFunc::_destroyFramebuffer = vk::__DestroyFrameBuffer;

		RenderFunc::_initializeMaterial = vk::__InitializeMaterial;
		RenderFunc::_postInitializeMaterial = vk::__PostInitializeMaterial;
		RenderFunc::_applyMaterial = vk::__ApplyMaterial;

		RenderFunc::_createPipeline = vk::__CreatePipeline;
		RenderFunc::_destoryPipeline = vk::__DestroyPipeline;
		RenderFunc::_initializePipeline = vk::__InitializePipeline;
		RenderFunc::_shutDownPipeline = vk::__ShutDownPipeline;
		RenderFunc::_setPipelineData = vk::__SetPipelineData;
		RenderFunc::_setPipelineTextureData = vk::__SetPipelineTextureData;
		RenderFunc::_bindPipeline_ = vk::__BindPipeline_;

		RenderFunc::_createRenderPass = vk::__CreateRenderPass;
		RenderFunc::_destroyRenderPass = vk::__DestroyRenderPass;

		RenderFunc::_createShader = vk::__CreateShader;
		RenderFunc::_createShader_ = vk::__CreateShader_;
		RenderFunc::_destroyShader = vk::__DestroyShader;

		RenderFunc::_createSwapchain = vk::__CreateSwapchain;
		RenderFunc::_destroySwapchain = vk::__DestroySwapchain;
		RenderFunc::_resizeSwapchain = vk::__ResizeSwapchain;
		RenderFunc::_presentSwapchain = vk::__PresentSwapchain;
		RenderFunc::_getSwapchainColorBuffer = vk::__GetSwapchainColorBuffer;
		RenderFunc::_getSwapchainDepthBuffer = vk::__GetSwapchainDepthBuffer;

		RenderFunc::_createTexture = vk::__CreateTexture;
		RenderFunc::_destroyTexture = vk::__DestroyTexture;
		RenderFunc::_getTextureNativeHandle = vk::__GetTextureNativeHandle;

		RenderFunc::_buildRenderGraph = vk::__BuildRenderGraph;
		RenderFunc::_destroyRenderGraph = vk::__DestroyRenderGraph;
		RenderFunc::_beginRenderGraphPass = vk::__BeginRenderGraphPass;
		RenderFunc::_endRenderGraphPass = vk::__EndRenderGraphPass;
		RenderFunc::_beginRenderGraph = vk::__BeginRenderGraph;
		RenderFunc::_endRenderGraph = vk::__EndRenderGraph;
		RenderFunc::_bindRenderGraphTexture = vk::__BindRenderGraphTexture;
		RenderFunc::_bindRenderGraphBuffer = vk::__BindRenderGraphBuffer;

		RenderFunc::_createBatchBuffer = vk::__CreateBatchBuffer;
		RenderFunc::_destroyBatchBuffer = vk::__DestroyBatchBuffer;
		RenderFunc::_flushBatchBuffer = vk::__FlushBatchBuffer;


		return true;
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
	__BindLineWidth RenderFunc::_bindLineWidth = nullptr;
	__BindPipeline RenderFunc::_bindPipeline = nullptr;
	__BindVertexBuffer RenderFunc::_bindVertexBuffer = nullptr;
	__BindVertexBufferBatch RenderFunc::_bindVertexBufferBatch = nullptr;
	__BindIndexBuffer RenderFunc::_bindIndexBuffer = nullptr;
	__BindIndexBufferBatch RenderFunc::_bindIndexBufferBatch = nullptr;
	__Draw RenderFunc::_draw = nullptr;
	__DrawIndexed RenderFunc::_drawIndexed = nullptr;
	__BeginRenderPass RenderFunc::_beginRenderPass = nullptr;
	__NextSubpass RenderFunc::_nextSubpass = nullptr;
	__EndRenderPass RenderFunc::_endRenderPass = nullptr;
	__BeginFrame RenderFunc::_beginFrame = nullptr;
	__EndFrame RenderFunc::_endFrame = nullptr;
	__OnDrawStart RenderFunc::_onDrawStart = nullptr;
	__OnDrawEnd RenderFunc::_onDrawEnd = nullptr;

	__CreateBuffer RenderFunc::_createBuffer = nullptr;
	__DestroyBuffer RenderFunc::_destroyBuffer = nullptr;
	__SetBufferData RenderFunc::_setBufferData = nullptr;
	__SetBufferDataOffset RenderFunc::_setBufferDataOffset = nullptr;

	__CreateFramebuffer RenderFunc::_createFramebuffer = nullptr;
	__DestroyFramebuffer RenderFunc::_destroyFramebuffer = nullptr;

	__InitializeMaterial RenderFunc::_initializeMaterial = nullptr;
	__PostInitializeMaterial RenderFunc::_postInitializeMaterial = nullptr;
	__ApplyMaterial RenderFunc::_applyMaterial = nullptr;

	__CreatePipeline RenderFunc::_createPipeline = nullptr;
	__DestroyPipeline RenderFunc::_destoryPipeline = nullptr;
	__InitializePipeline RenderFunc::_initializePipeline = nullptr;
	__ShutDownPipeline RenderFunc::_shutDownPipeline = nullptr;
	__SetPipelineData RenderFunc::_setPipelineData = nullptr;
	__SetPipelineTextureData RenderFunc::_setPipelineTextureData = nullptr;
	__BindPipeline_ RenderFunc::_bindPipeline_ = nullptr;

	__CreateRenderPass RenderFunc::_createRenderPass = nullptr;
	__DestroyRenderPass RenderFunc::_destroyRenderPass = nullptr;

	__CreateShader RenderFunc::_createShader = nullptr;
	__CreateShader_ RenderFunc::_createShader_ = nullptr;
	__DestroyShader RenderFunc::_destroyShader = nullptr;

	__CreateSwapchain RenderFunc::_createSwapchain = nullptr;
	__DestroySwapchain RenderFunc::_destroySwapchain = nullptr;
	__ResizeSwapchain RenderFunc::_resizeSwapchain = nullptr;
	__PresentSwapchain RenderFunc::_presentSwapchain = nullptr;
	__GetSwapchainColorBuffer RenderFunc::_getSwapchainColorBuffer = nullptr;
	__GetSwapchainDepthBuffer RenderFunc::_getSwapchainDepthBuffer = nullptr;

	__CreateTexture  RenderFunc::_createTexture = nullptr;
	__DestroyTexture RenderFunc::_destroyTexture = nullptr;
	__GetTextureNativeHandle RenderFunc::_getTextureNativeHandle = nullptr;

	__BuildRenderGraph RenderFunc::_buildRenderGraph = nullptr;
	__DestroyRenderGraph RenderFunc::_destroyRenderGraph = nullptr;
	__BeginRenderGraphPass RenderFunc::_beginRenderGraphPass = nullptr;
	__EndRenderGraphPass RenderFunc::_endRenderGraphPass = nullptr;
	__BeginRenderGraph RenderFunc::_beginRenderGraph = nullptr;
	__EndRenderGraph RenderFunc::_endRenderGraph = nullptr;
	__BindRenderGraphTexture RenderFunc::_bindRenderGraphTexture = nullptr;
	__BindRenderGraphBuffer RenderFunc::_bindRenderGraphBuffer = nullptr;

	__CreateBatchBuffer RenderFunc::_createBatchBuffer = nullptr;
	__DestroyBatchBuffer RenderFunc::_destroyBatchBuffer = nullptr;
	__FlushBatchBuffer RenderFunc::_flushBatchBuffer = nullptr;

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

	bool RenderFunc::SetBufferDataOffset(GBuffer* buffer, void* data, uint32_t offset, uint32_t size)
	{
		bool result = true;

		RENDER_FUNC_IS_VALID(_setBufferDataOffset);
		result = _setBufferDataOffset(buffer, data, offset, size);

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

	bool RenderFunc::InitializeMaterial(MaterialInstance* mat_instance, Ref<GPipeline> pipeline, Material material)
	{
		bool result = true;

		RENDER_FUNC_IS_VALID(_initializeMaterial);
		result = _initializeMaterial(mat_instance, pipeline, material);

		return result;
	}

	bool RenderFunc::PostInitializeMaterial(MaterialInstance* mat_instance, Ref<GPipeline> pipeline, Material material)
	{
		bool result = true;

		RENDER_FUNC_IS_VALID(_postInitializeMaterial);
		result = _postInitializeMaterial(mat_instance, pipeline, material);

		return result;
	}

	bool RenderFunc::ApplyMaterial(MaterialInstance* mat_instance)
	{
		bool result = true;

		RENDER_FUNC_IS_VALID(_applyMaterial);
		result = _applyMaterial(mat_instance);

		return result;
	}

	bool RenderFunc::CreatePipeline(GPipeline* pipeline, PipelineStateDesc desc)
	{
		bool result = true;

		RENDER_FUNC_IS_VALID(_createPipeline);
		result = _createPipeline(pipeline, desc);

		return result;
	}

	bool RenderFunc::DestroyPipeline(GPipeline* pipeline)
	{
		bool result = true;

		RENDER_FUNC_IS_VALID(_destoryPipeline);
		result = _destoryPipeline(pipeline);

		return result;
	}

	bool RenderFunc::InitializePipeline(GPipeline* pipeline)
	{
		bool result = true;

		RENDER_FUNC_IS_VALID(_initializePipeline);
		result = _initializePipeline(pipeline);

		return result;
	}

	bool RenderFunc::ShutDownPipeline(GPipeline* pipeline)
	{
		bool result = true;

		RENDER_FUNC_IS_VALID(_shutDownPipeline);
		result = _shutDownPipeline(pipeline);

		return result;
	}

	bool RenderFunc::SetPipelineData(GPipeline* pipeline, const std::string& resource_name, ShaderResourceStage resource_scope, void* data, uint32_t size)
	{
		bool result = true;

		RENDER_FUNC_IS_VALID(_setPipelineData);
		result = _setPipelineData(pipeline, resource_name, resource_scope, data, size);

		return result;
	}

	bool RenderFunc::SetPipelineTextureData(GPipeline* pipeline, const std::string& resource_name, ShaderResourceStage resource_scope, GTexture* texture, uint32_t index)
	{
		bool result = true;

		RENDER_FUNC_IS_VALID(_setPipelineTextureData);
		result = _setPipelineTextureData(pipeline, resource_name, resource_scope, texture, index);

		return result;
	}

	bool RenderFunc::BindPipeline_(GPipeline* pipeline)
	{
		bool result = true;

		RENDER_FUNC_IS_VALID(_bindPipeline_);
		result = _bindPipeline_(pipeline);

		return result;
	}

	bool RenderFunc::CreateRenderPass(GRenderPass* render_pass, RenderPassDescription desc)
	{
		bool result = true;

		RENDER_FUNC_IS_VALID(_createRenderPass);
		result = _createRenderPass(render_pass, desc);

		return result;
	}

	bool RenderFunc::DestroyRenderPass(GRenderPass* render_pass)
	{
		bool result = true;

		RENDER_FUNC_IS_VALID(_destroyRenderPass);
		result = _destroyRenderPass(render_pass);

		return result;
	}

	bool RenderFunc::CreateShader(GShader* shader, const std::string& src, ShaderStage stage)
	{
		bool result = true;

		RENDER_FUNC_IS_VALID(_createShader);
		result = _createShader(shader, src, stage);

		return result;
	}

	bool RenderFunc::CreateShader(GShader* shader, std::vector<uint32_t>& src, ShaderStage stage)
	{
		bool result = true;

		RENDER_FUNC_IS_VALID(_createShader_);
		result = _createShader_(shader, src, stage);

		return result;
	}

	bool RenderFunc::DestroyShader(GShader* shader)
	{
		bool result = true;

		RENDER_FUNC_IS_VALID(_destroyShader);
		result = _destroyShader(shader);

		return result;
	}

	bool RenderFunc::CreateSwapchain(GSwapchain* swapchain, GDevice* device, GContext* context)
	{
		bool result = true;

		RENDER_FUNC_IS_VALID(_createSwapchain);
		result = _createSwapchain(swapchain, device, context);

		return result;
	}

	bool RenderFunc::DestroySwapchain(GSwapchain* swapchain)
	{
		bool result = true;

		RENDER_FUNC_IS_VALID(_destroySwapchain);
		result = _destroySwapchain(swapchain);

		return result;
	}

	bool RenderFunc::ResizeSwapchain(GSwapchain* swapchain, uint32_t width, uint32_t height)
	{
		bool result = true;

		RENDER_FUNC_IS_VALID(_resizeSwapchain);
		result = _resizeSwapchain(swapchain, width, height);

		return result;
	}

	bool RenderFunc::PresentSwapchain(GSwapchain* swapchain)
	{
		bool result = true;

		RENDER_FUNC_IS_VALID(_presentSwapchain);
		result = _presentSwapchain(swapchain);

		return result;
	}

	bool RenderFunc::GetSwapchainColorBuffer(GSwapchain* swapchain, GTexture* out_texture)
	{
		bool result = true;

		RENDER_FUNC_IS_VALID(_getSwapchainColorBuffer);
		result = _getSwapchainColorBuffer(swapchain, out_texture);

		return result;
	}

	bool RenderFunc::GetSwapchainDepthBuffer(GSwapchain* swapchain, GTexture* out_texture)
	{
		bool result = true;

		RENDER_FUNC_IS_VALID(_getSwapchainDepthBuffer);
		result = _getSwapchainDepthBuffer(swapchain, out_texture);

		return result;
	}

	bool RenderFunc::CreateTexture(GTexture* texture, TextureDesc desc)
	{
		bool result = true;

		RENDER_FUNC_IS_VALID(_createTexture);
		result = _createTexture(texture, desc);

		return result;
	}

	bool RenderFunc::DestroyTexture(GTexture* texture)
	{
		bool result = true;

		RENDER_FUNC_IS_VALID(_destroyTexture);
		result = _destroyTexture(texture);

		return result;
	}

	bool RenderFunc::GetTextureNativeHandle(GTexture* texture, void*& out_handle)
	{
		bool result = true;

		RENDER_FUNC_IS_VALID(_getTextureNativeHandle);
		result = _getTextureNativeHandle(texture, out_handle);

		return result;
	}

	bool RenderFunc::BuildRenderGraph(GDevice* device, RenderGraph* render_graph)
	{
		bool result = true;

		RENDER_FUNC_IS_VALID(_buildRenderGraph);
		result = _buildRenderGraph(device, render_graph);

		return result;
	}

	bool RenderFunc::DestroyRenderGraph(GDevice* device, RenderGraph* render_graph)
	{
		bool result = true;

		RENDER_FUNC_IS_VALID(_destroyRenderGraph);
		result = _destroyRenderGraph(device, render_graph);

		return result;
	}

	bool RenderFunc::BeginRenderGraphPass(RenderGraph* render_graph, RenderGraphPass* pass)
	{
		bool result = true;

		RENDER_FUNC_IS_VALID(_beginRenderGraphPass);
		result = _beginRenderGraphPass(render_graph, pass);

		return result;
	}

	bool RenderFunc::EndRenderGraphPass(RenderGraph* render_graph, RenderGraphPass* pass)
	{
		bool result = true;

		RENDER_FUNC_IS_VALID(_endRenderGraphPass);
		result = _endRenderGraphPass(render_graph, pass);

		return result;
	}

	bool RenderFunc::BeginRenderGraph(RenderGraph* render_graph)
	{
		bool result = true;

		RENDER_FUNC_IS_VALID(_beginRenderGraph);
		result = _beginRenderGraph(render_graph);

		return result;
	}

	bool RenderFunc::EndRenderGraph(RenderGraph* render_graph)
	{
		bool result = true;

		RENDER_FUNC_IS_VALID(_endRenderGraph);
		result = _endRenderGraph(render_graph);

		return result;
	}

	bool RenderFunc::BindRenderGraphTexture(RenderGraph* render_graph, GPipeline* pipeline, const std::string& bind_name, ShaderResourceStage resource_stage, RenderGraphResource* resource, uint32_t index)
	{
		bool result = true;

		RENDER_FUNC_IS_VALID(_bindRenderGraphTexture);
		result = _bindRenderGraphTexture(render_graph, pipeline, bind_name, resource_stage, resource, index);

		return result;
	}

	bool RenderFunc::BindRenderGraphBuffer(RenderGraph* render_graph, GPipeline* pipeline, const std::string& bind_name, ShaderResourceStage resource_stage, RenderGraphResource* resource, uint32_t index)
	{
		bool result = true;

		RENDER_FUNC_IS_VALID(_bindRenderGraphBuffer);
		result = _bindRenderGraphBuffer(render_graph, pipeline, bind_name, resource_stage, resource, index);

		return result;
	}

	bool RenderFunc::CreateBatchBuffer(GBuffer* buffer, BufferInfo create_info)
	{
		bool result = true;

		RENDER_FUNC_IS_VALID(_createBatchBuffer);
		result = _createBatchBuffer(buffer, create_info);

		return result;
	}

	bool RenderFunc::DestroyBatchBuffer(GBuffer* buffer)
	{
		bool result = true;

		RENDER_FUNC_IS_VALID(_destroyBatchBuffer);
		result = _destroyBatchBuffer(buffer);

		return result;
	}

	bool RenderFunc::FlushBatchBuffer(GBuffer* buffer, void* data, uint32_t size)
	{
		bool result = true;

		RENDER_FUNC_IS_VALID(_flushBatchBuffer);
		result = _flushBatchBuffer(buffer, data, size);

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

	bool RenderFunc::BindLineWidth(GDevice* device, float value)
	{
		bool result = true;

		RENDER_FUNC_IS_VALID(_bindLineWidth);
		result = _bindLineWidth(device, value);

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
	
	bool RenderFunc::BindVertexBufferBatch(GDevice* device, GBuffer* buffer)
	{
		bool result = true;

		RENDER_FUNC_IS_VALID(_bindVertexBufferBatch);
		result = _bindVertexBufferBatch(device, buffer);

		return result;
	}

	bool RenderFunc::BindIndexBuffer(GDevice* device, GBuffer* buffer)
	{
		bool result = true;

		RENDER_FUNC_IS_VALID(_bindIndexBuffer);
		result = _bindIndexBuffer(device, buffer);

		return result;
	}
	
	bool RenderFunc::BindIndexBufferBatch(GDevice* device, GBuffer* buffer)
	{
		bool result = true;

		RENDER_FUNC_IS_VALID(_bindIndexBufferBatch);
		result = _bindIndexBufferBatch(device, buffer);

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

	bool RenderFunc::OnDrawStart(GDevice* device, GPipeline* pipeline)
	{
		bool result = true;

		RENDER_FUNC_IS_VALID(_onDrawStart);
		result = _onDrawStart(device, pipeline);

		return result;
	}

	bool RenderFunc::OnDrawEnd(GDevice* device, GPipeline* pipeline)
	{
		bool result = true;

		RENDER_FUNC_IS_VALID(_onDrawEnd);
		result = _onDrawEnd(device, pipeline);

		return result;
	}

}