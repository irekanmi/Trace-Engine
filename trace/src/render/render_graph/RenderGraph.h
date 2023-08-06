#pragma once

#include "core/Core.h"
#include "core/Enums.h"
#include <string>
#include <vector>
#include <functional>
#include "render/Graphics.h"
#include "render/GHandle.h"

namespace trace {
	class GTexture;
	class GSwapchain;
	class RenderGraph;
	class RenderGraphPass;
	class Renderer;

	enum RenderGraphResourceType
	{
		Texture,
		Buffer,
		SwapchainImage
	};

	struct RenderGraphResourceData
	{
		struct
		{
			Format format;
			uint32_t size;
			uint32_t stride;
		} buffer;

		struct
		{
			uint32_t width;
			uint32_t height;
			Format format;
			AttachmentType attachment_type;
			ImageType image_type;
			AddressMode addressModeU;
			AddressMode addressModeV;
			AddressMode addressModeW;
			FilterMode min;
			FilterMode mag;
		} texture;

	};

	struct RenderGraphResource
	{
		RenderGraphResourceType resource_type;
		RenderGraphResourceData resource_data;
		uint32_t ref_count;
		std::string resource_name;
		GPU_QUEUE queue;
		GHandle render_handle;
		std::vector<uint32_t> read_passes;
		std::vector<uint32_t> written_passes;
		uint32_t create_pass = INVALID_ID;
		bool external = false;
	};

	struct RenderGraphEdge
	{
		RenderGraphPass* from = nullptr;
		RenderGraphPass* to = nullptr;

		RenderGraphResource* resource = nullptr;
	};

	class TRACE_API RenderGraphPass
	{

	public:
				
		void CreateAttachmentOutput(const std::string& name, TextureDesc desc);
		void CreateDepthAttachmentOutput(const std::string& name, TextureDesc desc);
		void AddColorAttachmentInput(const std::string& name);
		void AddColorAttachmentOuput(const std::string& name);
		void AddTextureInput(const std::string& name, GTexture* texture);
		void AddTextureOutput(const std::string& name, GTexture* texture);
		void SetSwapchainOutput(const std::string& name, GSwapchain* swapchain);
		void SetDepthStencilInput(const std::string& name);
		void SetDepthStencilOutput(const std::string& name);
		void SetRunCB(std::function<void(std::vector<uint32_t>&)> run_cb) { m_run_cb = run_cb; }
		void SetResizeCB(std::function<void(RenderGraph*, RenderGraphPass*, uint32_t, uint32_t)> resize_cb) { m_resize_cb = resize_cb; }
		uint32_t GetDepthStencilInput() { return m_depthStencilInput; }
		uint32_t GetDepthStencilOutput() { return m_depthStencilOutput; }
		std::vector<uint32_t>& GetAttachmentInputs() { return m_attachmentInputs; }
		std::vector<uint32_t>& GetAttachmentOutputs() { return m_attachmentOutputs; }
		std::vector<uint32_t>& GetPassInputs() { return m_inputs; }
		std::vector<uint32_t>& GetPassOutputs() { return m_outputs; }
		std::string& GetPassName() { return m_passName; }
		std::vector<RenderGraphEdge>& GetPassEdges() { return m_edges; }
		void SetRenderGraph(RenderGraph* render_graph) { m_renderGraph = render_graph; }
		void SetGpuQueue(GPU_QUEUE queue) { m_queue = queue; }

		GHandle* GetRenderHandle() { return &m_renderHandle; }

	private:
		RenderGraphPass(const std::string& name, RenderGraph* render_graph);

	public:
		glm::vec4 clearColor;
		glm::vec4 renderArea;
		float depthValue;
		uint32_t stencilValue;

	private:
		RenderGraph* m_renderGraph;
		std::string m_passName;
		std::vector<RenderGraphEdge> m_edges;
		std::vector<uint32_t> m_inputs;
		std::vector<uint32_t> m_outputs;
		std::vector<uint32_t> m_attachmentInputs;
		std::vector<uint32_t> m_attachmentOutputs;
		uint32_t m_depthStencilInput = INVALID_ID;
		uint32_t m_depthStencilOutput = INVALID_ID;
		GHandle m_renderHandle;
		GPU_QUEUE m_queue;
		std::function<void(std::vector<uint32_t>&)> m_run_cb;
		std::function<void(RenderGraph*, RenderGraphPass*, uint32_t, uint32_t)> m_resize_cb;


	protected:
		friend RenderGraph;

	};

	class TRACE_API RenderGraph
	{
	public:
		RenderGraph();
		~RenderGraph();

		RenderGraphPass* AddPass(const std::string& pass_name, GPU_QUEUE queue);
		GHandle* GetRenderHandle() { return &m_renderHandle; }
		std::vector<RenderGraphPass>& GetPasses() { return m_passes; }
		std::vector<uint32_t>& GetSubmissionPasses() { return m_submissionPasses; }
		std::vector<RenderGraphResource>& GetResources() { return m_resources; }
		uint32_t AddTextureResource(const std::string& resource_name, const TextureDesc& desc);
		uint32_t AddTextureResource(const std::string& resource_name, GTexture* texture);
		void ModifyTextureResource(const std::string& resource_name, const TextureDesc& desc);
		uint32_t AddSwapchainResource(const std::string& name, GSwapchain* swapchain);
		uint32_t FindPassIndex(const std::string& pass_name);
		uint32_t FindResourceIndex(const std::string& resource_name);
		RenderGraphPass& GetPass(uint32_t index);
		RenderGraphResource& GetResource(uint32_t index);
		void SetFinalResourceOutput(const std::string& resource_name);
		void SetRenderer(Renderer* renderer) { m_renderer = renderer; }
		bool Compile();
		bool Execute();
		void Destroy();
		void Rebuild();
		bool ReConstruct();
		void Resize(uint32_t width, uint32_t height);

	private:
		void compute_graph();
		void compute_edges();

	private:
		GHandle m_renderHandle;
		Renderer* m_renderer;
		std::vector<RenderGraphPass> m_passes;
		std::vector<uint32_t> m_submissionPasses;
		std::vector<RenderGraphResource> m_resources;
		uint32_t m_finalResource = INVALID_ID;
		bool destroyed = true;

	protected:
	};


}
