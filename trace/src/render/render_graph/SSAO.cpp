#include "pch.h"

#include "SSAO.h"
#include "render/Renderutils.h"
#include "render/Renderer.h"
#include "FrameData.h"
#include "core/Utils.h"
#include "render/GShader.h"
#include "render/ShaderParser.h"
#include "resource/ResourceSystem.h"
#include <random>




namespace trace {
	void SSAO::Init(Renderer* renderer)
	{

		m_renderer = renderer;

		{
			AttachmentInfo ssao_attach;
			ssao_attach.attachmant_index = 0;
			ssao_attach.attachment_format = Format::R16_FLOAT;
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
			m_renderer->_avaliable_passes["SSAO_MAIN_PASS"] = &m_renderPass;


			RenderFunc::CreateRenderPass(&ssao_blur, pass_desc);
			m_renderer->_avaliable_passes["SSAO_BLUR_PASS"] = &ssao_blur;
		};

		{
			TextureDesc gPos = {};
			gPos.m_addressModeU = gPos.m_addressModeV = gPos.m_addressModeW = AddressMode::CLAMP_TO_EDGE;
			gPos.m_attachmentType = AttachmentType::COLOR;
			gPos.m_flag = BindFlag::RENDER_TARGET_BIT;
			gPos.m_format = Format::R16_FLOAT;
			gPos.m_width = 800;
			gPos.m_height = 600;
			gPos.m_minFilterMode = gPos.m_magFilterMode = FilterMode::LINEAR;
			gPos.m_mipLevels = gPos.m_numLayers = 1;
			gPos.m_usage = UsageFlag::DEFAULT;

			ssao_main_desc = gPos;

		};

		{
			std::string vert_src;
			std::string frag_src;
			GShader VertShader;
			GShader FragShader;

			vert_src = ShaderParser::load_shader_file("../assets/shaders/fullscreen.vert.glsl");
			frag_src = ShaderParser::load_shader_file("../assets/shaders/ssao_main.frag.glsl");

			RenderFunc::CreateShader(&VertShader, vert_src, ShaderStage::VERTEX_SHADER);
			RenderFunc::CreateShader(&FragShader, frag_src, ShaderStage::PIXEL_SHADER);

			ShaderResources sres = {};
			ShaderParser::generate_shader_resources(&VertShader, sres);
			ShaderParser::generate_shader_resources(&FragShader, sres);



			PipelineStateDesc ds2 = {};
			ds2.vertex_shader = &VertShader;
			ds2.pixel_shader = &FragShader;
			ds2.resources = sres;
			ds2.input_layout = Vertex2D::get_input_layout();


			AutoFillPipelineDesc(
				ds2,
				false
			);
			ds2.render_pass = Renderer::get_instance()->GetRenderPass("SSAO_MAIN_PASS");
			ds2.depth_sten_state = { false, false };


			if (!ResourceSystem::get_instance()->CreatePipeline(ds2, "ssao_main_pass_pipeline"))
			{
				TRC_ERROR("Failed to initialize or create ssao_main_pass_pipeline");
				RenderFunc::DestroyShader(&VertShader);
				RenderFunc::DestroyShader(&FragShader);
				return;
			}

			RenderFunc::DestroyShader(&VertShader);
			RenderFunc::DestroyShader(&FragShader);

			m_pipeline = ResourceSystem::get_instance()->GetPipeline("ssao_main_pass_pipeline");

			vert_src.clear();
			frag_src.clear();

			vert_src = ShaderParser::load_shader_file("../assets/shaders/fullscreen.vert.glsl");
			frag_src = ShaderParser::load_shader_file("../assets/shaders/ssao_blur.frag.glsl");

			RenderFunc::CreateShader(&VertShader, vert_src, ShaderStage::VERTEX_SHADER);
			RenderFunc::CreateShader(&FragShader, frag_src, ShaderStage::PIXEL_SHADER);

			ShaderResources s_res = {};
			ShaderParser::generate_shader_resources(&VertShader, s_res);
			ShaderParser::generate_shader_resources(&FragShader, s_res);



			PipelineStateDesc _ds2 = {};
			_ds2.vertex_shader = &VertShader;
			_ds2.pixel_shader = &FragShader;
			_ds2.resources = s_res;
			_ds2.input_layout = Vertex2D::get_input_layout();


			AutoFillPipelineDesc(
				_ds2,
				false
			);
			_ds2.render_pass = Renderer::get_instance()->GetRenderPass("SSAO_BLUR_PASS");
			_ds2.depth_sten_state = { false, false };


			if (!ResourceSystem::get_instance()->CreatePipeline(_ds2, "ssao_blur_pass_pipeline"))
			{
				TRC_ERROR("Failed to initialize or create ssao_blur_pass_pipeline");
				RenderFunc::DestroyShader(&VertShader);
				RenderFunc::DestroyShader(&FragShader);
				return;
			}

			RenderFunc::DestroyShader(&VertShader);
			RenderFunc::DestroyShader(&FragShader);

			m_blurPipe = ResourceSystem::get_instance()->GetPipeline("ssao_blur_pass_pipeline");

		};

		{
			std::uniform_real_distribution<float> rand_float(0.0f, 1.0f);
			std::default_random_engine gen;
			for (uint32_t i = 0; i < MAX_NUM_KERNEL; i++)
			{
				glm::vec3 samp = glm::vec3(
					rand_float(gen) * 2.0f - 1.0f,
					0.0f,
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
					rand_float(gen),
					rand_float(gen),
					0.0f
				);
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
	void SSAO::Setup(RenderGraph* render_graph, RGBlackBoard& black_board)
	{

		RenderGraphPass* main_pass = render_graph->AddPass("SSAO_MAIN_PASS", GPU_QUEUE::GRAPHICS);

		FrameData& fd = black_board.get<FrameData>();
		GBufferData& gbuffer_data = black_board.get<GBufferData>();
		SSAOData& ssao_data = black_board.add<SSAOData>();

		ssao_main_desc.m_width = fd.frame_width;
		ssao_main_desc.m_height = fd.frame_height;
		ssao_main_desc.m_format = Format::R16_FLOAT;
		

		main_pass->AddColorAttachmentInput(
			render_graph->GetResource(gbuffer_data.position_index).resource_name
		);

		main_pass->AddColorAttachmentInput(
			render_graph->GetResource(gbuffer_data.normal_index).resource_name
		);

		ssao_data.ssao_main = main_pass->CreateAttachmentOutput(
			"ssao_main",
			ssao_main_desc
		);

		

		main_pass->SetRunCB([=](std::vector<uint32_t>& inputs) {
				
			RenderGraphResource& pos_res = render_graph->GetResource(gbuffer_data.position_index);
			RenderGraphResource& norm_res = render_graph->GetResource(gbuffer_data.normal_index);

			RenderFunc::SetPipelineData(
				m_pipeline.get(),
				"u_kernel",
				ShaderResourceStage::RESOURCE_STAGE_GLOBAL,
				m_kernel.data(),
				static_cast<uint32_t>(m_kernel.size() * sizeof(glm::vec4))
			);

			glm::vec2 frame_size(static_cast<float>(fd.frame_width), static_cast<float>(fd.frame_height));

			RenderFunc::SetPipelineData(
				m_pipeline.get(),
				"frame_size",
				ShaderResourceStage::RESOURCE_STAGE_GLOBAL,
				&frame_size,
				sizeof(glm::vec2)
			);

			glm::mat4 proj = m_renderer->_camera->GetProjectionMatix();

			RenderFunc::SetPipelineData(
				m_pipeline.get(),
				"projection",
				ShaderResourceStage::RESOURCE_STAGE_GLOBAL,
				&proj,
				sizeof(glm::mat4)
			);

			RenderFunc::SetPipelineTextureData(
				m_pipeline.get(),
				"u_noiseTexture",
				ShaderResourceStage::RESOURCE_STAGE_GLOBAL,
				&noise_tex
			);

			RenderFunc::BindRenderGraphResource(
				render_graph,
				m_pipeline.get(),
				"g_bufferData0",
				ShaderResourceStage::RESOURCE_STAGE_GLOBAL,
				&pos_res
			);

			RenderFunc::BindRenderGraphResource(
				render_graph,
				m_pipeline.get(),
				"g_bufferData1",
				ShaderResourceStage::RESOURCE_STAGE_GLOBAL,
				&norm_res,
				1
			);
			RenderFunc::BindViewport(m_renderer->GetDevice(), m_renderer->_viewPort);
			RenderFunc::BindRect(m_renderer->GetDevice(), m_renderer->_rect);
			RenderFunc::BindPipeline_(m_pipeline.get());
			RenderFunc::BindPipeline(m_renderer->GetDevice(), m_pipeline.get());

			RenderFunc::Draw(m_renderer->GetDevice(), 0, 3);


			});

		RenderGraphPass* blur_pass = render_graph->AddPass("SSAO_BLUR_PASS", GPU_QUEUE::GRAPHICS);

		blur_pass->AddColorAttachmentInput(
			"ssao_main"
		);

		ssao_data.ssao_blur = blur_pass->CreateAttachmentOutput(
			"ssao_blur",
			ssao_main_desc
		);

		blur_pass->SetRunCB([=](std::vector<uint32_t>& inputs) {

			RenderGraphResource& res = render_graph->GetResource(ssao_data.ssao_main);

			RenderFunc::BindRenderGraphResource(
				render_graph,
				m_blurPipe.get(),
				res.resource_name,
				ShaderResourceStage::RESOURCE_STAGE_GLOBAL,
				&res
			);
			RenderFunc::BindViewport(m_renderer->GetDevice(), m_renderer->_viewPort);
			RenderFunc::BindRect(m_renderer->GetDevice(), m_renderer->_rect);
			RenderFunc::BindPipeline_(m_blurPipe.get());
			RenderFunc::BindPipeline(m_renderer->GetDevice(), m_blurPipe.get());

			RenderFunc::Draw(m_renderer->GetDevice(), 0, 3);

			});

	}
	void SSAO::ShutDown()
	{
		RenderFunc::DestroyTexture(&noise_tex);
		RenderFunc::DestroyRenderPass(&ssao_blur);
		RenderFunc::DestroyRenderPass(&m_renderPass);
	}
}
