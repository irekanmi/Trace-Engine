#include <pch.h>

#include "Renderer.h"
#include "Renderutils.h"
#include "GContext.h"
#include "core/Enums.h"
#include "core/io/Logging.h"
#include "core/platform/OpenGL/OpenGLContext.h"
#include "core/platform/OpenGL/OpenGLDevice.h"
#include "core/platform/Vulkan/VKDevice.h"
#include "resource/ResourceSystem.h"
#include "render/PerspectiveCamera.h"
#include "core/events/EventsSystem.h"
#include "core/Enums.h"
#include "GContext.h"

//Temp============
#include "glm/gtc/matrix_transform.hpp"


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
		AttachmentInfo color_attach;
		color_attach.attachmant_index = 0;
		color_attach.attachment_format = Format::R8G8B8A8_SRBG;
		color_attach.initial_format = TextureFormat::UNKNOWN;
		color_attach.final_format = TextureFormat::PRESENT;
		color_attach.is_depth = false;
		color_attach.load_operation = AttachmentLoadOp::LOAD_OP_CLEAR;
		color_attach.store_operation = AttachmentStoreOp::STORE_OP_STORE;


		AttachmentInfo depth_attach;
		depth_attach.attachmant_index = 1;
		depth_attach.attachment_format = Format::D32_SFLOAT_S8_SUINT;
		depth_attach.initial_format = TextureFormat::UNKNOWN;
		depth_attach.final_format = TextureFormat::DEPTH_STENCIL;
		depth_attach.is_depth = true;
		depth_attach.load_operation = AttachmentLoadOp::LOAD_OP_CLEAR;
		depth_attach.store_operation = AttachmentStoreOp::STORE_OP_STORE;

		AttachmentInfo att_infos[] = {
			color_attach,
			depth_attach
		};

		SubPassDescription subpass_desc;
		subpass_desc.attachment_count = 2;
		subpass_desc.attachments = att_infos;

		RenderPassDescription pass_desc;
		pass_desc.subpass_count = 1;
		pass_desc.subpasses = &subpass_desc;
		pass_desc.render_area = { 0, 0, 800, 600 };
		pass_desc.clear_color = { .0f, .01f, 0.015f, 1.0f };
		pass_desc.depth_value = 1.0f;
		pass_desc.stencil_value = 0;


		RenderFunc::CreateRenderPass(&_renderPass[RENDERPASS::MAIN_PASS], pass_desc);
		RenderFunc::CreateSwapchain(&_swapChain, &g_device, &g_context);

		GTexture swapchain_color;
		GTexture swapchain_depth;

		RenderFunc::GetSwapchainColorBuffer(&_swapChain, &swapchain_color);
		RenderFunc::GetSwapchainDepthBuffer(&_swapChain, &swapchain_depth);

		GTexture* attachments[] = {
			&swapchain_color,
			&swapchain_depth
		};


		RenderFunc::CreateFramebuffer(
			&_framebuffer,
			2,
			attachments,
			&_renderPass[RENDERPASS::MAIN_PASS],
			800,
			600,
			1,
			&_swapChain
		);

		_camera = new PerspectiveCamera(
			glm::vec3(0.0f, 0.0f, 3.0f),
			glm::vec3(0.0f, 0.0f, -1.0f),
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

		//---------------------------------------------------------------------------------------------

	}

	void Renderer::End()
	{

		// Temp-----------------------------


		/*sky_pipeline.~Ref();

		delete _camera;*/

		RenderFunc::DestroySwapchain(&_swapChain);
		RenderFunc::DestroyFramebuffer(&_framebuffer);
		RenderFunc::DestroyRenderPass(&_renderPass[RENDERPASS::MAIN_PASS]);
		
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

			break;
		}

		case EventType::TRC_WND_RESIZE:
		{
			WindowResize* wnd = reinterpret_cast<WindowResize*>(p_event);
			RenderFunc::ResizeSwapchain(&_swapChain, wnd->m_width, wnd->m_height);
			


			_viewPort.width = wnd->m_width;
			_viewPort.height = wnd->m_height;

			_rect.right = wnd->m_width;
			_rect.bottom = wnd->m_height;

			if (wnd->m_width == 0 || wnd->m_height == 0)
				break;

			RenderFunc::DestroyFramebuffer(&_framebuffer);
			GTexture swapchain_color;
			GTexture swapchain_depth;

			RenderFunc::GetSwapchainColorBuffer(&_swapChain, &swapchain_color);
			RenderFunc::GetSwapchainDepthBuffer(&_swapChain, &swapchain_depth);

			GTexture* attachments[] = {
				&swapchain_color,
				&swapchain_depth
			};

			RenderFunc::CreateFramebuffer(
				&_framebuffer,
				2,
				attachments,
				&_renderPass[RENDERPASS::MAIN_PASS],
				wnd->m_width,
				wnd->m_height,
				1,
				&_swapChain
			);
			_renderPass[RENDERPASS::MAIN_PASS].m_desc.render_area = { 0.0f, 0.0f, (float)wnd->m_width, (float)wnd->m_height };
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
			RenderFunc::BeginRenderPass(&g_device, &_renderPass[RENDERPASS::MAIN_PASS], &_framebuffer);
			RenderFunc::BindViewport(&g_device, _viewPort);
			RenderFunc::BindRect(&g_device, _rect);


			for (uint32_t i = 0; i < m_listCount; i++)
			{
				for (Command& cmd : m_cmdList[i]._commands)
				{
					cmd.func(cmd.params);
				}
			}

			RenderFunc::EndRenderPass(&g_device, &_renderPass[RENDERPASS::MAIN_PASS]);
			EndFrame();
			RenderFunc::PresentSwapchain(&_swapChain);
		}
		m_listCount = 0;
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

			RenderFunc::SetPipelineData(sp.get(), "projection", ShaderResourceStage::RESOURCE_STAGE_GLOBAL, &scene_data.projection, sizeof(glm::mat4));
			RenderFunc::SetPipelineData(sp.get(), "view", ShaderResourceStage::RESOURCE_STAGE_GLOBAL, &scene_data.view, sizeof(glm::mat4));
			RenderFunc::SetPipelineData(sp.get(), "view_position", ShaderResourceStage::RESOURCE_STAGE_GLOBAL, &scene_data.view_position, sizeof(glm::vec3));
			RenderFunc::SetPipelineData(sp.get(), "model", ShaderResourceStage::RESOURCE_STAGE_LOCAL, M_model, sizeof(glm::mat4));
			RenderFunc::SetPipelineData(sp.get(), "rest", ShaderResourceStage::RESOURCE_STAGE_GLOBAL, &render_mode, sizeof(glm::ivec4));
			RenderFunc::ApplyMaterial(_mi.get());
			RenderFunc::BindPipeline(&g_device, sp.get());
			RenderFunc::BindVertexBuffer(&g_device, _model->GetVertexBuffer());
			RenderFunc::BindIndexBuffer(&g_device, _model->GetIndexBuffer());

			RenderFunc::DrawIndexed(&g_device, 0, _model->GetIndexCount());
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