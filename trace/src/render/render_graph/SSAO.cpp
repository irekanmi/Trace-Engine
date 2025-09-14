#include "pch.h"

#include "SSAO.h"
#include "backends/Renderutils.h"
#include "render/Renderer.h"
#include "render/GShader.h"
#include "FrameData.h"
#include "core/Utils.h"
#include "render/GShader.h"
#include "render/ShaderParser.h"

#include "resource/GenericAssetManager.h"
#include "RenderGraph.h"
#include <random>




namespace trace {

	extern UUID GetUUIDFromName(const std::string& name);

	void SSAO::Init(Renderer* renderer)
	{

		m_renderer = renderer;

		{
			AttachmentInfo ssao_attach;
			ssao_attach.attachmant_index = 0;
			ssao_attach.attachment_format = Format::R8_UNORM;
			ssao_attach.initial_format = TextureFormat::UNKNOWN;
			ssao_attach.final_format = TextureFormat::COLOR_ATTACHMENT;
			ssao_attach.is_depth = false;
			ssao_attach.load_operation = AttachmentLoadOp::LOAD_OP_CLEAR;
			ssao_attach.store_operation = AttachmentStoreOp::STORE_OP_STORE;

			AttachmentInfo att_infos[] = {
				ssao_attach
			};

			SubPassDescription subpass_desc;
			subpass_desc.attachment_count = 1;
			subpass_desc.attachments = att_infos;

			RenderPassDescription pass_desc;
			pass_desc.subpass = subpass_desc;
			pass_desc.render_area = { 0, 0, 800, 600 };
			pass_desc.clear_color = { .0f, .01f, 0.015f, 1.0f };
			pass_desc.depth_value = 1.0f;
			pass_desc.stencil_value = 0;




			RenderFunc::CreateRenderPass(&m_renderPass, pass_desc);
			m_renderer->GetAvaliableRenderPasses()["SSAO_MAIN_PASS"] = &m_renderPass;


			RenderFunc::CreateRenderPass(&ssao_blur, pass_desc);
			m_renderer->GetAvaliableRenderPasses()["SSAO_BLUR_PASS"] = &ssao_blur;
		};

		{
			TextureDesc gPos = {};
			gPos.m_addressModeU = gPos.m_addressModeV = gPos.m_addressModeW = AddressMode::CLAMP_TO_EDGE;
			gPos.m_attachmentType = AttachmentType::COLOR;
			gPos.m_flag = BindFlag::RENDER_TARGET_BIT;
			gPos.m_format = Format::R8_UNORM;
			gPos.m_width = 800;
			gPos.m_height = 600;
			gPos.m_minFilterMode = gPos.m_magFilterMode = FilterMode::LINEAR;
			gPos.m_mipLevels = gPos.m_numLayers = 1;
			gPos.m_usage = UsageFlag::DEFAULT;

			ssao_main_desc = gPos;

		};

		GenericAssetManager* asset_manager = GenericAssetManager::get_instance();

		if(AppSettings::is_editor)
		{
			Ref<GShader> VertShader;
			Ref<GShader> FragShader;

			Ref<GShader> vert_shader = asset_manager->CreateAssetHandle<GShader>("fullscreen.vert.glsl", "fullscreen.vert.glsl", ShaderStage::VERTEX_SHADER);

			Ref<GShader> frag_shader = asset_manager->CreateAssetHandle<GShader>("ssao_main.frag.glsl", "ssao_main.frag.glsl", ShaderStage::PIXEL_SHADER);

			VertShader = vert_shader;
			FragShader = frag_shader;

			ShaderResources sres = {};
			ShaderParser::generate_shader_resources(VertShader.get(), sres);
			ShaderParser::generate_shader_resources(FragShader.get(), sres);

			PipelineStateDesc ds2 = {};
			ds2.vertex_shader = VertShader.get();
			ds2.pixel_shader = FragShader.get();
			ds2.resources = sres;
			ds2.input_layout = {};


			AutoFillPipelineDesc(
				ds2,
				false
			);
			ds2.render_pass = Renderer::get_instance()->GetRenderPass("SSAO_MAIN_PASS");
			ds2.depth_sten_state = { false, false };
			ds2.rasteriser_state = { CullMode::FRONT, FillMode::SOLID };


			m_pipeline = asset_manager->CreateAssetHandle<GPipeline>("ssao_main_pass_pipeline", ds2);
			if (!m_pipeline)
			{
				TRC_ERROR("Failed to initialize or create ssao_main_pass_pipeline");
				return;
			}


			VertShader = asset_manager->CreateAssetHandle<GShader>("fullscreen.vert.glsl", "fullscreen.vert.glsl", ShaderStage::VERTEX_SHADER);
			FragShader = asset_manager->CreateAssetHandle<GShader>("ssao_blur.frag.glsl", "ssao_blur.frag.glsl", ShaderStage::PIXEL_SHADER);

			ShaderResources s_res = {};
			ShaderParser::generate_shader_resources(VertShader.get(), s_res);
			ShaderParser::generate_shader_resources(FragShader.get(), s_res);

			PipelineStateDesc _ds2 = {};
			_ds2.vertex_shader = VertShader.get();
			_ds2.pixel_shader = FragShader.get();
			_ds2.resources = s_res;
			_ds2.input_layout = {};


			AutoFillPipelineDesc(
				_ds2,
				false
			);
			_ds2.render_pass = Renderer::get_instance()->GetRenderPass("SSAO_BLUR_PASS");
			_ds2.depth_sten_state = { false, false };
			_ds2.rasteriser_state = { CullMode::FRONT, FillMode::SOLID };



			m_blurPipe = asset_manager->CreateAssetHandle<GPipeline>("ssao_blur_pass_pipeline", _ds2);
			if (!m_blurPipe)
			{
				TRC_ERROR("Failed to initialize or create ssao_blur_pass_pipeline");
				return;
			}

		}
		else
		{
			UUID id = GetUUIDFromName("ssao_main_pass_pipeline");
			m_pipeline = asset_manager->Load_Runtime<GPipeline>(id);
			id = GetUUIDFromName("ssao_blur_pass_pipeline");
			m_blurPipe = asset_manager->Load_Runtime<GPipeline>(id);
		}

		{
			std::uniform_real_distribution<float> rand_float(0.0f, 1.0f);
			std::default_random_engine gen;
			for (uint32_t i = 0; i < MAX_NUM_KERNEL; i++)
			{
				glm::vec3 samp = glm::vec3(
					rand_float(gen) * 2.0f - 1.0f,
					rand_float(gen) * 2.0f - 1.0f,					
					rand_float(gen)					
				);
				samp = glm::normalize(samp);
				samp *= rand_float(gen);
				
				float scale = (float)i / MAX_NUM_KERNEL;
				scale = lerp(0.1f, 1.0f, scale * scale);
				samp *= scale;
				m_kernel[i] = glm::vec4(samp, 0.0f);
			}

			std::vector<glm::vec4> noise;
			noise.resize(16);
			for (uint32_t i = 0; i < 16; i++)
			{
				glm::vec3 samp = glm::vec3(
					rand_float(gen) * 2.0f - 1.0f,
					rand_float(gen) * 2.0f - 1.0f,
					0.0f
				);
				samp = glm::normalize(samp);
				noise[i] = glm::vec4(samp, 0.0f);
			}

			TextureDesc gPos = {};
			gPos.m_addressModeU = gPos.m_addressModeV = gPos.m_addressModeW = AddressMode::REPEAT;
			gPos.m_attachmentType = AttachmentType::COLOR;
			gPos.m_flag = BindFlag::SHADER_RESOURCE_BIT;
			gPos.m_format = Format::R32G32B32A32_FLOAT;
			gPos.m_width = 4;
			gPos.m_height = 4;
			gPos.m_minFilterMode = gPos.m_magFilterMode = FilterMode::LINEAR;
			gPos.m_mipLevels = gPos.m_numLayers = 1;
			gPos.m_usage = UsageFlag::DEFAULT;
			gPos.m_data.push_back(reinterpret_cast<unsigned char*>(noise.data()));
			gPos.m_channels = 4;

			RenderFunc::CreateTexture(&noise_tex, gPos);


		};

	}
	void SSAO::Setup(RenderGraph* render_graph, RGBlackBoard& black_board, int32_t render_graph_index, int32_t draw_index)
	{
		RenderGraphFrameData* graph_data = m_renderer->GetRenderGraphData(render_graph_index);
		if (!graph_data->_camera)
		{
			return;
		}
		RenderGraphPass* main_pass = render_graph->AddPass("SSAO_MAIN_PASS", GPU_QUEUE::GRAPHICS);

		FrameData& fd = black_board.get<FrameData>();
		GBufferData& gbuffer_data = black_board.get<GBufferData>();
		SSAOData& ssao_data = black_board.add<SSAOData>();

		ssao_main_desc.m_width = fd.frame_width / 2;
		ssao_main_desc.m_height = fd.frame_height / 2;
		

		main_pass->AddColorAttachmentInput(
			gbuffer_data.position_index
		);

		main_pass->AddColorAttachmentInput(
			gbuffer_data.normal_index
		);

		ssao_data.ssao_main = main_pass->CreateAttachmentOutput(
			"ssao_main",
			ssao_main_desc
		);

		uint32_t width = render_graph->GetResource(ssao_data.ssao_main).resource_data.texture.width;
		uint32_t height = render_graph->GetResource(ssao_data.ssao_main).resource_data.texture.height;

		main_pass->SetRunCB([=](Renderer* renderer, RenderGraph* render_graph, RenderGraphPass* render_graph_pass, int32_t render_graph_index, std::vector<uint32_t>& inputs) {
			RenderGraphFrameData* graph_data = renderer->GetRenderGraphData(render_graph_index);

			RenderFunc::OnDrawStart(m_renderer->GetDevice(), m_pipeline.get());

			RenderFunc::SetPipelineData(
				m_pipeline.get(),
				"u_kernel",
				ShaderResourceStage::RESOURCE_STAGE_GLOBAL,
				m_kernel.data(),
				static_cast<uint32_t>(m_kernel.size() * sizeof(glm::vec4)),
				0,
				render_graph_index
			);

			glm::vec2 frame_size(static_cast<float>(fd.frame_width), static_cast<float>(fd.frame_height));

			RenderFunc::SetPipelineData(
				m_pipeline.get(),
				"frame_size",
				ShaderResourceStage::RESOURCE_STAGE_GLOBAL,
				&frame_size,
				sizeof(glm::vec2),
				0,
				render_graph_index
			);

			glm::mat4 proj = graph_data->_camera->GetProjectionMatix();

			RenderFunc::SetPipelineData(
				m_pipeline.get(),
				"_projection",
				ShaderResourceStage::RESOURCE_STAGE_GLOBAL,
				&proj,
				sizeof(glm::mat4),
				0,
				render_graph_index
			);

			RenderFunc::SetPipelineTextureData(
				m_pipeline.get(),
				"u_noiseTexture",
				ShaderResourceStage::RESOURCE_STAGE_GLOBAL,
				&noise_tex,
				render_graph_index
			);

			RenderFunc::BindRenderGraphTexture( render_graph, m_pipeline.get(), "g_bufferData", ShaderResourceStage::RESOURCE_STAGE_GLOBAL, render_graph->GetResource_ptr(gbuffer_data.position_index), render_graph_index, 0 );

			RenderFunc::BindRenderGraphTexture( render_graph, m_pipeline.get(), "g_bufferData", ShaderResourceStage::RESOURCE_STAGE_GLOBAL, render_graph->GetResource_ptr(gbuffer_data.normal_index), render_graph_index, 1 );

			Viewport view_port = m_renderer->_viewPort;
			Rect2D rect = m_renderer->_rect;
			view_port.width = static_cast<float>(width);
			view_port.height = static_cast<float>(height);

			rect.right = width;
			rect.bottom = height;
			RenderFunc::BindViewport(m_renderer->GetDevice(), view_port);
			RenderFunc::BindRect(m_renderer->GetDevice(), rect);
			RenderFunc::BindPipeline_(m_pipeline.get(), render_graph_index);
			RenderFunc::BindPipeline(m_renderer->GetDevice(), m_pipeline.get());

			RenderFunc::Draw(m_renderer->GetDevice(), 0, 3);

			RenderFunc::OnDrawEnd(m_renderer->GetDevice(), m_pipeline.get());

			});

		RenderGraphPass* blur_pass = render_graph->AddPass("SSAO_BLUR_PASS", GPU_QUEUE::GRAPHICS);

		blur_pass->AddColorAttachmentInput(
			"ssao_main"
		);

		ssao_data.ssao_blur = blur_pass->CreateAttachmentOutput(
			"ssao_blur",
			ssao_main_desc
		);

		blur_pass->SetRunCB([=](Renderer* renderer, RenderGraph* render_graph, RenderGraphPass* render_graph_pass, int32_t render_graph_index, std::vector<uint32_t>& inputs) {
			RenderGraphFrameData* graph_data = renderer->GetRenderGraphData(render_graph_index);

			RenderFunc::OnDrawStart(m_renderer->GetDevice(), m_blurPipe.get());
			RenderFunc::BindRenderGraphTexture(
				render_graph,
				m_blurPipe.get(),
				"ssao_main",
				ShaderResourceStage::RESOURCE_STAGE_GLOBAL,
				render_graph->GetResource_ptr(ssao_data.ssao_main),
				render_graph_index
			);
			Viewport view_port = m_renderer->_viewPort;
			Rect2D rect = m_renderer->_rect;
			view_port.width = static_cast<float>(width);
			view_port.height = static_cast<float>(height);

			rect.right = width;
			rect.bottom = height;
			RenderFunc::BindViewport(m_renderer->GetDevice(), view_port);
			RenderFunc::BindRect(m_renderer->GetDevice(), rect);
			RenderFunc::BindPipeline_(m_blurPipe.get(), render_graph_index);
			RenderFunc::BindPipeline(m_renderer->GetDevice(), m_blurPipe.get());

			RenderFunc::Draw(m_renderer->GetDevice(), 0, 3);
			RenderFunc::OnDrawEnd(m_renderer->GetDevice(), m_blurPipe.get());

			});

	}
	void SSAO::ShutDown()
	{
		RenderFunc::DestroyTexture(&noise_tex);
		RenderFunc::DestroyRenderPass(&ssao_blur);
		RenderFunc::DestroyRenderPass(&m_renderPass);
	}
}
