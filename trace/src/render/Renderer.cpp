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


namespace trace {

	Renderer* Renderer::s_instance = nullptr;
	RenderAPI Renderer::s_api = RenderAPI::None;

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
	
		GContext::s_API = api;
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

		return result;
	}

	void Renderer::Update(float deltaTime)
	{

		// Temp--------------------
		_camera->Update(0.0f);
		//-------------------------
		

			if (m_device->BeginFrame(_swapChain))
			{
				SceneGlobals scene_data = {};
				scene_data.view = _camera->GetViewMatrix();
				scene_data.projection = _camera->GetProjectionMatix();

				m_device->BeginRenderPass(_renderPass, _framebuffer);
				m_device->BindViewport(_viewPort);
				m_device->BindRect(_rect);


				_pipeline0->SetData("scene_globals", ShaderResourceStage::RESOURCE_STAGE_GLOBAL, &scene_data, sizeof(SceneGlobals));
				_pipeline0->SetTextureData("testing", ShaderResourceStage::RESOURCE_STAGE_GLOBAL, _texture_ref.get());

				m_device->BindPipeline(_pipeline0);
				m_device->BindVertexBuffer(VertexBuffer);
				m_device->BindIndexBuffer(IndexBuffer);

				m_device->DrawIndexed(0, 6);
				m_device->EndRenderPass(_renderPass);


				m_device->EndFrame();
				_swapChain->Present();
			}


		m_context->Update(deltaTime);
	}

	bool Renderer::BeginFrame()
	{
		 return m_device->BeginFrame(_swapChain);
	}

	void Renderer::BeginScene()
	{
	}

	void Renderer::EndScene()
	{
	}

	void Renderer::EndFrame()
	{
		m_device->EndFrame();
	}

	void Renderer::UsePipeline(GPipeline* pipeline)
	{
		m_device->m_pipeline = pipeline;
	}

	void Renderer::Start()
	{

		// Temp------------------------------------------------------------------------------------------------


		EventsSystem::s_instance->AddEventListener(EventType::TRC_KEY_PRESSED, BIND_EVENT_FN(Renderer::OnEvent));
		EventsSystem::s_instance->AddEventListener(EventType::TRC_KEY_RELEASED, BIND_EVENT_FN(Renderer::OnEvent));
		EventsSystem::s_instance->AddEventListener(EventType::TRC_WND_RESIZE, BIND_EVENT_FN(Renderer::OnEvent));
		EventsSystem::s_instance->AddEventListener(EventType::TRC_WND_CLOSE, BIND_EVENT_FN(Renderer::OnEvent));


		trace::FileHandle vert_shad;
		trace::FileHandle frag_shad;

		std::string vert_src;
		std::string frag_src;

		if (!trace::FileSystem::open_file("../assets/shaders/trace_core.shader.vert.glsl", trace::FileMode::READ, vert_shad))
		{
			TRC_ERROR("Failed to open file");
		}

		if (!trace::FileSystem::open_file("../assets/shaders/trace_core.shader.frag.glsl", trace::FileMode::READ, frag_shad))
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
		blend_state.alpha_to_blend_coverage = false;

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
		scene_tex.resource_name = "testing";
		scene_tex.count = 1;
		scene_tex.index = 0;
		scene_tex.slot = 1;

		ShaderResourceBinding scene[] = {
			scene_data,
			scene_tex
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
		desc.resource_bindings_count = 2;
		desc.resource_bindings = scene;

		//_pipeline = GPipeline::Create_(desc);


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


		_renderPass = GRenderPass::Create_(pass_desc);

		desc.render_pass = _renderPass;
		desc.subpass_index = 0;

		_pipeline0 = GPipeline::Create_(desc);
		if (_pipeline0)
		{
			if (!_pipeline0->Initialize())
			{
				TRC_ERROR("Failed to initialized pipeline");
			}
		}

		_swapChain = GSwapchain::Create_(m_device, m_context);

		GTexture* attachments[] = {
			_swapChain->GetBackColorBuffer(),
			_swapChain->GetBackDepthBuffer()
		};

		_framebuffer = GFramebuffer::Create_(
			2,
			attachments,
			_renderPass,
			800,
			600,
			1,
			_swapChain
		);
		const float scale = 10;


		m_vertices = {
			{ {-0.75f, -0.5f, 0.0f}, { 0.0f, 0.0f}},
			{ {0.75f, -0.5f, 0.0f}, {1.0f, 0.0f} },
			{ {0.75f, 0.5f, 0.0f}, {1.0f, 1.0f} },
			{ { -0.75f, 0.5f, 0.0f }, {0.0f, 1.0f }}
		};

		for (auto& i : m_vertices)
		{
			i.pos *= scale;
		}



		m_indices = {
			0, 1, 2,
			0, 2, 3
		};

		BufferInfo vertex_buffer_info;
		vertex_buffer_info.m_size = m_vertices.size() * sizeof(Vertex);
		vertex_buffer_info.m_stide = sizeof(Vertex);
		vertex_buffer_info.m_usageFlag = UsageFlag::DEFAULT;
		vertex_buffer_info.m_flag = BindFlag::VERTEX_BIT;
		vertex_buffer_info.m_data = m_vertices.data();

		BufferInfo index_buffer_info;
		index_buffer_info.m_size = m_indices.size() * sizeof(uint32_t);
		index_buffer_info.m_stide = sizeof(uint32_t);
		index_buffer_info.m_usageFlag = UsageFlag::DEFAULT;
		index_buffer_info.m_flag = BindFlag::INDEX_BIT;
		index_buffer_info.m_data = m_indices.data();

		VertexBuffer = GBuffer::Create_(vertex_buffer_info);
		IndexBuffer = GBuffer::Create_(index_buffer_info);


		_camera = new PerspectiveCamera(
			glm::vec3(0.0f, 0.0f, 15.0f),
			glm::vec3(0.0f, 0.0f, -1.0f),
			glm::vec3(0.0f, 1.0f, 0.0f),
			((float)800.0f) / ((float)600.0f),
			45.0f,
			0.1f,
			500.0f
		);

		UsePipeline(_pipeline0);

		//		uint32_t dimension = 256;
		//		uint32_t channels = 4;
		//		
		//		unsigned char* pixel = new unsigned char[dimension * dimension * channels];
		//		memset(pixel, 255, dimension * dimension * channels);
		//
		//		for (uint32_t row = 0; row < dimension; row++)
		//		{
		//			for (uint32_t coloumn = 0; coloumn < dimension; coloumn++)
		//			{
		//				uint32_t index = (row * dimension) + coloumn;
		//				uint32_t _idx = index * channels;
		//				if (row % 2)
		//				{
		//					if (coloumn % 2)
		//					{
		//						pixel[_idx + 0] = 0;
		//						pixel[_idx + 1] = 0;
		//					}
		//				}
		//				else
		//				{
		//					if (!(coloumn % 2))
		//					{
		//						pixel[_idx + 0] = 0;
		//						pixel[_idx + 1] = 0;
		//					}
		//				}
		//			}
		//
		//		}
		//



		_texture_ref = ResourceSystem::s_instance->GetTexture("trace_core.texture0.jpg");
		_texture_ref0 = ResourceSystem::s_instance->GetTexture("trace_core.texture0.jpg");
		_texture_ref1 = ResourceSystem::s_instance->GetTexture("trace_core.texture1.jpg");
		_texture_ref2 = ResourceSystem::s_instance->GetTexture("trace_core.texture2.jpg");


		//---------------------------------------------------------------------------------------------

	}

	void Renderer::End()
	{

		// Temp-----------------------------
		
		_texture_ref.~Ref();
		_texture_ref0.~Ref();
		_texture_ref1.~Ref();
		_texture_ref2.~Ref();
		
		
		delete _swapChain;
		delete _framebuffer;
		delete _renderPass;

		_pipeline0->Shutdown();
		delete _pipeline0;

		//delete _pipeline;
		
		delete VertexBuffer;
		delete IndexBuffer;
		
		delete VertShader;
		delete FragShader;
		
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
			//shouldRender = false;
			//
			

			break;
		}
		case trace::EventType::TRC_KEY_RELEASED:
		{
			KeyReleased* release = reinterpret_cast<KeyReleased*>(p_event);
			if (release->m_keycode == Keys::KEY_1)
			{
				_texture_ref = _texture_ref0;
			}
			if (release->m_keycode == Keys::KEY_2)
			{
				_texture_ref = _texture_ref1;
			}
			if (release->m_keycode == Keys::KEY_3)
			{
				_texture_ref = _texture_ref2;
			}
			break;
		}

		case EventType::TRC_WND_RESIZE:
		{
			WindowResize* wnd = reinterpret_cast<WindowResize*>(p_event);
			_renderPass->m_desc.render_area = { 0.0f, 0.0f, (float)wnd->m_width, (float)wnd->m_height };

			_swapChain->Resize(wnd->m_width, wnd->m_height);

			delete _framebuffer;
			GTexture* attachments[] = {
			_swapChain->GetBackColorBuffer(),
			_swapChain->GetBackDepthBuffer()
			};

			_framebuffer = GFramebuffer::Create_(
				2,
				attachments,
				_renderPass,
				wnd->m_width,
				wnd->m_height,
				1,
				_swapChain
			);


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

	Renderer* Renderer::get_instance()
	{
		if (s_instance == nullptr)
		{
			s_instance = new Renderer();
		}
		return s_instance;
	}

	RenderAPI Renderer::get_api()
	{
		return GContext::get_render_api();
	}

}