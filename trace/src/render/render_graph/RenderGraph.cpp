#include "pch.h"

#include "RenderGraph.h"
#include "render/GTexture.h"
#include "render/GSwapchain.h"
#include "io/Logging.h"
#include "render/Renderutils.h"
#include "render/Renderer.h"

namespace trace {



	// Render Graph Pass ===================================================
	RenderGraphPass::RenderGraphPass(const std::string& name, RenderGraph* render_graph)
		:m_renderGraph(render_graph), m_passName(name)
	{
	}

	uint32_t RenderGraphPass::CreateAttachmentOutput(const std::string& name, TextureDesc desc)
	{
		desc.m_attachmentType = AttachmentType::COLOR;
		uint32_t index = m_renderGraph->AddTextureResource(name, desc);
		m_outputs.push_back(index);
		RenderGraphResource* output_tex = &m_renderGraph->GetResource(index);
		m_attachmentOutputs.push_back(index);
		uint32_t pass_index = m_renderGraph->FindPassIndex(m_passName);
		output_tex->written_passes.push_back(pass_index);
		output_tex->create_pass = pass_index;

		return index;
	}

	uint32_t RenderGraphPass::CreateDepthAttachmentOutput(const std::string& name, TextureDesc desc)
	{
		desc.m_attachmentType = AttachmentType::DEPTH;
		uint32_t index = m_renderGraph->AddTextureResource(name, desc);
		m_outputs.push_back(index);
		RenderGraphResource* output_tex = &m_renderGraph->GetResource(index);
		m_depthStencilOutput = index;
		uint32_t pass_index = m_renderGraph->FindPassIndex(m_passName);
		output_tex->written_passes.push_back(pass_index);
		output_tex->create_pass = index;

		return index;
	}

	void RenderGraphPass::AddColorAttachmentInput(const std::string& name)
	{

		uint32_t index = m_renderGraph->FindResourceIndex(name);
		m_inputs.push_back(index);
		RenderGraphResource* input_tex = &m_renderGraph->GetResource(index);
		m_attachmentInputs.push_back(index);
		uint32_t pass_index = m_renderGraph->FindPassIndex(m_passName);
		input_tex->read_passes.push_back(pass_index);

	}

	void RenderGraphPass::AddColorAttachmentOuput(const std::string& name)
	{
		uint32_t index = m_renderGraph->FindResourceIndex(name);
		m_outputs.push_back(index);
		RenderGraphResource* output_tex = &m_renderGraph->GetResource(index);
		m_attachmentOutputs.push_back(index);
		uint32_t pass_index = m_renderGraph->FindPassIndex(m_passName);
		output_tex->written_passes.push_back(pass_index);
	}

	void RenderGraphPass::AddTextureInput(const std::string& name, GTexture* texture)
	{
		TextureDesc& desc = texture->GetTextureDescription();
		desc.m_attachmentType = AttachmentType::COLOR;
		uint32_t index = m_renderGraph->AddTextureResource(name, texture);
		m_inputs.push_back(index);
		RenderGraphResource* input_tex = &m_renderGraph->GetResource(index);
		uint32_t pass_index = m_renderGraph->FindPassIndex(m_passName);
		input_tex->read_passes.push_back(pass_index);

	}

	void RenderGraphPass::AddTextureOutput(const std::string& name, GTexture* texture)
	{
		TextureDesc& desc = texture->GetTextureDescription();
		desc.m_attachmentType = AttachmentType::COLOR;
		uint32_t index = m_renderGraph->AddTextureResource(name, texture);
		m_outputs.push_back(index);
		RenderGraphResource* output_tex = &m_renderGraph->GetResource(index);
		uint32_t pass_index = m_renderGraph->FindPassIndex(m_passName);
		output_tex->written_passes.push_back(pass_index);
	}

	void RenderGraphPass::SetSwapchainOutput(const std::string& name, GSwapchain* swapchain)
	{
		
		uint32_t index = m_renderGraph->AddSwapchainResource(name, swapchain);
		m_outputs.push_back(index);
		RenderGraphResource* output_tex = &m_renderGraph->GetResource(index);
		uint32_t pass_index = m_renderGraph->FindPassIndex(m_passName);
		output_tex->written_passes.push_back(pass_index);
		m_attachmentOutputs.push_back(index);
	}

	void RenderGraphPass::SetDepthStencilInput(const std::string& name)
	{
		uint32_t res_index = m_renderGraph->FindResourceIndex(name);
		m_inputs.push_back(res_index);
		RenderGraphResource* output_tex = &m_renderGraph->GetResource(res_index);
		m_depthStencilInput = res_index;
		uint32_t pass_index = m_renderGraph->FindPassIndex(m_passName);
		output_tex->read_passes.push_back(pass_index);
	}

	void RenderGraphPass::SetDepthStencilOutput(const std::string& name)
	{
		uint32_t index = m_renderGraph->FindResourceIndex(name);
		m_outputs.push_back(index);
		RenderGraphResource* output_tex = &m_renderGraph->GetResource(index);
		m_depthStencilOutput = index;
		uint32_t pass_index = m_renderGraph->FindPassIndex(m_passName);
		output_tex->written_passes.push_back(pass_index);
	}



	// Render Graph =========================================================

	RenderGraph::RenderGraph()
	{
	}

	RenderGraph::~RenderGraph()
	{
	}

	RenderGraphPass* RenderGraph::AddPass(const std::string& pass_name, GPU_QUEUE queue)
	{
		auto it = std::find_if(m_passes.begin(), m_passes.end(), [pass_name](RenderGraphPass pass) { return pass_name == pass.GetPassName(); });

		if (it != m_passes.end())
		{
			TRC_WARN("Pass has already been added -> {}", it->GetPassName());
			return &(*it);
		}

		uint32_t pass_index = static_cast<uint32_t>(m_passes.size());
		m_passes.push_back(RenderGraphPass(pass_name, this));

		return &m_passes[pass_index];
	}

	uint32_t RenderGraph::AddTextureResource(const std::string& resource_name, const TextureDesc& desc)
	{
		auto it = std::find_if(m_resources.begin(), m_resources.end(), [resource_name](RenderGraphResource resource) { return resource_name == resource.resource_name; });

		if (it != m_resources.end())
		{
			//TRC_INFO("These resource exists -> {}", it->resource_name);
			return FindResourceIndex(it->resource_name);
		}

		RenderGraphResourceData resource_data = {};
		resource_data.texture.attachment_type = desc.m_attachmentType;
		resource_data.texture.format = desc.m_format;
		resource_data.texture.width = desc.m_width;
		resource_data.texture.height = desc.m_height;
		resource_data.texture.image_type = desc.m_image_type;
		resource_data.texture.addressModeU = desc.m_addressModeU;
		resource_data.texture.addressModeV = desc.m_addressModeV;
		resource_data.texture.addressModeW = desc.m_addressModeW;
		resource_data.texture.min = desc.m_minFilterMode;
		resource_data.texture.mag = desc.m_magFilterMode;

		RenderGraphResource resource = {};
		resource.external = false;
		resource.resource_name = resource_name;
		resource.resource_type = RenderGraphResourceType::Texture;
		resource.resource_data = resource_data;

		uint32_t resource_index = static_cast<uint32_t>(m_resources.size());
		m_resources.push_back(resource);

		return resource_index;
	}

	uint32_t RenderGraph::AddTextureResource(const std::string& resource_name, GTexture* texture)
	{
		auto it = std::find_if(m_resources.begin(), m_resources.end(), [resource_name](RenderGraphResource resource) { return resource_name == resource.resource_name; });

		if (it != m_resources.end())
		{
			//TRC_INFO("These resource exists -> {}", it->resource_name);
			return FindResourceIndex(it->resource_name);
		}

		TextureDesc& desc = texture->GetTextureDescription();

		RenderGraphResourceData resource_data = {};
		resource_data.texture.attachment_type = desc.m_attachmentType;
		resource_data.texture.format = desc.m_format;
		resource_data.texture.width = desc.m_width;
		resource_data.texture.height = desc.m_height;
		resource_data.texture.image_type = desc.m_image_type;
		resource_data.texture.addressModeU = desc.m_addressModeU;
		resource_data.texture.addressModeV = desc.m_addressModeV;
		resource_data.texture.addressModeW = desc.m_addressModeW;
		resource_data.texture.min = desc.m_minFilterMode;
		resource_data.texture.mag = desc.m_magFilterMode;


		RenderGraphResource resource = {};
		resource.external = true;
		resource.resource_name = resource_name;
		resource.resource_type = RenderGraphResourceType::Texture;
		resource.resource_data = resource_data;

		uint32_t resource_index = static_cast<uint32_t>(m_resources.size());
		m_resources.push_back(resource);

		return resource_index;
	}

	void RenderGraph::ModifyTextureResource(const std::string& resource_name, const TextureDesc& desc)
	{
		auto it = std::find_if(m_resources.begin(), m_resources.end(), [resource_name](RenderGraphResource resource) { return resource_name == resource.resource_name; });

		if (it == m_resources.end())
		{
			TRC_INFO("These resource does not exists -> {}", it->resource_name);
			return ;
		}

		RenderGraphResourceData resource_data = {};

		//TODO: Maybe allow all parameters to be modified
		//resource_data.texture.attachment_type = desc.m_attachmentType;
		//resource_data.texture.format = desc.m_format;
		//resource_data.texture.image_type = desc.m_image_type;
		//resource_data.texture.addressModeU = desc.m_addressModeU;
		//resource_data.texture.addressModeV = desc.m_addressModeV;
		//resource_data.texture.addressModeW = desc.m_addressModeW;
		//resource_data.texture.min = desc.m_minFilterMode;
		//resource_data.texture.mag = desc.m_magFilterMode;
		
		it->resource_data.texture.width = desc.m_width;
		it->resource_data.texture.height = desc.m_height;

	}

	uint32_t RenderGraph::AddSwapchainResource(const std::string& name, GSwapchain* swapchain)
	{
		auto it = std::find_if(m_resources.begin(), m_resources.end(), [&name](RenderGraphResource resource) { return name == resource.resource_name; });

		if (it != m_resources.end())
		{
			//TRC_INFO("These resource exists -> {}", it->resource_name);
			return FindResourceIndex(it->resource_name);
		}

		//TODO: Resolve hard coded values
		RenderGraphResourceData resource_data = {};
		resource_data.texture.attachment_type = AttachmentType::SWAPCHAIN;
		resource_data.texture.format = Format::R8G8B8A8_SNORM;
		resource_data.texture.width = swapchain->GetWidth();
		resource_data.texture.height = swapchain->GetHeight();

		RenderGraphResource resource = {};
		resource.external = true;
		resource.resource_name = name;
		resource.resource_type = RenderGraphResourceType::SwapchainImage;
		resource.resource_data = resource_data;

		resource.render_handle = (*swapchain->GetRenderHandle());

		uint32_t resource_index = static_cast<uint32_t>(m_resources.size());
		m_resources.push_back(resource);

		return resource_index;
	}

	uint32_t RenderGraph::FindPassIndex(const std::string& pass_name)
	{
		auto it = std::find_if(m_passes.begin(), m_passes.end(), [pass_name](RenderGraphPass pass) { return pass_name == pass.GetPassName(); });

		if (it == m_passes.end())
		{
			TRC_WARN("Pass has not been added or does not exist -> {}", pass_name);
			return INVALID_ID;
		}

		uint32_t i = 0;
		for (RenderGraphPass& pass : m_passes)
		{
			if (pass.GetPassName() == pass_name)
			{
				break;
			}

			i++;
		}

		return i;
	}

	uint32_t RenderGraph::FindResourceIndex(const std::string& resource_name)
	{
		auto it = std::find_if(m_resources.begin(), m_resources.end(), [resource_name](RenderGraphResource resource) { return resource_name == resource.resource_name; });

		if (it == m_resources.end())
		{
			TRC_WARN("Pass has not been added or does not exist -> {}", resource_name);
			return INVALID_ID;
		}

		uint32_t i = 0;
		for (RenderGraphResource& resource : m_resources)
		{
			if (resource.resource_name == resource_name)
			{
				break;
			}

			i++;
		}

		return i;
	}

	RenderGraphPass& RenderGraph::GetPass(uint32_t index)
	{
		return m_passes[index];
	}

	RenderGraphResource& RenderGraph::GetResource(uint32_t index)
	{
		return m_resources[index];
	}

	void RenderGraph::SetFinalResourceOutput(const std::string& resource_name)
	{

		uint32_t index = FindResourceIndex(resource_name);
		if (index == INVALID_ID)
		{
			TRC_ERROR("Ensure resource has being used by a render pass -> {}", resource_name);
			m_finalResource = INVALID_ID;
			return;
		}

		m_finalResource = index;

	}

	bool RenderGraph::Compile()
	{
		if (m_finalResource == INVALID_ID)
		{
			TRC_ERROR("Final Graph Resource has not been assigned");
			return false;
		}

		compute_graph();
		return true;
	}

	bool RenderGraph::Execute()
	{
		bool result = RenderFunc::BeginRenderGraph(this);

		
		if (result)
		{

			for (auto& pass_index : m_submissionPasses)
			{
				RenderGraphPass* pass = &GetPass(pass_index);
				RenderFunc::BeginRenderGraphPass(this, pass);

				pass->m_run_cb(pass->m_attachmentInputs);

				RenderFunc::EndRenderGraphPass(this, pass);
			}

			RenderFunc::EndRenderGraph(this);
		}
		return result;
	}

	void RenderGraph::Destroy()
	{
		if (!destroyed)
		{
			RenderFunc::DestroyRenderGraph(&m_renderer->g_device, this);
			m_passes.clear();
			m_submissionPasses.clear();
			m_resources.clear();
			destroyed = true;
		}
	}

	void RenderGraph::Rebuild()
	{
		RenderFunc::BuildRenderGraph(&m_renderer->g_device, this);
		destroyed = false;
	}

	bool RenderGraph::ReConstruct()
	{
		if (!destroyed)
		{
			m_passes.clear();
			m_submissionPasses.clear();
			m_resources.clear();
		}

		return false;
	}

	void RenderGraph::Resize(uint32_t width, uint32_t height)
	{

		for (auto& pass_index : m_submissionPasses)
		{
			RenderGraphPass* pass = &GetPass(pass_index);
			pass->m_resize_cb(this, pass, width, height);

			for (auto& res_index : pass->GetAttachmentOutputs())
			{
				RenderGraphResource* res = &GetResource(res_index);
				if (width == 0)
				{
					width = res->resource_data.texture.width;
				}
				else
				{
					TRC_ASSERT(width == res->resource_data.texture.width, "width of output not equal, {}", res->resource_name);
				}

				if (height == 0)
				{
					height = res->resource_data.texture.height;
				}
				else
				{
					TRC_ASSERT(height == res->resource_data.texture.height, "height of output not equal, {}", res->resource_name);
				}
			}

			pass->renderArea.x = 0;
			pass->renderArea.y = 0;
			pass->renderArea.z = static_cast<float>(width);
			pass->renderArea.w = static_cast<float>(height);
		}

	}

	void RenderGraph::compute_graph()
	{

		compute_edges();

		std::vector<RenderGraphPass*> pass_stack;
		std::vector<RenderGraphPass*> sorted_passes;
		std::vector<int8_t> visited_pass;
		visited_pass.resize(m_passes.size());

		for (uint32_t i = 0; i < m_passes.size(); i++)
			visited_pass[i] = 0;


		for (uint32_t& pass_index : GetResource(m_finalResource).written_passes)
		{
			RenderGraphPass* pass = &GetPass(pass_index);
			pass_stack.push_back(pass);

			while (pass_stack.size() > 0)
			{
				RenderGraphPass* curr_pass = pass_stack.back();
				uint32_t p_index = FindPassIndex(curr_pass->GetPassName());
				if (visited_pass[p_index] > 0)
				{
					pass_stack.pop_back();
					continue;
				}

				pass_stack.pop_back();
				visited_pass[p_index] = 1;
				sorted_passes.push_back(curr_pass);

				uint32_t width = 0;
				uint32_t height = 0;

				for (auto& res_index : curr_pass->GetAttachmentOutputs())
				{
					RenderGraphResource* res = &GetResource(res_index);
					if (width == 0)
					{
						width = res->resource_data.texture.width;
					}
					else
					{
						TRC_ASSERT(width == res->resource_data.texture.width, "width of out not equal, {}", res->resource_name);
					}

					if (height == 0)
					{
						height = res->resource_data.texture.height;
					}
					else
					{
						TRC_ASSERT(height == res->resource_data.texture.height, "height of out not equal, {}", res->resource_name);
					}
				}

				curr_pass->renderArea.x = 0;
				curr_pass->renderArea.y = 0;
				curr_pass->renderArea.z = static_cast<float>(width);
				curr_pass->renderArea.w = static_cast<float>(height);

				curr_pass->clearColor.r = 0.22f;
				curr_pass->clearColor.g = 0.217f;
				curr_pass->clearColor.b = 0.20f;
				curr_pass->clearColor.a = 0.85f;

				curr_pass->depthValue = 1.0f;
				curr_pass->stencilValue = 0;

				for (RenderGraphEdge& edge : curr_pass->GetPassEdges())
				{
					if (!edge.from) continue;
					RenderGraphPass* from = edge.from;
					uint32_t f_index = FindPassIndex(from->GetPassName());
					if (visited_pass[f_index] > 0)
					{
						auto curr_it = std::find(sorted_passes.begin(), sorted_passes.end(), curr_pass);
						auto it = std::find(sorted_passes.begin(), sorted_passes.end(), from);
						if (curr_it < it) continue;
						sorted_passes.erase(curr_it);
						sorted_passes.emplace(it, curr_pass);
					}
					else
					{
						pass_stack.push_back(from);
					}
				}

			}
		}

		for (int i = sorted_passes.size() - 1; i >= 0; i--)
		{
			m_submissionPasses.push_back(FindPassIndex(sorted_passes[i]->GetPassName()));
		}



	}

	void RenderGraph::compute_edges()
	{

		for (auto& resource : m_resources)
		{
			if (resource.written_passes.empty())
			{
				for (auto& pass_index : resource.read_passes)
				{
					RenderGraphPass* pass = &GetPass(pass_index);
					std::vector<RenderGraphEdge>& pass_edges = pass->GetPassEdges();
					RenderGraphEdge edge = {};
					edge.from = nullptr;
					edge.to = pass;
					edge.resource = &resource;
					pass_edges.push_back(edge);
				}
				continue;
			}
			for (auto& pass_index : resource.written_passes)
			{
				RenderGraphPass* pass = &GetPass(pass_index);
				std::vector<RenderGraphEdge>& pass_edges = pass->GetPassEdges();
				for (auto& read_pass_index : resource.read_passes)
				{
					RenderGraphPass* read_pass = &GetPass(read_pass_index);
					RenderGraphEdge edge = {};
					edge.from = pass;
					edge.to = read_pass;
					edge.resource = &resource;
					pass_edges.push_back(edge);
					read_pass->GetPassEdges().push_back(edge);
				}
			}
		}


	}

}