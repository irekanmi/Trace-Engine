#include <pch.h>

#include "Renderer.h"
#include "Renderutils.h"
#include "GContext.h"
#include "core/Enums.h"
#include "core/io/Logging.h"
#include "resource/ResourceSystem.h"
#include "core/events/EventsSystem.h"
#include "core/Enums.h"
#include "Model.h"
#include "Mesh.h"
#include "SkyBox.h"

//Temp============
#include "glm/gtc/matrix_transform.hpp"
#include "core/Utils.h"
#include "render_graph/FrameData.h"
#include "ShaderParser.h"
#include "GShader.h"


namespace trace {

	Renderer* Renderer::s_instance = nullptr;

	//Temp------------
	static FrameSettings frame_settings = RENDER_DEFAULT | RENDER_HDR | RENDER_BLOOM;
	//----------------

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
			
			RenderFunc::CreateSwapchain(&m_swapChain, &g_device, &g_context);

			_camera = new Camera(CameraType::PERSPECTIVE);
			_camera->SetPosition(glm::vec3(109.72446f, 95.70557f, -10.92075f));
			_camera->SetLookDir(glm::vec3(-0.910028f, -0.4126378f, 0.039738327f));
			_camera->SetUpDir(glm::vec3(0.0f, 1.0f, 0.0f));
			_camera->SetAspectRatio(((float)800.0f) / ((float)600.0f));
			_camera->SetFov(60.0f);
			_camera->SetNear(0.1f);
			_camera->SetFar(1500.0f);

			_viewPort.width = 800.0f;
			_viewPort.height = 600.0f;
			Rect2D rect;
			rect.top = rect.left = 0;
			rect.right = 800;
			rect.bottom = 600;

			_rect = rect;
			m_frameWidth = 800;
			m_frameHeight = 600;
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
		m_opaqueObjects.resize(1024);
		m_opaqueObjectsSize = 0;		

		return result;
	}

	void Renderer::Update(float deltaTime)
	{

	}

	bool Renderer::BeginFrame()
	{
		bool result = RenderFunc::BeginFrame(&g_device, &m_swapChain);


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
		current_quad_batch = 0;
		for (uint32_t i = 0; i < num_avalible_quad_batch; i++)
		{
			quadBatches[i].current_index = 0;
			quadBatches[i].current_texture_units = 0;
			quadBatches[i].current_unit = 0;
		}
		RenderFunc::EndFrame(&g_device);
		current_sky_box = nullptr;
	}


	void Renderer::Start()
	{

		// Temp------------------------------------------------------------------------------------------------


		EventsSystem::get_instance()->AddEventListener(EventType::TRC_KEY_RELEASED, BIND_EVENT_FN(Renderer::OnEvent));
		EventsSystem::get_instance()->AddEventListener(EventType::TRC_WND_RESIZE, BIND_EVENT_FN(Renderer::OnEvent));
		EventsSystem::get_instance()->AddEventListener(EventType::TRC_KEY_PRESSED, BIND_EVENT_FN(Renderer::OnEvent));
		EventsSystem::get_instance()->AddEventListener(EventType::TRC_WND_CLOSE, BIND_EVENT_FN(Renderer::OnEvent));
		render_mode = {};


		//lights[0].position = { 0.3597f, -0.4932f, 0.7943f, 0.0f };
		//lights[0].direction = { 0.3597f, -0.4932f, 0.7943f, 0.0f };
		//lights[0].color = { 0.8f, 0.8f, 0.8f, 1.0f };
		//lights[0].params1 = { 1.0f, 0.467f, 0.896f, 0.0f };
		//lights[0].params2 = { 0.0f, 2.0f, 0.0f, 0.0f };

		lights[0].position = { 0.0f, 2.5f, 2.0f, 0.0f };
		lights[0].direction = { -0.3597f, 0.4932f, -0.7943f, 0.0f };
		lights[0].color = { 0.37f, 0.65f, 0.66f, 1.0f };
		lights[0].params1 = { 1.0f, 0.022f, 0.0019f, glm::cos(glm::radians(6.0f))};
		lights[0].params2 = { glm::cos(glm::radians(30.0f)), 5.5f, 0.0f, 0.0f };

		//lights[0].position = { _camera->GetPosition(), 0.0f };
		//lights[0].direction = { _camera->GetLookDir(), 0.0f };
		//lights[0].color = { 0.6f, 0.8f, 0.0f, 1.0f };
		//lights[0].params1 = { 1.0f, 0.022f, 0.0019f, 0.939f };
		//lights[0].params2 = { 0.866f, 3.1f, 0.0f, 0.0f };
			

		light_data = { 0, 1, 0, 0 };
		exposure = 0.9f;
		m_composer = new RenderComposer();
		m_composer->Init(this);
		current_sky_box = nullptr;

		// Quad batch .........................................
		Ref<GShader> VertShader = ResourceSystem::get_instance()->CreateShader("quad.vert.glsl", ShaderStage::VERTEX_SHADER);
		Ref<GShader> FragShader = ResourceSystem::get_instance()->CreateShader("quad.frag.glsl", ShaderStage::PIXEL_SHADER);

		ShaderResources s_res = {};
		ShaderParser::generate_shader_resources(VertShader.get(), s_res);
		ShaderParser::generate_shader_resources(FragShader.get(), s_res);

		PipelineStateDesc _ds2 = {};
		_ds2.vertex_shader = VertShader.get();
		_ds2.pixel_shader = FragShader.get();
		_ds2.resources = s_res;
		_ds2.input_layout = QuadBatch::get_input_layout();


		AutoFillPipelineDesc(
			_ds2,
			false
		);
		_ds2.render_pass = Renderer::get_instance()->GetRenderPass("FORWARD_PASS");


		if (!ResourceSystem::get_instance()->CreatePipeline(_ds2, "quad_batch_pipeline"))
		{
			TRC_ERROR("Failed to initialize or create quad_batch_pipeline");
			return;
		}

		quadBatchPipeline = ResourceSystem::get_instance()->GetPipeline("quad_batch_pipeline");
		
		create_quad_batch();

 		// ..................................................
		

		//---------------------------------------------------------------------------------------------

	}

	void Renderer::End()
	{

		// Temp-----------------------------

		RenderFunc::DestroyBuffer(&quadBuffer);
		m_composer->Shutdowm();
		delete m_composer;
		m_composer = nullptr;
		delete _camera;
		_camera = nullptr;

		//----------------------------------
		RenderFunc::DestroySwapchain(&m_swapChain);		

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

			if (press->m_keycode == KEY_O)
			{
				exposure += 0.4;
			}
			else if (press->m_keycode == KEY_P)
			{
				exposure -= 0.4;
			}
			else if (press->m_keycode == KEY_1)
			{
				frame_settings |= RENDER_SSAO;
			}
			else if (press->m_keycode == KEY_2)
			{
				frame_settings &= ~RENDER_SSAO;
			}
			else if (press->m_keycode == KEY_N)
			{
				frame_settings |= RENDER_BLOOM;
			}
			else if (press->m_keycode == KEY_M)
			{
				frame_settings &= ~RENDER_BLOOM;
			}

			break;
		}
		case trace::EventType::TRC_WND_CLOSE:
		{

			break;
		}
		case trace::EventType::TRC_KEY_RELEASED:
		{
			KeyReleased* release = reinterpret_cast<KeyReleased*>(p_event);

			if (release->m_keycode == Keys::KEY_0)
			{
				render_mode.x = 0;
			}
			else if (release->m_keycode == Keys::KEY_1)
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
			else if (release->m_keycode == Keys::KEY_5)
			{
				render_mode.x = 5;
			}
			else if (release->m_keycode == Keys::KEY_6)
			{
				render_mode.x = 6;
			}
			else if (release->m_keycode == Keys::KEY_7)
			{
				render_mode.x = 7;
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
			else if (release->m_keycode == Keys::KEY_R)
			{
				lights[2].color.r += 0.01f;
				if (lights[2].color.r > 1.0f) lights[2].color.r = 0.0f;

			}
			else if (release->m_keycode == Keys::KEY_G)
			{
				lights[2].color.g += 0.01f;
				if (lights[2].color.g > 1.0f) lights[2].color.g = 0.0f;
			}
			else if (release->m_keycode == Keys::KEY_B)
			{
				lights[2].color.b += 0.01f;
				if (lights[2].color.b > 1.0f) lights[2].color.b = 0.0f;
			}


			break;
		}

		case EventType::TRC_WND_RESIZE:
		{
			WindowResize* wnd = reinterpret_cast<WindowResize*>(p_event);
			RenderFunc::ResizeSwapchain(&m_swapChain, wnd->m_width, wnd->m_height);
			float width = static_cast<float>(wnd->m_width);
			float height = static_cast<float>(wnd->m_height);
			
			m_frameWidth = wnd->m_width;
			m_frameHeight = wnd->m_height;


			_viewPort.width = width;
			_viewPort.height = height;

			_rect.right = wnd->m_width;
			_rect.bottom = wnd->m_height;

			if (wnd->m_width == 0 || wnd->m_height == 0)
				break;

			_camera->SetAspectRatio(((float)wnd->m_width / (float)wnd->m_height));

			break;
		}

		}

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
				glm::vec3 light_pos = glm::vec3(lights[0].position);
				glm::vec3 light_dir = glm::vec3(lights[0].direction);
				glm::vec3 cam_pos = _camera->GetPosition();
				glm::vec3 cam_dir = _camera->GetLookDir();
				if (light_pos != cam_pos)
				{

					lights[0].position.x = lerp(light_pos.x, cam_pos.x, deltaTime);
					lights[0].position.y = lerp(light_pos.y, cam_pos.y, deltaTime);
					lights[0].position.z = lerp(light_pos.z, cam_pos.z, deltaTime);
				}
				if (light_dir != cam_dir)
				{
					lights[0].direction.x = lerp(light_dir.x, cam_dir.x, deltaTime);
					lights[0].direction.y = lerp(light_dir.y, cam_dir.y, deltaTime);
					lights[0].direction.z = lerp(light_dir.z, cam_dir.z, deltaTime);
				}
			}

			for (uint32_t i = 0; i < m_listCount; i++)
			{
				for (Command& cmd : m_cmdList[i]._commands)
				{
					cmd.func(cmd.params);
				}
			}


			RGBlackBoard frame_blck_bd;
			RenderGraph f_g;
			
			m_composer->PreFrame(
				f_g,
				frame_blck_bd,
				frame_settings
			);
			f_g.Execute();
			m_composer->PostFrame(
				f_g,
				frame_blck_bd
			);

			EndFrame();
			RenderFunc::PresentSwapchain(&m_swapChain);
			f_g.Destroy();
		}
		m_listCount = 0;
		m_opaqueObjectsSize = 0;

	}

	void Renderer::DrawQuad()
	{
		RenderFunc::BindVertexBuffer(&g_device, &quadBuffer);
		RenderFunc::Draw(&g_device, 0, 6);
	}

	void Renderer::DrawQuad(glm::mat4 transform)
	{
		if (quadBatches[current_quad_batch].current_unit >= quadBatches[current_quad_batch].max_units - 1)
		{
			flush_current_quad_batch();
			current_quad_batch++;
			num_avalible_quad_batch++;
			create_quad_batch();
		}

		uint32_t current_vertex = quadBatches[current_quad_batch].current_unit * 4;
		quadBatchVertex[current_vertex].pos = glm::vec3( transform * glm::vec4(-1.0f, -1.0f, 0.0f, 1.0f));
		quadBatchVertex[current_vertex].texCoord = glm::vec2(0.0f);
		quadBatchVertex[current_vertex].tex_index = 0;
		current_vertex++;

		quadBatchVertex[current_vertex].pos = glm::vec3(transform * glm::vec4(1.0f, 1.0f, 0.0f, 1.0f));
		quadBatchVertex[current_vertex].texCoord = glm::vec2(1.0f);
		quadBatchVertex[current_vertex].tex_index = 0;
		current_vertex++;

		quadBatchVertex[current_vertex].pos = glm::vec3(transform * glm::vec4(-1.0f, 1.0f, 0.0f, 1.0f));
		quadBatchVertex[current_vertex].texCoord = glm::vec2(0.0f, 1.0f);
		quadBatchVertex[current_vertex].tex_index = 0;
		current_vertex++;

		quadBatchVertex[current_vertex].pos = glm::vec3(transform * glm::vec4(1.0f, -1.0f, 0.0f, 1.0f));
		quadBatchVertex[current_vertex].texCoord = glm::vec2(1.0f, 0.0f);
		quadBatchVertex[current_vertex].tex_index = 0;
		current_vertex++;


		uint32_t current_index = quadBatches[current_quad_batch].current_index * 6;
		quadBatchIndex[current_index] = (quadBatches[current_quad_batch].current_unit * 4) + 0;
		current_index++;
		quadBatchIndex[current_index] = (quadBatches[current_quad_batch].current_unit * 4) + 1;
		current_index++;
		quadBatchIndex[current_index] = (quadBatches[current_quad_batch].current_unit * 4) + 2;
		current_index++;
		quadBatchIndex[current_index] = (quadBatches[current_quad_batch].current_unit * 4) + 0;
		current_index++;
		quadBatchIndex[current_index] = (quadBatches[current_quad_batch].current_unit * 4) + 1;
		current_index++;
		quadBatchIndex[current_index] = (quadBatches[current_quad_batch].current_unit * 4) + 3;
		current_index++;

		quadBatches[current_quad_batch].current_unit++;
		quadBatches[current_quad_batch].current_index++;

	}

	void Renderer::RenderOpaqueObjects()
	{

		glm::mat4 proj = _camera->GetProjectionMatix();
		glm::mat4 view = _camera->GetViewMatrix();
		glm::vec3 view_position = _camera->GetPosition();

		for (uint32_t i = 0; i < m_opaqueObjectsSize; i++)
		{
			auto& data = m_opaqueObjects[i];
			glm::mat4* M_model = &data.first;
			Model* _model = data.second;
			Ref<MaterialInstance> _mi = _model->m_matInstance.is_valid() ? _model->m_matInstance : ResourceSystem::get_instance()->GetMaterial("default");
			Ref<GPipeline> sp = _mi->GetRenderPipline();

			RenderFunc::OnDrawStart(&g_device, sp.get());
			RenderFunc::SetPipelineData(sp.get(), "projection", ShaderResourceStage::RESOURCE_STAGE_GLOBAL, &proj, sizeof(glm::mat4));
			RenderFunc::SetPipelineData(sp.get(), "view", ShaderResourceStage::RESOURCE_STAGE_GLOBAL, &view, sizeof(glm::mat4));
			RenderFunc::SetPipelineData(sp.get(), "view_position", ShaderResourceStage::RESOURCE_STAGE_GLOBAL, &view_position, sizeof(glm::vec3));
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

	void Renderer::RenderLights()
	{
		Ref<Mesh> sphere = ResourceSystem::get_instance()->GetDefaultMesh("Sphere");
		Ref<GPipeline> sp = ResourceSystem::get_instance()->GetPipeline("light_pipeline");
		Model* _model = sphere->GetModels()[0].get();
		glm::mat4 view_proj= _camera->GetProjectionMatix() * _camera->GetViewMatrix();

		for (int i = 0; i < light_data.y; i++)
		{
			int index = i + light_data.x;
			Light light = lights[index];
			glm::mat4 model = glm::identity<glm::mat4>();
			model = glm::translate(model, glm::vec3(light.position));
			glm::vec4 light_color = glm::vec4(glm::vec3(light.color), light.params2.y);


			RenderFunc::OnDrawStart(&g_device, sp.get());
			RenderFunc::SetPipelineData(sp.get(), "view_projection", ShaderResourceStage::RESOURCE_STAGE_GLOBAL, &view_proj, sizeof(glm::mat4));
			RenderFunc::SetPipelineData(sp.get(), "color", ShaderResourceStage::RESOURCE_STAGE_INSTANCE, &light_color, sizeof(glm::vec4));
			RenderFunc::SetPipelineData(sp.get(), "model", ShaderResourceStage::RESOURCE_STAGE_LOCAL, &model, sizeof(glm::mat4));
			RenderFunc::BindPipeline(&g_device, sp.get());
			RenderFunc::BindPipeline_(sp.get());
			RenderFunc::BindVertexBuffer(&g_device, _model->GetVertexBuffer());
			RenderFunc::BindIndexBuffer(&g_device, _model->GetIndexBuffer());

			RenderFunc::DrawIndexed(&g_device, 0, _model->GetIndexCount());
			RenderFunc::OnDrawEnd(&g_device, sp.get());

		}


		//TEMP: Find vaild function to render sky box
		if (current_sky_box)
		{
			SkyBox* sky_box = current_sky_box;

			glm::mat4 proj = _camera->GetProjectionMatix();
			glm::mat4 view = _camera->GetViewMatrix();
			glm::vec3 view_position = _camera->GetPosition();

			Ref<GPipeline> sp = ResourceSystem::get_instance()->GetDefaultPipeline("skybox");
			RenderFunc::OnDrawStart(&g_device, sp.get());

			RenderFunc::SetPipelineTextureData(
				sp.get(),
				"CubeMap",
				ShaderResourceStage::RESOURCE_STAGE_GLOBAL,
				sky_box->GetCubeMap()
			);
			RenderFunc::SetPipelineData(sp.get(), "projection", ShaderResourceStage::RESOURCE_STAGE_GLOBAL, &proj, sizeof(glm::mat4));
			RenderFunc::SetPipelineData(sp.get(), "view", ShaderResourceStage::RESOURCE_STAGE_GLOBAL, &view, sizeof(glm::mat4));
			RenderFunc::SetPipelineData(sp.get(), "view_position", ShaderResourceStage::RESOURCE_STAGE_GLOBAL, &view_position, sizeof(glm::vec3));
			RenderFunc::BindPipeline_(sp.get());

			Ref<Model> mod = sky_box->GetCube()->GetModels()[0];
			RenderFunc::BindPipeline(&g_device, sp.get());
			RenderFunc::BindVertexBuffer(&g_device, mod->GetVertexBuffer());
			RenderFunc::BindIndexBuffer(&g_device, mod->GetIndexBuffer());

			RenderFunc::DrawIndexed(&g_device, 0, mod->GetIndexCount());
			RenderFunc::OnDrawEnd(&g_device, sp.get());
		}

	}

	void Renderer::RenderQuads()
	{
		flush_current_quad_batch();
		glm::mat4 proj = _camera->GetProjectionMatix() * _camera->GetViewMatrix();
		for (uint32_t i = 0; i < num_avalible_quad_batch; i++)
		{
			RenderFunc::OnDrawStart(&g_device, quadBatchPipeline.get());
			RenderFunc::SetPipelineData(quadBatchPipeline.get(), "projection", ShaderResourceStage::RESOURCE_STAGE_GLOBAL, &proj, sizeof(glm::mat4));
			RenderFunc::BindPipeline_(quadBatchPipeline.get());
			RenderFunc::BindPipeline(&g_device, quadBatchPipeline.get());
			RenderFunc::BindVertexBuffer(&g_device, &quadBatches[i].vertex_buffer);
			RenderFunc::BindIndexBuffer(&g_device, &quadBatches[i].index_buffer);
			RenderFunc::DrawIndexed(&g_device, 0, quadBatches[i].current_index * 6);
			RenderFunc::OnDrawEnd(&g_device, quadBatchPipeline.get());

		}
	}


	void Renderer::draw_mesh(CommandParams params)
	{
		Mesh* mesh = (Mesh*)params.ptrs[0];

		glm::mat4* M_model = (glm::mat4*)(&params.data);
		glm::mat4 proj = _camera->GetProjectionMatix();
		glm::mat4 view = _camera->GetViewMatrix();
		glm::vec3 view_position = _camera->GetPosition();

		for (Ref<Model> _model : mesh->GetModels())
		{
			m_opaqueObjects[m_opaqueObjectsSize++] = std::make_pair(*M_model, _model.get());
		}

	}

	void Renderer::draw_skybox(CommandParams params)
	{
		if (current_sky_box)
		{
			TRC_WARN("Only sky can be drawn per frame {}", __FUNCTION__);
			return;
		}
		current_sky_box = (SkyBox*)params.ptrs[0];

	}

	void Renderer::create_quad_batch()
	{
		quadBatches.resize(num_avalible_quad_batch);
		quadBatches[current_quad_batch].current_index = 0;
		quadBatches[current_quad_batch].current_texture_units = 0;
		quadBatches[current_quad_batch].current_unit = 0;
		quadBatches[current_quad_batch].max_texture_units = 16;
		quadBatches[current_quad_batch].max_units = 256;
		BufferInfo vertex_info;
		vertex_info.m_data = nullptr;
		vertex_info.m_flag = BindFlag::VERTEX_BIT;
		vertex_info.m_size = quadBatches[current_quad_batch].max_units * sizeof(QuadBatch) * 4;
		vertex_info.m_usageFlag = UsageFlag::UPLOAD;
		RenderFunc::CreateBuffer(&quadBatches[current_quad_batch].vertex_buffer, vertex_info);

		BufferInfo index_info;
		index_info.m_data = nullptr;
		index_info.m_flag = BindFlag::INDEX_BIT;
		index_info.m_size = quadBatches[current_quad_batch].max_units * sizeof(uint32_t) * 6;
		index_info.m_usageFlag = UsageFlag::UPLOAD;
		RenderFunc::CreateBuffer(&quadBatches[current_quad_batch].index_buffer, index_info);
		quadBatchVertex.resize(quadBatches[current_quad_batch].max_units * sizeof(QuadBatch) * 4);
		quadBatchIndex.resize(quadBatches[current_quad_batch].max_units * sizeof(uint32_t) * 6);

	}

	void Renderer::flush_current_quad_batch()
	{
		RenderFunc::SetBufferData(&quadBatches[current_quad_batch].vertex_buffer, quadBatchVertex.data(), quadBatchVertex.size() * sizeof(QuadBatch));
		RenderFunc::SetBufferData(&quadBatches[current_quad_batch].index_buffer, quadBatchIndex.data(), quadBatchIndex.size() * sizeof(uint32_t));
	}

	void Renderer::destroy_quad_batchs()
	{
		for (uint32_t i = 0; i < num_avalible_quad_batch; i++)
		{
			RenderFunc::DestroyBuffer(&quadBatches[i].vertex_buffer);
			RenderFunc::DestroyBuffer(&quadBatches[i].index_buffer);
		}
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