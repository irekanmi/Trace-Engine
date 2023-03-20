#include <pch.h>

#include "Renderer.h"
#include "GContext.h"
#include "core/Enums.h"
#include "core/io/Logging.h"
#include "core/platform/OpenGL/OpenGLContext.h"
#include "core/platform/OpenGL/OpenGLDevice.h"
#include "core/platform/Vulkan/VKContext.h"
#include "core/platform/Vulkan/VKDevice.h"
#include "resource/ResourceSystem.h"
#include "render/PerspectiveCamera.h"
#include "core/events/EventsSystem.h"
#include "core/Enums.h"

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
		bool result = false;

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
			m_context = new OpenGLContext();
			if (m_context == nullptr)
			{
				TRC_ERROR(" Failed to create a graphics context ");
				return false;
			}
			m_context->Init();

			// TODO: Implement OpenGl Device
			//m_device = new OpenGLDevice();
			result = m_device->Init();
			break;

		}

		case RenderAPI::Vulkan:
		{

			m_context = new VKContext();
			m_context->Init();

			m_device = new VKDevice();
			result = m_device->Init();

			break;
		}

		default:
			TRC_ASSERT(false, "Graphics context can not be null");
			return result;
		}

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
		pass_desc.stencil_value = 0.0f;


		_renderPass[RENDERPASS::MAIN_PASS] = GRenderPass::Create_(pass_desc);

		_swapChain = GSwapchain::Create_(m_device, m_context);

		GTexture* attachments[] = {
			_swapChain->GetBackColorBuffer(),
			_swapChain->GetBackDepthBuffer()
		};

		_framebuffer = GFramebuffer::Create_(
			2,
			attachments,
			_renderPass[RENDERPASS::MAIN_PASS],
			800,
			600,
			1,
			_swapChain
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

		return result;
	}

	void Renderer::Update(float deltaTime)
	{

		// Temp--------------------
		_camera->Update(deltaTime);
		//-------------------------
		

		m_context->Update(deltaTime);
	}

	bool Renderer::BeginFrame()
	{
		bool result = m_device->BeginFrame(_swapChain);
		//if (result)
		//{
		//	m_device->BeginRenderPass(_renderPass, _framebuffer);
		//	m_device->BindViewport(_viewPort);
		//	m_device->BindRect(_rect);
		//}


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
		//m_device->EndRenderPass(_renderPass);
		m_device->EndFrame();
		//_swapChain->Present();
	}


	void Renderer::Start()
	{

		// Temp------------------------------------------------------------------------------------------------


		EventsSystem::s_instance->AddEventListener(EventType::TRC_KEY_PRESSED, BIND_EVENT_FN(Renderer::OnEvent));
		EventsSystem::s_instance->AddEventListener(EventType::TRC_KEY_RELEASED, BIND_EVENT_FN(Renderer::OnEvent));
		EventsSystem::s_instance->AddEventListener(EventType::TRC_WND_RESIZE, BIND_EVENT_FN(Renderer::OnEvent));
		EventsSystem::s_instance->AddEventListener(EventType::TRC_WND_CLOSE, BIND_EVENT_FN(Renderer::OnEvent));

		_pipeline = ResourceSystem::get_instance()->GetDefaultPipeline("standard");
		{
		trace::FileHandle vert_shad;
		trace::FileHandle frag_shad;

		std::string vert_src;
		std::string frag_src;

		if (!trace::FileSystem::open_file("../assets/shaders/trace_core.shader.vert.glsl", trace::FileMode::READ, vert_shad))
		{
			TRC_ERROR("Failed to open file");
		}

		if (!trace::FileSystem::open_file("../assets/shaders/reflect.frag.glsl", trace::FileMode::READ, frag_shad))
		{
			TRC_ERROR("Failed to open file");
		}

		trace::FileSystem::read_all_lines(vert_shad, vert_src);
		trace::FileSystem::read_all_lines(frag_shad, frag_src);

		std::cout << vert_src;
		std::cout << frag_src;

		trace::FileSystem::close_file(vert_shad);
		trace::FileSystem::close_file(frag_shad);

		VertShader = GShader::Create_(vert_src, ShaderStage::VERTEX_SHADER);
		FragShader = GShader::Create_(frag_src, ShaderStage::PIXEL_SHADER);

		ColorBlendState blend_state;
		blend_state.alpha_to_blend_coverage = true;

		DepthStencilState depth_stenc_state;
		depth_stenc_state.depth_test_enable = true;
		depth_stenc_state.maxDepth = 1.0f;
		depth_stenc_state.minDepth = 0.0f;
		depth_stenc_state.stencil_test_enable = false;

		InputLayout _layout = Vertex::get_input_layout();

		RaterizerState raterizer_state;
		raterizer_state.cull_mode = CullMode::BACK;
		raterizer_state.fill_mode = FillMode::SOLID;

		Viewport vp = {};
		vp.minDepth = 0.0f;
		vp.maxDepth = 1.0f;
		vp.width = 800;
		vp.height = 600;
		vp.x = 0;
		vp.y = 0;

		_viewPort = vp;

		Rect2D rect;
		rect.top = rect.left = 0;
		rect.right = 800;
		rect.bottom = 600;

		_rect = rect;

		ShaderResourceBinding scene_data;
		scene_data.shader_stage = ShaderStage::VERTEX_SHADER;
		scene_data.resource_stage = ShaderResourceStage::RESOURCE_STAGE_GLOBAL;
		scene_data.resource_size = sizeof(SceneGlobals);
		scene_data.resource_type = ShaderResourceType::SHADER_RESOURCE_TYPE_UNIFORM_BUFFER;
		scene_data.resource_name = "scene_globals";
		scene_data.count = 1;
		scene_data.index = 0;
		scene_data.slot = 0;

		ShaderResourceBinding scene_tex;
		scene_tex.shader_stage = ShaderStage::PIXEL_SHADER;
		scene_tex.resource_stage = ShaderResourceStage::RESOURCE_STAGE_GLOBAL;
		scene_tex.resource_size = 0;
		scene_tex.resource_type = ShaderResourceType::SHADER_RESOURCE_TYPE_COMBINED_SAMPLER;
		scene_tex.resource_name = "CubeMap";
		scene_tex.count = 1;
		scene_tex.index = 0;
		scene_tex.slot = 1;


		ShaderResourceBinding local_data;
		local_data.shader_stage = ShaderStage::VERTEX_SHADER;
		local_data.resource_stage = ShaderResourceStage::RESOURCE_STAGE_LOCAL;
		local_data.resource_size = sizeof(glm::mat4);
		local_data.resource_type = ShaderResourceType::SHADER_RESOURCE_TYPE_UNIFORM_BUFFER;
		local_data.resource_name = "local_data";
		local_data.count = 1;
		local_data.index = 0;
		local_data.slot = 3;

		ShaderResourceBinding normal_data;
		normal_data.shader_stage = ShaderStage::PIXEL_SHADER;
		normal_data.resource_stage = ShaderResourceStage::RESOURCE_STAGE_INSTANCE;
		normal_data.resource_size = 0;
		normal_data.resource_type = ShaderResourceType::SHADER_RESOURCE_TYPE_COMBINED_SAMPLER;
		normal_data.resource_name = "normal_map";
		normal_data.count = 1;
		normal_data.index = 0;
		normal_data.slot = 1;



		std::vector<ShaderResourceBinding> scene = {
			scene_data,
			scene_tex,
			local_data,
			normal_data
		};

		PipelineStateDesc desc;
		desc.blend_state = blend_state;
		desc.depth_sten_state = depth_stenc_state;
		desc.input_layout = _layout;
		desc.pixel_shader = FragShader;
		desc.rateriser_state = raterizer_state;
		desc.topology = PrimitiveTopology::TRIANGLE_LIST;
		desc.vertex_shader = VertShader;
		desc.view_port = vp;
		desc.resource_bindings_count = 4;
		desc.resource_bindings = scene;
		desc.render_pass = _renderPass[RENDERPASS::MAIN_PASS];
		desc.subpass_index = 0;

		reflect_pipeline = GPipeline::Create_(desc);
		if (reflect_pipeline)
		{
			if (!reflect_pipeline->Initialize())
			{
				TRC_ERROR("Failed to initialize pipeline");
			}
			delete VertShader;
			delete FragShader;
		}

		}
		skybox_pipeline = ResourceSystem::get_instance()->GetDefaultPipeline("skybox");
		

		


		
		std::vector<std::string> cube_maps = {
			"sky_right.jpg",
			"sky_left.jpg",
			"sky_top.jpg",
			"sky_bottom.jpg",
			"sky_front.jpg",
			"sky_back.jpg"
		};

		TextureDesc cubeMapDesc;
		cubeMapDesc.m_image_type = ImageType::CUBE_MAP;
		cubeMapDesc.m_addressModeU = cubeMapDesc.m_addressModeV = cubeMapDesc.m_addressModeW = AddressMode::CLAMP_TO_EDGE;
		cubeMapDesc.m_numLayers = 6;
		cubeMapDesc.m_format = Format::R8G8B8A8_SRBG;
		cubeMapDesc.m_minFilterMode = cubeMapDesc.m_magFilterMode = FilterMode::LINEAR;
		cubeMapDesc.m_usage = UsageFlag::DEFAULT;
		cubeMapDesc.m_flag = BindFlag::SHADER_RESOURCE_BIT;

		Texture_Ref temp = ResourceSystem::get_instance()->LoadTexture(cube_maps, cubeMapDesc, "cube_map");

		_sky = SkyBox(temp);

		//---------------------------------------------------------------------------------------------

	}

	void Renderer::End()
	{

		// Temp-----------------------------
		
		delete _swapChain;
		delete _framebuffer;
		delete _renderPass[RENDERPASS::MAIN_PASS];

		_pipeline.~Ref();
		skybox_pipeline.~Ref();
		reflect_pipeline->Shutdown();
		delete reflect_pipeline;

		_sky.~SkyBox();
		
		delete _camera;
		
		//----------------------------------

	}
	void Renderer::ShutDown()
	{

		

		m_device->ShutDown();
		m_context->ShutDown();

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

			break;
		}

		case EventType::TRC_WND_RESIZE:
		{
			WindowResize* wnd = reinterpret_cast<WindowResize*>(p_event);
			_renderPass[RENDERPASS::MAIN_PASS]->m_desc.render_area = { 0.0f, 0.0f, (float)wnd->m_width, (float)wnd->m_height };

			_swapChain->Resize(wnd->m_width, wnd->m_height);

			


			_viewPort.width = wnd->m_width;
			_viewPort.height = wnd->m_height;

			_rect.right = wnd->m_width;
			_rect.bottom = wnd->m_height;

			if (wnd->m_width == 0 || wnd->m_height == 0)
				break;

			delete _framebuffer;
			GTexture* attachments[] = {
			_swapChain->GetBackColorBuffer(),
			_swapChain->GetBackDepthBuffer()
			};

			_framebuffer = GFramebuffer::Create_(
				2,
				attachments,
				_renderPass[RENDERPASS::MAIN_PASS],
				wnd->m_width,
				wnd->m_height,
				1,
				_swapChain
			);
			_camera->SetAspectRatio(((float)wnd->m_width / (float)wnd->m_height));
			break;
		}

		}

	}

	GRenderPass* Renderer::GetRenderPass(RENDERPASS render_pass)
	{
		return _renderPass[render_pass];
	}

	void Renderer::Render(float deltaTime)
	{

		if (BeginFrame())
		{
			//Temp=====================
			_camera->Update(deltaTime);
			//=========================
			m_device->BeginRenderPass(_renderPass[RENDERPASS::MAIN_PASS], _framebuffer);
			m_device->BindViewport(_viewPort);
			m_device->BindRect(_rect);


			for (uint32_t i = 0; i < m_listCount; i++)
			{
				for (Command& cmd : m_cmdList[i]._commands)
				{
					cmd.func(cmd.params);
				}
			}

			m_device->EndRenderPass(_renderPass[RENDERPASS::MAIN_PASS]);
			EndFrame();
			_swapChain->Present();
		}
		m_listCount = 0;
	}


	void Renderer::Draw_Mesh(CommandParams params)
	{
		Mesh* mesh = (Mesh*)params.ptrs[0];

		Material* _mat;
		SceneGlobals scene_data = {};
		scene_data.view = _camera->GetViewMatrix();
		scene_data.view_position = _camera->GetPosition();
		static float x = 70.0f;
		static float r = -0.5f;

		x += r;
		//glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -1.0f));
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::scale(model, glm::vec3(0.15f));
		//model = glm::rotate(model, glm::radians(x), glm::vec3(0.0f, 1.0f, 0.0f));

		scene_data.projection = _camera->GetProjectionMatix();


		

		for (Ref<Model> _model : mesh->GetModels())
		{
			Ref<MaterialInstance> _mi = _model->m_matInstance;
			Ref<GPipeline> sp = _mi->GetRenderPipline();

			//reflect_pipeline->SetData("scene_globals",
			//	ShaderResourceStage::RESOURCE_STAGE_GLOBAL,
			//	&scene_data,
			//	sizeof(SceneGlobals)
			//);
			//reflect_pipeline->SetData("local_data",
			//	ShaderResourceStage::RESOURCE_STAGE_LOCAL,
			//	&model,
			//	sizeof(glm::mat4)
			//);
			//reflect_pipeline->SetTextureData(
			//	"CubeMap",
			//	ShaderResourceStage::RESOURCE_STAGE_GLOBAL,
			//	_sky.GetCubeMap()
			//);
			//
			//if (_mat->m_normalMap.is_valid())
			//{
			//	reflect_pipeline->SetTextureData("normal_map", ShaderResourceStage::RESOURCE_STAGE_INSTANCE, _mat->m_normalMap.get());
			//}

			sp->SetData("scene_globals", ShaderResourceStage::RESOURCE_STAGE_GLOBAL, &scene_data, sizeof(SceneGlobals));
			sp->SetData("local_data", ShaderResourceStage::RESOURCE_STAGE_LOCAL, &model, sizeof(glm::mat4));
			m_device->BindPipeline(sp.get());
			_mi->Apply();
			m_device->BindVertexBuffer(_model->GetVertexBuffer());
			m_device->BindIndexBuffer(_model->GetIndexBuffer());

			m_device->DrawIndexed(0, _model->GetIndexCount());
		}

	}

	void Renderer::DrawSkyBox(CommandParams params)
	{
		SkyBox* sky_box = (SkyBox*)params.ptrs[0];

		SceneGlobals scene_data = {};
		scene_data.view = _camera->GetViewMatrix();
		scene_data.view_position = _camera->GetPosition();
		scene_data.projection = _camera->GetProjectionMatix();


		skybox_pipeline->SetTextureData(
			"CubeMap",
			ShaderResourceStage::RESOURCE_STAGE_GLOBAL,
			sky_box->GetCubeMap()
		);
		skybox_pipeline->SetData(
			"scene_globals",
			ShaderResourceStage::RESOURCE_STAGE_GLOBAL,
			&scene_data,
			sizeof(SceneGlobals)
		);
		skybox_pipeline->Bind();
		m_device->BindPipeline(skybox_pipeline.get());
		m_device->BindVertexBuffer(sky_box->GetCube()->GetModels()[0]->GetVertexBuffer());
		m_device->BindIndexBuffer(sky_box->GetCube()->GetModels()[0]->GetIndexBuffer());
		
		m_device->DrawIndexed(0, sky_box->GetCube()->GetModels()[0]->GetIndexCount());
	}

	void Renderer::DrawMesh(CommandList& cmd_list, Ref<Mesh> mesh)
	{
		Command cmd;
		cmd.params.ptrs[0] = mesh.get();
		cmd.func = BIND_RENDER_COMMAND_FN(Renderer::Draw_Mesh);
		cmd_list._commands.push_back(cmd);
	}

	void Renderer::DrawSky(CommandList& cmd_list, SkyBox* sky)
	{

		Command cmd;
		cmd.params.ptrs[0] = sky;
		cmd.func = BIND_RENDER_COMMAND_FN(Renderer::DrawSkyBox);
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