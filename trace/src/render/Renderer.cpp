#include <pch.h>

#include "Renderer.h"
#include "Renderutils.h"
#include "GContext.h"
#include "core/Enums.h"
#include "core/io/Logging.h"
#include "resource/ResourceSystem.h"
#include "render/PerspectiveCamera.h"
#include "core/events/EventsSystem.h"
#include "core/Enums.h"

//Temp============
#include "glm/gtc/matrix_transform.hpp"
#include "core/Utils.h"


namespace trace {

	Renderer* Renderer::s_instance = nullptr;

	Renderer::Renderer()
		: Object(_STR(Renderer))
	{
	}

	Renderer::~Renderer()
	{


	}

	bool Renderer::Init(RenderAPI api)
	{
		bool result = true;

		m_cmdList.resize(8);

		for (uint32_t i = 0; i < 8; i++)
		{
			m_cmdList[i]._commands.reserve(10);
		}
		m_listCount = 0;
	
		switch (api)
		{
		case RenderAPI::OpenGL:
		{
			
			break;

		}

		case RenderAPI::Vulkan:
		{
			RenderFuncLoader::LoadVulkanRenderFunctions();
			break;
		}

		default:
			TRC_ASSERT(false, "Graphics context can not be null");
			return result;
		}

		RenderFunc::CreateContext(&g_context);
		result = RenderFunc::CreateDevice(&g_device);
		RenderFunc::CreateDevice(&g_device);
		{
			
			RenderFunc::CreateSwapchain(&_swapChain, &g_device, &g_context);

			_camera = new PerspectiveCamera(
				glm::vec3(36.186146f, 38.094185f, -19.778137f),
				glm::vec3(-0.92247f, -0.23771466f, 0.30420545f),
				glm::vec3(0.0f, 1.0f, 0.0f),
				((float)800.0f) / ((float)600.0f),
				45.0f,
				0.1f,
				1500.0f
			);
			_viewPort.width = 800.0f;
			_viewPort.height = 600.0f;
			Rect2D rect;
			rect.top = rect.left = 0;
			rect.right = 800;
			rect.bottom = 600;

			_rect = rect;
		}

		

		Vertex2D quadData[6] = {
			{glm::vec2(-1.0f, -1.0f), glm::vec2(0.0f, 1.0f)},
			{glm::vec2(1.0f, 1.0f), glm::vec2(1.0f, 0.0f)},
			{glm::vec2(-1.0f, 1.0f), glm::vec2(0.0f, 0.0f)},
			{glm::vec2(-1.0f, -1.0f), glm::vec2(0.0f, 1.0f)},
			{glm::vec2(1.0f, -1.0f), glm::vec2(1.0f, 1.0f)},
			{glm::vec2(1.0f, 1.0f), glm::vec2(1.0f, 0.0f)},
		};

		BufferInfo quadInfo = {};
		quadInfo.m_data = quadData;
		quadInfo.m_flag = BindFlag::VERTEX_BIT;
		quadInfo.m_size = sizeof(Vertex2D) * 6;
		quadInfo.m_stide = sizeof(Vertex2D);
		quadInfo.m_usageFlag = UsageFlag::DEFAULT;

		RenderFunc::CreateBuffer(&quadBuffer, quadInfo);

		main_pass.Init(this);
		custom_pass.Init(this);
		gbuffer_pass.Init(this);
		lighting_pass.Init(this);

		return result;
	}

	void Renderer::Update(float deltaTime)
	{

	}

	bool Renderer::BeginFrame()
	{
		bool result = RenderFunc::BeginFrame(&g_device, &_swapChain);


		return result;
	}

	void Renderer::BeginScene()
	{
	}

	void Renderer::EndScene()
	{
	}

	void Renderer::EndFrame()
	{
		RenderFunc::EndFrame(&g_device);
	}


	void Renderer::Start()
	{

		// Temp------------------------------------------------------------------------------------------------


		EventsSystem::get_instance()->AddEventListener(EventType::TRC_KEY_RELEASED, BIND_EVENT_FN(Renderer::OnEvent));
		EventsSystem::get_instance()->AddEventListener(EventType::TRC_WND_RESIZE, BIND_EVENT_FN(Renderer::OnEvent));
		EventsSystem::get_instance()->AddEventListener(EventType::TRC_KEY_PRESSED, BIND_EVENT_FN(Renderer::OnEvent));
		EventsSystem::get_instance()->AddEventListener(EventType::TRC_WND_CLOSE, BIND_EVENT_FN(Renderer::OnEvent));
		render_mode = {};

		sky_pipeline = ResourceSystem::get_instance()->GetDefaultPipeline("skybox");

		lights[0].position = { 0.3597f, -0.4932f, 0.7943f, 0.0f };
		lights[0].direction = { 0.3597f, -0.4932f, 0.7943f, 0.0f };
		lights[0].color = { 0.8f, 0.8f, 0.8f, 1.0f };
		lights[0].params1 = { 1.0f, 0.467f, 0.896f, 0.0f };
		lights[0].params2 = { 0.0f, 0.0f, 0.0f, 0.0f };

		lights[1].position = { 0.0f, 2.5f, 2.0f, 0.0f };
		lights[1].direction = { -0.3597f, 0.4932f, -0.7943f, 0.0f };
		lights[1].color = { 0.37f, 0.65f, 0.66f, 1.0f };
		lights[1].params1 = { 1.0f, 0.022f, 0.0019f, 0.0f };
		lights[1].params2 = { 0.0f, 0.0f, 0.0f, 0.0f };

		lights[2].position = { _camera->GetPosition(), 0.0f };
		lights[2].direction = { _camera->GetLookDir(), 0.0f };
		lights[2].color = { 0.6f, 0.8f, 0.0f, 1.0f };
		lights[2].params1 = { 1.0f, 0.07f, 0.017f, 0.939f };
		lights[2].params2 = { 0.866f, 0.0f, 0.0f, 0.0f };

		{

			TextureDesc depth = {};
			depth.m_addressModeU = depth.m_addressModeV = depth.m_addressModeW = AddressMode::REPEAT;
			depth.m_attachmentType = AttachmentType::DEPTH;
			depth.m_flag = BindFlag::DEPTH_STENCIL_BIT;
			depth.m_format = Format::D32_SFLOAT_S8_SUINT;
			depth.m_width = 800;
			depth.m_height = 600;
			depth.m_minFilterMode = depth.m_magFilterMode = FilterMode::LINEAR;
			depth.m_mipLevels = depth.m_numLayers = 1;
			depth.m_usage = UsageFlag::DEFAULT;

			TextureDesc color = {};
			color.m_addressModeU = color.m_addressModeV = color.m_addressModeW = AddressMode::REPEAT;
			color.m_attachmentType = AttachmentType::COLOR;
			color.m_flag = BindFlag::RENDER_TARGET_BIT;
			color.m_format = Format::R8G8B8A8_SRBG;
			color.m_width = 800;
			color.m_height = 600;
			color.m_minFilterMode = color.m_magFilterMode = FilterMode::LINEAR;
			color.m_mipLevels = color.m_numLayers = 1;
			color.m_usage = UsageFlag::DEFAULT;

			TextureDesc gPos = {};
			gPos.m_addressModeU = color.m_addressModeV = color.m_addressModeW = AddressMode::REPEAT;
			gPos.m_attachmentType = AttachmentType::COLOR;
			gPos.m_flag = BindFlag::RENDER_TARGET_BIT;
			gPos.m_format = Format::R16G16B16A16_FLOAT;
			gPos.m_width = 800;
			gPos.m_height = 600;
			gPos.m_minFilterMode = color.m_magFilterMode = FilterMode::LINEAR;
			gPos.m_mipLevels = color.m_numLayers = 1;
			gPos.m_usage = UsageFlag::DEFAULT;

			TextureDesc gNorm = {};
			gNorm.m_addressModeU = color.m_addressModeV = color.m_addressModeW = AddressMode::REPEAT;
			gNorm.m_attachmentType = AttachmentType::COLOR;
			gNorm.m_flag = BindFlag::RENDER_TARGET_BIT;
			gNorm.m_format = Format::R16G16B16A16_FLOAT;
			gNorm.m_width = 800;
			gNorm.m_height = 600;
			gNorm.m_minFilterMode = color.m_magFilterMode = FilterMode::LINEAR;
			gNorm.m_mipLevels = color.m_numLayers = 1;
			gNorm.m_usage = UsageFlag::DEFAULT;

			TextureDesc gColor = {};
			gColor.m_addressModeU = color.m_addressModeV = color.m_addressModeW = AddressMode::REPEAT;
			gColor.m_attachmentType = AttachmentType::COLOR;
			gColor.m_flag = BindFlag::RENDER_TARGET_BIT;
			gColor.m_format = Format::R16G16B16A16_FLOAT;
			gColor.m_width = 800;
			gColor.m_height = 600;
			gColor.m_minFilterMode = color.m_magFilterMode = FilterMode::LINEAR;
			gColor.m_mipLevels = color.m_numLayers = 1;
			gColor.m_usage = UsageFlag::DEFAULT;

			test_graph.AddTextureResource("color", color);
			test_graph.AddTextureResource("depth", depth);
			test_graph.AddTextureResource("gPosition", gPos);
			test_graph.AddTextureResource("gNormal", gNorm);
			test_graph.AddTextureResource("gColor", gColor);
			test_graph.AddSwapchainResource("swapchain", &_swapChain);

			//RenderPassPacket main_pass_input = {};
			//main_pass_input.outputs[0] = test_graph.FindResourceIndex("swapchain");
			//main_pass_input.outputs[1] = test_graph.FindResourceIndex("depth");
			//
			//RenderPassPacket custom_pass_packet = {};
			//custom_pass_packet.inputs[0] = test_graph.FindResourceIndex("color");
			//custom_pass_packet.outputs[0] = test_graph.FindResourceIndex("swapchain");


			//custom_pass.Setup(&test_graph, custom_pass_packet);
			//main_pass.Setup(&test_graph, main_pass_input);

			RenderPassPacket gbufferData = {};
			gbufferData.outputs[0] = test_graph.FindResourceIndex("gPosition");
			gbufferData.outputs[1] = test_graph.FindResourceIndex("gNormal");
			gbufferData.outputs[2] = test_graph.FindResourceIndex("gColor");
			gbufferData.outputs[3] = test_graph.FindResourceIndex("depth");

			RenderPassPacket lightingData = {};
			lightingData.inputs[0] = test_graph.FindResourceIndex("gPosition");
			lightingData.inputs[1] = test_graph.FindResourceIndex("gNormal");
			lightingData.inputs[2] = test_graph.FindResourceIndex("gColor");
			lightingData.outputs[0] = test_graph.FindResourceIndex("swapchain");

			gbuffer_pass.Setup(&test_graph, gbufferData);
			lighting_pass.Setup(&test_graph, lightingData);



			test_graph.SetFinalResourceOutput("swapchain");
			test_graph.SetRenderer(this);
			test_graph.Compile();

		}

		light_data = { 1, 1, 1, 0 };

		//---------------------------------------------------------------------------------------------

	}

	void Renderer::End()
	{

		// Temp-----------------------------

		RenderFunc::DestroyBuffer(&quadBuffer);

		test_graph.Destroy();
		sky_pipeline.~Ref();
		
		lighting_pass.ShutDown();
		custom_pass.ShutDown();
		gbuffer_pass.ShutDown();
		main_pass.ShutDown();
		delete _camera;

		RenderFunc::DestroySwapchain(&_swapChain);		
		//----------------------------------

	}
	void Renderer::ShutDown()
	{

		RenderFunc::DestroyDevice(&g_device);
		RenderFunc::DestroyContext(&g_context);

	}

	void Renderer::OnEvent(Event* p_event)
	{

		switch (p_event->m_type)
		{
		case trace::EventType::TRC_KEY_PRESSED:
		{
			KeyPressed* press = reinterpret_cast<KeyPressed*>(p_event);
			break;
		}
		case trace::EventType::TRC_WND_CLOSE:
		{
			//trace::WindowClose* wnd_close = reinterpret_cast<trace::WindowClose*>(p_event);

			

			break;
		}
		case trace::EventType::TRC_KEY_RELEASED:
		{
			KeyReleased* release = reinterpret_cast<KeyReleased*>(p_event);

			if (release->m_keycode == Keys::KEY_1)
			{
				render_mode.x = 1;
			}
			else if (release->m_keycode == Keys::KEY_2)
			{
				render_mode.x = 2;
			}
			else if (release->m_keycode == Keys::KEY_3)
			{
				render_mode.x = 3;
			}
			else if (release->m_keycode == Keys::KEY_4)
			{
				render_mode.x = 4;
			}
			else if (release->m_keycode == Keys::KEY_0)
			{
				render_mode.x = 0;
			}
			else if (release->m_keycode == Keys::KEY_5)
			{
				render_mode.x = 5;
			}
			else if (release->m_keycode == Keys::KEY_8)
			{
				render_mode.w = 8;
			}
			else if (release->m_keycode == Keys::KEY_9)
			{
				render_mode.w = 9;
			}
			else if (release->m_keycode == Keys::KEY_C)
			{
				TRC_DEBUG(
					"Position: x:{}, y:{}, z:{}\n \tLook Direction: x:{}, y:{}, z:{}", 
					_camera->GetPosition().x, 
					_camera->GetPosition().y, 
					_camera->GetPosition().z, 
					_camera->GetLookDir().x,
					_camera->GetLookDir().y,
					_camera->GetLookDir().z
				);
			}


			break;
		}

		case EventType::TRC_WND_RESIZE:
		{
			WindowResize* wnd = reinterpret_cast<WindowResize*>(p_event);
			RenderFunc::ResizeSwapchain(&_swapChain, wnd->m_width, wnd->m_height);
			float width = static_cast<float>(wnd->m_width);
			float height = static_cast<float>(wnd->m_height);
			test_graph.Resize(wnd->m_width, wnd->m_height);


			_viewPort.width = wnd->m_width;
			_viewPort.height = wnd->m_height;

			_rect.right = wnd->m_width;
			_rect.bottom = wnd->m_height;

			if (wnd->m_width == 0 || wnd->m_height == 0)
				break;

			_camera->SetAspectRatio(((float)wnd->m_width / (float)wnd->m_height));

			break;
		}

		}

	}

	GRenderPass* Renderer::GetRenderPass(RENDERPASS render_pass)
	{
		return &_renderPass[render_pass];
	}

	void Renderer::Render(float deltaTime)
	{

		if (BeginFrame())
		{
			//Temp=====================
			_camera->Update(deltaTime);
			//=========================

			if (render_mode.w == 8)
			{
				glm::vec3 light_pos = glm::vec3(lights[2].position);
				glm::vec3 light_dir = glm::vec3(lights[2].direction);
				glm::vec3 cam_pos = _camera->GetPosition();
				glm::vec3 cam_dir = _camera->GetLookDir();
				if (light_pos != cam_pos)
				{

					lights[2].position.x = lerp(light_pos.x, cam_pos.x, deltaTime);
					lights[2].position.y = lerp(light_pos.y, cam_pos.y, deltaTime);
					lights[2].position.z = lerp(light_pos.z, cam_pos.z, deltaTime);
				}
				if (light_dir != cam_dir)
				{
					lights[2].direction.x = lerp(light_dir.x, cam_dir.x, deltaTime);
					lights[2].direction.y = lerp(light_dir.y, cam_dir.y, deltaTime);
					lights[2].direction.z = lerp(light_dir.z, cam_dir.z, deltaTime);
				}
			}
			
			test_graph.Execute();

			EndFrame();
			RenderFunc::PresentSwapchain(&_swapChain);
		}
		m_listCount = 0;
	}

	void Renderer::DrawQuad()
	{
		RenderFunc::BindVertexBuffer(&g_device, &quadBuffer);
		RenderFunc::Draw(&g_device, 0, 6);
	}


	void Renderer::draw_mesh(CommandParams params)
	{
		Mesh* mesh = (Mesh*)params.ptrs[0];

		SceneGlobals scene_data = {};
		scene_data.view = _camera->GetViewMatrix();
		scene_data.view_position = _camera->GetPosition();
		glm::mat4* M_model = (glm::mat4*)(&params.data);
		scene_data.projection = _camera->GetProjectionMatix();


		

		for (Ref<Model> _model : mesh->GetModels())
		{
			Ref<MaterialInstance> _mi = _model->m_matInstance;
			Ref<GPipeline> sp = _mi->GetRenderPipline();

			RenderFunc::OnDrawStart(&g_device, sp.get());
			RenderFunc::SetPipelineData(sp.get(), "projection", ShaderResourceStage::RESOURCE_STAGE_GLOBAL, &scene_data.projection, sizeof(glm::mat4));
			RenderFunc::SetPipelineData(sp.get(), "view", ShaderResourceStage::RESOURCE_STAGE_GLOBAL, &scene_data.view, sizeof(glm::mat4));
			RenderFunc::SetPipelineData(sp.get(), "view_position", ShaderResourceStage::RESOURCE_STAGE_GLOBAL, &scene_data.view_position, sizeof(glm::vec3));
			RenderFunc::SetPipelineData(sp.get(), "light_data", ShaderResourceStage::RESOURCE_STAGE_GLOBAL, &light_data, sizeof(glm::ivec4));
			RenderFunc::SetPipelineData(sp.get(), "model", ShaderResourceStage::RESOURCE_STAGE_LOCAL, M_model, sizeof(glm::mat4));
			RenderFunc::ApplyMaterial(_mi.get());
			RenderFunc::BindPipeline(&g_device, sp.get());
			RenderFunc::BindVertexBuffer(&g_device, _model->GetVertexBuffer());
			RenderFunc::BindIndexBuffer(&g_device, _model->GetIndexBuffer());

			RenderFunc::DrawIndexed(&g_device, 0, _model->GetIndexCount());
			RenderFunc::OnDrawEnd(&g_device, sp.get());
		}

	}

	void Renderer::draw_skybox(CommandParams params)
	{
		SkyBox* sky_box = (SkyBox*)params.ptrs[0];

		SceneGlobals scene_data = {};
		scene_data.view = _camera->GetViewMatrix();
		scene_data.view_position = _camera->GetPosition();
		scene_data.projection = _camera->GetProjectionMatix();

		Ref<GPipeline> sp = sky_pipeline;
		RenderFunc::OnDrawStart(&g_device, sp.get());

		RenderFunc::SetPipelineTextureData(
			sp.get(),
			"CubeMap",
			ShaderResourceStage::RESOURCE_STAGE_GLOBAL,
			sky_box->GetCubeMap()
		);
		RenderFunc::SetPipelineData(sp.get(), "projection", ShaderResourceStage::RESOURCE_STAGE_GLOBAL, &scene_data.projection, sizeof(glm::mat4));
		RenderFunc::SetPipelineData(sp.get(), "view", ShaderResourceStage::RESOURCE_STAGE_GLOBAL, &scene_data.view, sizeof(glm::mat4));
		RenderFunc::SetPipelineData(sp.get(), "view_position", ShaderResourceStage::RESOURCE_STAGE_GLOBAL, &scene_data.view_position, sizeof(glm::vec3));
		RenderFunc::BindPipeline_(sp.get());

		Ref<Model> mod = sky_box->GetCube()->GetModels()[0];
		RenderFunc::BindPipeline(&g_device, sp.get());
		RenderFunc::BindVertexBuffer(&g_device, mod->GetVertexBuffer());
		RenderFunc::BindIndexBuffer(&g_device, mod->GetIndexBuffer());
		
		RenderFunc::DrawIndexed(&g_device, 0, mod->GetIndexCount());
		RenderFunc::OnDrawEnd(&g_device, sp.get());
	}

	void Renderer::DrawMesh(CommandList& cmd_list, Ref<Mesh> mesh, glm::mat4 model)
	{
		Command cmd;
		cmd.params.ptrs[0] = mesh.get();
		cmd.func = BIND_RENDER_COMMAND_FN(Renderer::draw_mesh);
		memcpy(cmd.params.data, &model, sizeof(glm::mat4));
		cmd_list._commands.push_back(cmd);
	}

	void Renderer::DrawSky(CommandList& cmd_list, SkyBox* sky)
	{

		Command cmd;
		cmd.params.ptrs[0] = sky;
		cmd.func = BIND_RENDER_COMMAND_FN(Renderer::draw_skybox);
		cmd_list._commands.push_back(cmd);
	}

	CommandList Renderer::BeginCommandList()
	{
		return {};
	}

	void Renderer::SubmitCommandList(CommandList& list)
	{
		if (m_listCount >= m_cmdList.size())
			m_cmdList.resize(m_listCount * 2);
		m_cmdList[m_listCount++] = list;
	}

	Renderer* Renderer::get_instance()
	{
		if (s_instance == nullptr)
		{
			s_instance = new Renderer();
		}
		return s_instance;
	}



}