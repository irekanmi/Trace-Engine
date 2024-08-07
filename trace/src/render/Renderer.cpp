#include "pch.h"

#include "Renderer.h"
#include "backends/Renderutils.h"
#include "GContext.h"
#include "core/Enums.h"
#include "core/io/Logging.h"
#include "core/events/EventsSystem.h"
#include "core/Enums.h"
#include "Model.h"
#include "Mesh.h"
#include "SkyBox.h"
#include "core/Coretypes.h"
#include "RenderComposer.h"
#include "render_graph/RenderGraph.h"
#include "backends/UIutils.h"
#include "resource/MaterialManager.h"
#include "resource/PipelineManager.h"
#include "core/memory/MemoryManager.h"
#include "debug/Debugger.h"

//Temp============
#include "glm/gtc/matrix_transform.hpp"
#include "core/Utils.h"
#include "render_graph/FrameData.h"
#include "ShaderParser.h"
#include "GShader.h"
#include "Font.h"
#include "backends/Fontutils.h"
#include "Application.h"


namespace trace {


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

	bool Renderer::Init(trc_app_data app_data)
	{
		bool result = true;

		m_cmdList.resize(8);

		for (uint32_t i = 0; i < 8; i++)
		{
			m_cmdList[i]._commands.reserve(10);
		}
		m_listCount = 0;
	
		switch (app_data.graphics_api)
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
		RenderFunc::CreateSwapchain(&m_swapChain, &g_device, &g_context);
		{
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

		if (!app_data.client_render)
		{
			TRC_ERROR("Failed to provide callback for render");
			return false;
		}
		m_client_render = app_data.client_render;
				

		m_composer = nullptr;
		if (app_data.render_composer)
		{
			m_composer = (RenderComposer*)app_data.render_composer;
		}


		// Text rendering initializaton ----------------------------

		//TODO: allow it to automatically grow when needed
		text_vertices.resize(16);

		BufferInfo text_buffer_info;
		text_buffer_info.m_data = nullptr;
		text_buffer_info.m_flag = BindFlag::VERTEX_BIT;
		text_buffer_info.m_size = sizeof(TextVertex) * KB * 2;
		text_buffer_info.m_stide = sizeof(TextVertex);
		text_buffer_info.m_usageFlag = UsageFlag::DEFAULT;

		RenderFunc::CreateBuffer(&text_buffer, text_buffer_info);


		// --------------------------------------------------------

		return result;
	}

	void Renderer::Update(float deltaTime)
	{

	}

	bool Renderer::BeginFrame()
	{
		bool result = RenderFunc::BeginFrame(&g_device, &m_swapChain);

		if(result) UIFunc::UINewFrame();
		return result;
	}

	void Renderer::EndFrame()
	{
		current_quad_batch = 0;
		for (uint32_t i = 0; i < num_avalible_quad_batch; i++)
		{
			quadBatches[i].current_index = 0;
			quadBatches[i].current_texture_unit = 0;
			quadBatches[i].current_unit = 0;
			quadBatches[i].tex = nullptr;
		}
		boundQuadTextures.clear();

		current_text_batch = 0;
		for (uint32_t i = 0; i < num_avalible_text_batch; i++)
		{
			textBatches[i].current_index = 0;
			textBatches[i].current_texture_unit = 0;
			textBatches[i].current_unit = 0;
		}
		boundTextTextures.clear();

		/*if (m_debug)
		{
			m_debug->positions.clear();
			m_debug->vert_count = 0;
		}*/

		Debugger* debugger = Debugger::get_instance();
		if (debugger->IsInitialized())
		{
			Debugger::DebugRenderData& render_data = debugger->GetRenderData();
			render_data.positions.clear();
			render_data.vert_count = 0;
		}


		UIFunc::UIEndFrame();
		RenderFunc::EndFrame(&g_device);
		current_sky_box = nullptr;

		for (auto& i : text_vertices)
		{
			i.clear();
		}
		bound_text_atlases.clear();
		text_atlases.clear();
	}


	void Renderer::Start()
	{

		// Temp------------------------------------------------------------------------------------------------


		EventsSystem::get_instance()->AddEventListener(EventType::TRC_WND_RESIZE, BIND_EVENT_FN(Renderer::OnEvent));
		EventsSystem::get_instance()->AddEventListener(EventType::TRC_KEY_PRESSED, BIND_EVENT_FN(Renderer::OnEvent));

		m_opaqueObjects.resize(1024);
		m_opaqueObjectsSize = 0;

		/*m_meshedLights.resize(64);
		m_meshLightSize = 0;*/

		lights.resize(MAX_LIGHT_COUNT);
		light_data = glm::ivec4(0);
					
		text_verts = true;

		exposure = 0.9f;
		current_sky_box = nullptr;
		if (m_composer)
		{
			m_composer->Init(this);
		}
		else
		{
			m_composer = new RenderComposer();
			m_composer->Init(this);
		}


				
		//---------------------------------------------------------------------------------------------

		// Quad batch .........................................	
		create_quad_batch();
 		// ..................................................
		
		// Text batch .........................................	
		create_text_batch();
 		// ..................................................	



		if (AppSettings::is_editor)
		{
			UIFuncLoader::LoadImGuiFunc();
			UIFunc::InitUIRenderBackend(Application::get_instance(), Renderer::get_instance());
		}

	}

	void Renderer::End()
	{
		//Text Rendering -------------------------

		RenderFunc::DestroyBuffer(&text_buffer);

		// ---------------------------------------

		m_composer->Shutdowm();
		delete m_composer;
		m_composer = nullptr;
		_camera = nullptr;
		RenderFunc::DestroySwapchain(&m_swapChain);

	}

	void Renderer::ShutDown()
	{
		RenderFunc::DestroyDevice(&g_device);
		RenderFunc::DestroyContext(&g_context);
	}
	 
	void Renderer::OnEvent(Event* p_event)
	{

		switch (p_event->GetEventType())
		{
		case trace::EventType::TRC_KEY_PRESSED:
		{
			KeyPressed* press = reinterpret_cast<KeyPressed*>(p_event);

			if (press->GetKeyCode() == KEY_O)
			{
				exposure += 0.4;
			}
			else if (press->GetKeyCode() == KEY_P)
			{
				exposure -= 0.4;
			}
			else if (press->GetKeyCode() == KEY_1)
			{
				frame_settings |= RENDER_SSAO;
			}
			else if (press->GetKeyCode() == KEY_2)
			{
				frame_settings &= ~RENDER_SSAO;
			}
			else if (press->GetKeyCode() == KEY_N)
			{
				frame_settings |= RENDER_BLOOM;
			}
			else if (press->GetKeyCode() == KEY_M)
			{
				frame_settings &= ~RENDER_BLOOM;
			}

			break;
		}
		case EventType::TRC_WND_RESIZE:
		{
			WindowResize* wnd = reinterpret_cast<WindowResize*>(p_event);
			RenderFunc::ResizeSwapchain(&m_swapChain, wnd->GetWidth(), wnd->GetHeight());
			float width = static_cast<float>(wnd->GetWidth());
			float height = static_cast<float>(wnd->GetHeight());
			
			m_frameWidth = wnd->GetWidth();
			m_frameHeight = wnd->GetHeight();

			_viewPort.width = width;
			_viewPort.height = height;

			_rect.right = wnd->GetWidth();
			_rect.bottom = wnd->GetHeight();
			break;
		}

		}

	}


	void Renderer::Render(float deltaTime)
	{
		if (BeginFrame())
		{

			m_client_render(deltaTime);

			for (uint32_t i = 0; i < m_listCount; i++)
			{
				for (Command& cmd : m_cmdList[i]._commands)
				{
					cmd.func(cmd.params);
				}
			}


			RGBlackBoard frame_blck_bd;
			RenderGraph frame_graph;			
			m_composer->PreFrame(frame_graph, frame_blck_bd, frame_settings);
			frame_graph.Execute();
			m_composer->PostFrame(frame_graph, frame_blck_bd);

			EndFrame();
			RenderFunc::PresentSwapchain(&m_swapChain);
			frame_graph.Destroy();
		}
		m_listCount = 0;
		m_opaqueObjectsSize = 0;

		//m_meshLightSize = 0;

		light_data = glm::ivec4(0);
		lights.clear();

		light_positions.clear();
		light_directions.clear();
		light_colors.clear();
		light_params1s.clear();
		light_params2s.clear();

		

	}

	void Renderer::DrawQuad()
	{
		TRC_ASSERT(false, "Funtion {} has not been implemented", __FUNCTION__);
	}

	void Renderer::DrawQuad(glm::mat4 _transform, Ref<GTexture> texture)
	{
		if (quadBatches[current_quad_batch].current_texture_unit >= quadBatches[current_quad_batch].max_texture_units - 1)
		{
			flush_current_quad_batch();
		}
		if (quadBatches[current_quad_batch].current_unit >= quadBatches[current_quad_batch].max_units - 1)
		{
			flush_current_quad_batch();
		}

		float current_tex_index = 0.0f;
		auto tex_index = boundQuadTextures.find(texture->m_id);
		if (tex_index == boundQuadTextures.end())
		{
			quadBatches[current_quad_batch].textures[quadBatches[current_quad_batch].current_texture_unit] = texture.get();
			current_tex_index = (float)(quadBatches[current_quad_batch].current_texture_unit);
			quadBatches[current_quad_batch].current_texture_unit++;
			boundQuadTextures.emplace(texture->m_id);
		}
		else
		{
			uint32_t index = 0;
			//TODO: Find a better way to find texture index
			auto value = std::find_if(quadBatches[current_quad_batch].textures.begin(), quadBatches[current_quad_batch].textures.end(), [&tex_index, &index](GTexture* tex) {
				index++;
				return tex->m_id == *tex_index;
				});
			current_tex_index = (float)(index - 1);
		}

		uint32_t current_vert = quadBatches[current_quad_batch].current_unit * 6;
		quadBatches[current_quad_batch].positions[current_vert] = _transform * glm::vec4(-1.0f, -1.0f, 0.0f, 1.0f);
		quadBatches[current_quad_batch].tex_coords[current_vert] = glm::vec4(0.0f, 0.0f, current_tex_index, 0.0f);
		current_vert++;

		quadBatches[current_quad_batch].positions[current_vert] = _transform * glm::vec4(1.0f, -1.0f, 0.0f, 1.0f);
		quadBatches[current_quad_batch].tex_coords[current_vert] = glm::vec4(1.0f, 0.0f, current_tex_index, 0.0f);
		current_vert++;

		quadBatches[current_quad_batch].positions[current_vert] = _transform * glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);
		quadBatches[current_quad_batch].tex_coords[current_vert] = glm::vec4(1.0f, 1.0f, current_tex_index, 0.0f);
		current_vert++;

		quadBatches[current_quad_batch].positions[current_vert] = _transform * glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);
		quadBatches[current_quad_batch].tex_coords[current_vert] = glm::vec4(1.0f, 1.0f, current_tex_index, 0.0f);
		current_vert++;

		quadBatches[current_quad_batch].positions[current_vert] = _transform * glm::vec4(-1.0f, 1.0f, 0.0f, 1.0f);
		quadBatches[current_quad_batch].tex_coords[current_vert] = glm::vec4(0.0f, 1.0f, current_tex_index, 0.0f);
		current_vert++;

		quadBatches[current_quad_batch].positions[current_vert] = _transform * glm::vec4(-1.0f, -1.0f, 0.0f, 1.0f);
		quadBatches[current_quad_batch].tex_coords[current_vert] = glm::vec4(0.0f, 0.0f, current_tex_index, 0.0f);
		current_vert++;

		quadBatches[current_quad_batch].current_unit++;

	}

	void Renderer::DrawQuad_(glm::mat4 _transform, Ref<GTexture> texture)
	{	
		
		DrawQuad_(_transform, texture, TRC_COL32_BLACK);

	}

	void Renderer::DrawQuad_(glm::mat4 _transform, Ref<GTexture> texture, uint32_t color)
	{
		if (!quadBatches[current_quad_batch].tex)
		{
			quadBatches[current_quad_batch].tex = texture.get();
		}
		else if (quadBatches[current_quad_batch].tex != texture.get())
		{
			int index = -1;
			auto it = std::find_if(quadBatches.begin(), quadBatches.end(), [&texture, &index](BatchInfo& a) {
				index++;
				return a.tex == texture.get();
				});


			if (it != quadBatches.end())
			{
				current_quad_batch = index;
			}
			else
			{
				flush_current_quad_batch();
				quadBatches[current_quad_batch].tex = texture.get();
			}
		}

		if (quadBatches[current_quad_batch].current_unit >= quadBatches[current_quad_batch].max_units - 1)
		{
			flush_current_quad_batch();
			quadBatches[current_quad_batch].tex = texture.get();
		}

		float new_color = 0.0f;
		memcpy(&new_color, &color, sizeof(uint32_t));

		uint32_t current_vert = quadBatches[current_quad_batch].current_unit * 6;
		quadBatches[current_quad_batch].positions[current_vert] = _transform * glm::vec4(-1.0f, -1.0f, 0.0f, 1.0f);
		quadBatches[current_quad_batch].positions[current_vert].a =  new_color;
		quadBatches[current_quad_batch].tex_coords[current_vert] = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
		current_vert++;

		quadBatches[current_quad_batch].positions[current_vert] = _transform * glm::vec4(1.0f, -1.0f, 0.0f, 1.0f);
		quadBatches[current_quad_batch].positions[current_vert].a = new_color;
		quadBatches[current_quad_batch].tex_coords[current_vert] = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
		current_vert++;

		quadBatches[current_quad_batch].positions[current_vert] = _transform * glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);
		quadBatches[current_quad_batch].positions[current_vert].a = new_color;
		quadBatches[current_quad_batch].tex_coords[current_vert] = glm::vec4(1.0f, 1.0f, 0.0f, 0.0f);
		current_vert++;

		quadBatches[current_quad_batch].positions[current_vert] = _transform * glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);
		quadBatches[current_quad_batch].positions[current_vert].a = new_color;
		quadBatches[current_quad_batch].tex_coords[current_vert] = glm::vec4(1.0f, 1.0f, 0.0f, 0.0f);
		current_vert++;

		quadBatches[current_quad_batch].positions[current_vert] = _transform * glm::vec4(-1.0f, 1.0f, 0.0f, 1.0f);
		quadBatches[current_quad_batch].positions[current_vert].a = new_color;
		quadBatches[current_quad_batch].tex_coords[current_vert] = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
		current_vert++;

		quadBatches[current_quad_batch].positions[current_vert] = _transform * glm::vec4(-1.0f, -1.0f, 0.0f, 1.0f);
		quadBatches[current_quad_batch].positions[current_vert].a = new_color;
		quadBatches[current_quad_batch].tex_coords[current_vert] = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
		current_vert++;

		quadBatches[current_quad_batch].current_unit++;
	}

	void Renderer::DrawString(Font* font, const std::string& text, glm::mat4 _transform)
	{
		Ref<GTexture> texture = font->GetAtlas();
		if (textBatches[current_text_batch].current_texture_unit >= textBatches[current_text_batch].max_texture_units - 1)
		{
			flush_current_text_batch();
		}
		if (textBatches[current_text_batch].current_unit >= textBatches[current_text_batch].max_units - 1)
		{
			flush_current_text_batch();
		}

		float current_tex_index = 0.0f;
		auto tex_index = boundTextTextures.find(texture->m_id);
		if (tex_index == boundTextTextures.end())
		{
			textBatches[current_text_batch].textures[textBatches[current_text_batch].current_texture_unit] = texture.get();
			current_tex_index = (float)(textBatches[current_text_batch].current_texture_unit);
			textBatches[current_text_batch].current_texture_unit++;
			boundTextTextures.emplace(texture->m_id);
		}
		else
		{
			uint32_t index = 0;
			//TODO: Find a better to find texture index
			auto value = std::find_if(textBatches[current_text_batch].textures.begin(), textBatches[current_text_batch].textures.end(), [&tex_index, &index](GTexture* tex) {
				index++;
				return tex->m_id == *tex_index;
				});
			current_tex_index = (float)(index - 1);
		}

		uint32_t current_vert = textBatches[current_text_batch].current_unit;
		uint32_t count = 0;
		FontFunc::ComputeTextString(font, text, textBatches[current_text_batch].positions, current_vert, textBatches[current_text_batch].tex_coords, _transform, current_tex_index, count);
		textBatches[current_text_batch].current_unit += count;
	}

	void Renderer::DrawString_(Font* font, const std::string& text, glm::vec3 color, glm::mat4 _transform)
	{
		Ref<GTexture> texture = font->GetAtlas();

		// Finding texture index ===========================================
		uint32_t current_tex_index;
		auto tex_index = bound_text_atlases.find(texture->m_id);
		if (tex_index == bound_text_atlases.end())
		{
			current_tex_index = text_atlases.size();
			text_atlases.emplace_back(texture.get());
			bound_text_atlases.emplace(texture->m_id);
		}
		else
		{
			uint32_t index = 0;
			//TODO: Find a better to find texture index
			auto value = std::find_if(text_atlases.begin(), text_atlases.end(), [&tex_index, &index](GTexture* tex) {
				index++;
				return tex->m_id == *tex_index;
				});
			current_tex_index = (float)(index - 1);
		}
		// ==================================================================


		std::vector<TextVertex>& vertices = text_vertices[current_tex_index];
		FontFunc::ComputeTextVertex(font, text, vertices, _transform, color);

	}

	void Renderer::RenderOpaqueObjects()
	{

		glm::mat4 proj = _camera->GetProjectionMatix();
		glm::mat4 view = _camera->GetViewMatrix();
		glm::vec3 view_position = _camera->GetPosition();
		glm::mat4 view_proj = proj * view;

		for (uint32_t i = 0; i < m_opaqueObjectsSize; i++)
		{
			auto& data = m_opaqueObjects[i];
			glm::mat4* M_model = &data.transform;
			Model* _model = data.object;
			MaterialInstance* _mi = data.material? data.material : MaterialManager::get_instance()->GetMaterial("default").get();
			Ref<GPipeline> sp = _mi->GetRenderPipline();

			RenderFunc::OnDrawStart(&g_device, sp.get());
			RenderFunc::ApplyMaterial(_mi);
			RenderFunc::SetPipelineData(sp.get(), "_projection", ShaderResourceStage::RESOURCE_STAGE_GLOBAL, &proj, sizeof(glm::mat4));
			RenderFunc::SetPipelineData(sp.get(), "_view", ShaderResourceStage::RESOURCE_STAGE_GLOBAL, &view, sizeof(glm::mat4));
			RenderFunc::SetPipelineData(sp.get(), "_view_position", ShaderResourceStage::RESOURCE_STAGE_GLOBAL, &view_position, sizeof(glm::vec3));
			RenderFunc::SetPipelineData(sp.get(), "_model", ShaderResourceStage::RESOURCE_STAGE_INSTANCE, M_model, sizeof(glm::mat4));
			RenderFunc::BindPipeline(&g_device, sp.get());
			RenderFunc::BindPipeline_(sp.get());
			RenderFunc::BindVertexBuffer(&g_device, _model->GetVertexBuffer());
			RenderFunc::BindIndexBuffer(&g_device, _model->GetIndexBuffer());

			RenderFunc::DrawIndexed(&g_device, 0, _model->GetIndexCount());
			RenderFunc::OnDrawEnd(&g_device, sp.get());
		}

	}

	void Renderer::RenderLights()
	{
		Ref<GPipeline> sp = PipelineManager::get_instance()->GetPipeline("light_pipeline");
		glm::mat4 view_proj = _camera->GetProjectionMatix() * _camera->GetViewMatrix();

		/*for (int i = 0; i < m_meshLightSize; i++)
		{
			auto& data = m_meshedLights[i];
			int index = data.first;
			Light& _light = lights[index];
			Model* _model = data.second;
			glm::mat4 model = glm::identity<glm::mat4>();
			model = glm::translate(model, glm::vec3(_light.position));
			glm::vec4 light_color = glm::vec4(glm::vec3(_light.color), _light.params2.y);


			RenderFunc::OnDrawStart(&g_device, sp.get());
			RenderFunc::SetPipelineData(sp.get(), "_view_projection", ShaderResourceStage::RESOURCE_STAGE_GLOBAL, &view_proj, sizeof(glm::mat4));
			RenderFunc::SetPipelineData(sp.get(), "color", ShaderResourceStage::RESOURCE_STAGE_INSTANCE, &light_color, sizeof(glm::vec4));
			RenderFunc::SetPipelineData(sp.get(), "_model", ShaderResourceStage::RESOURCE_STAGE_LOCAL, &model, sizeof(glm::mat4));
			RenderFunc::BindPipeline_(sp.get());
			RenderFunc::BindPipeline(&g_device, sp.get());
			RenderFunc::BindVertexBuffer(&g_device, _model->GetVertexBuffer());
			RenderFunc::BindIndexBuffer(&g_device, _model->GetIndexBuffer());

			RenderFunc::DrawIndexed(&g_device, 0, _model->GetIndexCount());
			RenderFunc::OnDrawEnd(&g_device, sp.get());

		}*/


		//TEMP: Find vaild function to render sky box
		if (current_sky_box)
		{
			SkyBox* sky_box = current_sky_box;

			glm::mat4 proj = _camera->GetProjectionMatix();
			glm::mat4 view = _camera->GetViewMatrix();
			glm::vec3 view_position = _camera->GetPosition();

			Ref<GPipeline> sp = PipelineManager::get_instance()->GetDefault("skybox");
			RenderFunc::OnDrawStart(&g_device, sp.get());

			RenderFunc::SetPipelineTextureData(
				sp.get(),
				"CubeMap",
				ShaderResourceStage::RESOURCE_STAGE_GLOBAL,
				sky_box->GetCubeMap()
			);
			RenderFunc::SetPipelineData(sp.get(), "_projection", ShaderResourceStage::RESOURCE_STAGE_GLOBAL, &proj, sizeof(glm::mat4));
			RenderFunc::SetPipelineData(sp.get(), "_view", ShaderResourceStage::RESOURCE_STAGE_GLOBAL, &view, sizeof(glm::mat4));
			RenderFunc::SetPipelineData(sp.get(), "_view_position", ShaderResourceStage::RESOURCE_STAGE_GLOBAL, &view_position, sizeof(glm::vec3));
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
		quadBatchPipeline = PipelineManager::get_instance()->GetPipeline("quad_batch_pipeline");
		glm::mat4 proj = _camera->GetProjectionMatix() * _camera->GetViewMatrix();
		for (uint32_t i = 0; i < num_avalible_quad_batch; i++)
		{
			if (quadBatches[i].current_unit == 0) continue;
			/*for (uint32_t j = 0; j < quadBatches[i].current_texture_unit; j++)
			{
				RenderFunc::SetPipelineTextureData(quadBatchPipeline.get(), "u_textures" + std::to_string(j), ShaderResourceStage::RESOURCE_STAGE_GLOBAL, quadBatches[i].textures[j], j);
			}*/
			RenderFunc::OnDrawStart(&g_device, quadBatchPipeline.get());
			RenderFunc::SetPipelineTextureData(quadBatchPipeline.get(), "u_textures", ShaderResourceStage::RESOURCE_STAGE_INSTANCE, quadBatches[i].tex);
			RenderFunc::SetPipelineData(quadBatchPipeline.get(), "_projection", ShaderResourceStage::RESOURCE_STAGE_GLOBAL, &proj, sizeof(glm::mat4));
			RenderFunc::SetPipelineData(quadBatchPipeline.get(), "positions", ShaderResourceStage::RESOURCE_STAGE_INSTANCE, quadBatches[i].positions.data(), quadBatches[i].current_unit * sizeof(glm::vec4) * 6);
			RenderFunc::SetPipelineData(quadBatchPipeline.get(), "tex_coords", ShaderResourceStage::RESOURCE_STAGE_INSTANCE, quadBatches[i].tex_coords.data(), quadBatches[i].current_unit * sizeof(glm::vec4) * 6);
			RenderFunc::BindPipeline_(quadBatchPipeline.get());
			RenderFunc::BindPipeline(&g_device, quadBatchPipeline.get());
			RenderFunc::Draw(&g_device, 0, quadBatches[i].current_unit * 6);
			RenderFunc::OnDrawEnd(&g_device, quadBatchPipeline.get());
		}
	}
	
	void Renderer::RenderTexts()
	{
		textBatchPipeline = PipelineManager::get_instance()->GetPipeline("text_batch_pipeline");
		glm::mat4 proj = _camera->GetProjectionMatix() * _camera->GetViewMatrix();
		for (uint32_t i = 0; i < num_avalible_text_batch; i++)
		{
			if (textBatches[i].current_unit == 0) continue;
			for (uint32_t j = 0; j < textBatches[i].current_texture_unit; j++)
			{
				RenderFunc::SetPipelineTextureData(textBatchPipeline.get(), "u_textures" + std::to_string(j), ShaderResourceStage::RESOURCE_STAGE_GLOBAL, textBatches[i].textures[j], j);
			}
			RenderFunc::OnDrawStart(&g_device, textBatchPipeline.get());
			RenderFunc::SetPipelineData(textBatchPipeline.get(), "_projection", ShaderResourceStage::RESOURCE_STAGE_GLOBAL, &proj, sizeof(glm::mat4));
			RenderFunc::SetPipelineData(textBatchPipeline.get(), "positions", ShaderResourceStage::RESOURCE_STAGE_INSTANCE, textBatches[i].positions.data(), textBatches[i].current_unit * sizeof(glm::vec4) * 6);
			RenderFunc::SetPipelineData(textBatchPipeline.get(), "tex_coords", ShaderResourceStage::RESOURCE_STAGE_INSTANCE, textBatches[i].tex_coords.data(), textBatches[i].current_unit * sizeof(glm::vec4) * 6);
			RenderFunc::BindPipeline_(textBatchPipeline.get());
			RenderFunc::BindPipeline(&g_device, textBatchPipeline.get());
			RenderFunc::Draw(&g_device, 0, textBatches[i].current_unit * 6);
			RenderFunc::OnDrawEnd(&g_device, textBatchPipeline.get());
		}
	}

	void Renderer::RenderTextVerts()
	{
		uint32_t index = 0;
		uint32_t offset = 0;
		uint32_t start_vertex = 0;
		glm::mat4 proj = _camera->GetProjectionMatix() * _camera->GetViewMatrix();
		text_pipeline = PipelineManager::get_instance()->GetPipeline("text_pipeline");
		for (GTexture*& i : text_atlases)
		{
			std::vector<TextVertex>& vertices = text_vertices[index];
			uint32_t count = vertices.size();
			if (count == 0) continue;
			uint32_t size = count * sizeof(TextVertex);
			RenderFunc::SetBufferDataOffset(&text_buffer, vertices.data(), offset, size);

			RenderFunc::OnDrawStart(&g_device, text_pipeline.get());
			RenderFunc::SetPipelineTextureData(text_pipeline.get(), "u_texture", ShaderResourceStage::RESOURCE_STAGE_INSTANCE, i);
			RenderFunc::SetPipelineData(text_pipeline.get(), "_projection", ShaderResourceStage::RESOURCE_STAGE_GLOBAL, &proj, sizeof(glm::mat4));
			RenderFunc::BindVertexBuffer(&g_device, &text_buffer);
			RenderFunc::BindPipeline_(text_pipeline.get());
			RenderFunc::BindPipeline(&g_device, text_pipeline.get());
			RenderFunc::Draw(&g_device, start_vertex, count);
			RenderFunc::OnDrawEnd(&g_device, text_pipeline.get());

			++index;
			offset += size;
			start_vertex += count;
		}

	}

	void Renderer::RenderDebugData()
	{
		Debugger* debugger = Debugger::get_instance();
		if (debugger->IsInitialized())
		{
			Debugger::DebugRenderData& render_data = debugger->GetRenderData();
			if (render_data.vert_count <= 0) return;
			Ref<GPipeline> render_pipeline = PipelineManager::get_instance()->GetPipeline("debug_line_pipeline");

			glm::mat4 proj = _camera->GetProjectionMatix() * _camera->GetViewMatrix();
			RenderFunc::OnDrawStart(&g_device, render_pipeline.get());
			RenderFunc::BindLineWidth(&g_device, 1.0f);
			RenderFunc::SetPipelineData(render_pipeline.get(), "_projection", ShaderResourceStage::RESOURCE_STAGE_GLOBAL, &proj, sizeof(glm::mat4));
			RenderFunc::SetPipelineData(render_pipeline.get(), "positions", ShaderResourceStage::RESOURCE_STAGE_INSTANCE, render_data.positions.data(), render_data.vert_count * sizeof(glm::vec4));
			RenderFunc::BindPipeline_(render_pipeline.get());
			RenderFunc::BindPipeline(&g_device, render_pipeline.get());
			RenderFunc::Draw(&g_device, 0, render_data.vert_count);
			RenderFunc::OnDrawEnd(&g_device, render_pipeline.get());
		}
	}


	void Renderer::draw_mesh(CommandParams& params)
	{
		Mesh* _mesh = (Mesh*)params.ptrs[0];

		glm::mat4* M_model = (glm::mat4*)(params.data);
		glm::mat4 proj = _camera->GetProjectionMatix();
		glm::mat4 view = _camera->GetViewMatrix();
		glm::vec3 view_position = _camera->GetPosition();

		//TODO: Implement draw mesh

	}

	void Renderer::draw_skybox(CommandParams& params)
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
		if (current_quad_batch >= num_avalible_quad_batch)
		{
			num_avalible_quad_batch++;
			quadBatches.push_back({});
			quadBatches[current_quad_batch].current_index = 0;
			quadBatches[current_quad_batch].current_texture_unit = 0;
			quadBatches[current_quad_batch].current_unit = 0;
			quadBatches[current_quad_batch].max_texture_units = 16;
			quadBatches[current_quad_batch].max_units = 80;
			quadBatches[current_quad_batch].textures.resize(quadBatches[current_quad_batch].max_texture_units);
			quadBatches[current_quad_batch].positions.resize(quadBatches[current_quad_batch].max_units * 6);
			quadBatches[current_quad_batch].tex_coords.resize(quadBatches[current_quad_batch].max_units * 6);
			quadBatches[current_quad_batch].tex = nullptr;
		}
	}

	void Renderer::flush_current_quad_batch()
	{
		current_quad_batch++;
		create_quad_batch();
		boundQuadTextures.clear();
	}

	void Renderer::destroy_quad_batchs()
	{
		// TODO: implement batch destruction
	}

	void Renderer::create_text_batch()
	{
		if (current_text_batch >= num_avalible_text_batch)
		{
			num_avalible_text_batch++;
			textBatches.resize(num_avalible_text_batch);
			textBatches[current_text_batch].current_index = 0;
			textBatches[current_text_batch].current_texture_unit = 0;
			textBatches[current_text_batch].current_unit = 0;
			textBatches[current_text_batch].max_texture_units = 4;
			textBatches[current_text_batch].max_units = 80;
			textBatches[current_text_batch].textures.resize(textBatches[current_text_batch].max_texture_units);
			textBatches[current_text_batch].positions.resize(textBatches[current_text_batch].max_units * 6);
			textBatches[current_text_batch].tex_coords.resize(textBatches[current_text_batch].max_units * 6);
		}
	}

	void Renderer::flush_current_text_batch()
	{
		current_text_batch++;
		create_text_batch();
		boundTextTextures.clear();
	}

	void Renderer::destroy_text_batchs()
	{
		// TODO: implement batch destruction
	}

	void Renderer::BeginScene(CommandList& cmd_list, Camera* camera)
	{
		Command cmd;
		cmd.params.ptrs[0] = camera;
		cmd.func = [&](CommandParams& params) { _camera = (Camera*)params.ptrs[0]; };
		cmd_list._commands.emplace_back(cmd);
	}

	void Renderer::EndScene(CommandList& cmd_list)
	{
		Command cmd;
		cmd.func = [&](CommandParams& params) { return; }; //TODO: Add Logic to End Scene
		cmd_list._commands.emplace_back(cmd);
	}

	void Renderer::DrawMesh(CommandList& cmd_list, Ref<Mesh> _mesh, glm::mat4 model)
	{
		if (!_mesh) return;
		Command cmd;
		cmd.params.ptrs[0] = _mesh.get();
		cmd.func = BIND_RENDER_COMMAND_FN(Renderer::draw_mesh);
		cmd.params.data = (char*)MemoryManager::get_instance()->FrameAlloc(sizeof(glm::mat4));
		memcpy(cmd.params.data, &model, sizeof(glm::mat4));
		cmd_list._commands.emplace_back(cmd);
	}

	void Renderer::DrawModel(CommandList& cmd_list, Ref<Model> _model, glm::mat4 transform)
	{
		if (!_model) return;
		Command cmd;
		cmd.params.ptrs[0] = _model.get();
		cmd.func = [&](CommandParams params) {
			Model* model = (Model*)params.ptrs[0];
			Renderer::RenderObjectData data;
			data.transform = *(glm::mat4*)(params.data);
			data.material = nullptr;
			data.object = _model.get();

			m_opaqueObjects[m_opaqueObjectsSize++] = data;
		};
		cmd.params.data = (char*)MemoryManager::get_instance()->FrameAlloc(sizeof(glm::mat4));
		memcpy(cmd.params.data, &transform, sizeof(glm::mat4));
		cmd_list._commands.emplace_back(cmd);
	}

	void Renderer::DrawModel(CommandList& cmd_list, Ref<Model> _model, Ref<MaterialInstance> material, glm::mat4 transform)
	{
		if (!_model || !material) return;
		Command cmd;
		cmd.params.ptrs[0] = _model.get();
		cmd.params.ptrs[1] = material.get();
		cmd.func = [&](CommandParams params) {
			Model* model = (Model*)params.ptrs[0];
			MaterialInstance* mat = (MaterialInstance*)params.ptrs[1];
			Renderer::RenderObjectData data;
			data.transform = *(glm::mat4*)(params.data);
			data.material = mat;
			data.object = model;

			m_opaqueObjects[m_opaqueObjectsSize++] = data;
		};
		cmd.params.data = (char*)MemoryManager::get_instance()->FrameAlloc(sizeof(glm::mat4));
		memcpy(cmd.params.data, &transform, sizeof(glm::mat4));
		cmd_list._commands.emplace_back(cmd);
	}

	void Renderer::DrawSky(CommandList& cmd_list, SkyBox* sky)
	{

		Command cmd;
		cmd.params.ptrs[0] = sky;
		cmd.func = BIND_RENDER_COMMAND_FN(Renderer::draw_skybox);
		cmd_list._commands.emplace_back(cmd);
	}

	//FIX: Find a way if fit the types light in an array {DIRECTIONAL - POINT - SPOT}
	void Renderer::DrawLight(CommandList& cmd_list, Ref<Mesh> _mesh, Light& _light, LightType light_type)
	{
		/*Command cmd;
		cmd.params.ptrs[0] = _mesh.get();
		cmd.params.val[0] = light_type;
		cmd.params.data = (char*)MemoryManager::get_instance()->FrameAlloc(sizeof(Light));
		memcpy(cmd.params.data, &_light, sizeof(Light));

		cmd.func = [&](CommandParams& params) {
			Mesh* _mesh = (Mesh*)params.ptrs[0];
			uint32_t light_type_ = params.val[0];
			Light* _light = (Light*)params.data;
			uint32_t light_index = 0;
			if (light_type_ == LightType::DIRECTIONAL)
			{
				auto it = lights.begin();
				uint32_t dir_light_count = light_data.x;
				light_index = dir_light_count;
				lights.insert(it + dir_light_count, *_light);
				light_data.x++;
			}
			else if (light_type_ == LightType::POINT)
			{
				auto it = lights.begin();
				uint32_t point_light_count = light_data.x + light_data.y;
				light_index = point_light_count;
				lights.insert(it + point_light_count, *_light);
				light_data.y++;
			}
			else if (light_type_ == LightType::SPOT)
			{
				auto it = lights.begin();
				uint32_t spot_light_count = light_data.x + light_data.y + light_data.z;
				light_index = spot_light_count;
				lights.insert(it + spot_light_count, *_light);
				light_data.z++;
			}

			for (Ref<Model> _model : _mesh->GetModels())
			{
				m_meshedLights[m_meshLightSize++] = std::make_pair(light_index, _model.get());
			}
		};

		cmd_list._commands.push_back(cmd);*/
	}

	//FIX: Find a way if fit the types light in an array {DIRECTIONAL - POINT - SPOT}
	void Renderer::AddLight(CommandList& cmd_list, Light& _light, LightType light_type)
	{
		Command cmd;
		cmd.params.val[0] = light_type;
		cmd.params.data = (char*)MemoryManager::get_instance()->FrameAlloc(sizeof(Light));
		memcpy(cmd.params.data, &_light, sizeof(Light));

		cmd.func = [&](CommandParams& params) {
			uint32_t light_type_ = params.val[0];
			Light* _light = (Light*)params.data;
			uint32_t light_index = 0;
			if (light_type_ == LightType::DIRECTIONAL)
			{
				auto it = lights.begin();
				uint32_t dir_light_count = light_data.x;
				light_index = dir_light_count;
				lights.insert(it + dir_light_count, *_light);
				light_data.x++;
			}
			else if (light_type_ == LightType::POINT)
			{
				auto it = lights.begin();
				uint32_t point_light_count = light_data.x + light_data.y;
				light_index = point_light_count;
				lights.insert(it + point_light_count, *_light);
				light_data.y++;
			}
			else if (light_type_ == LightType::SPOT)
			{
				auto it = lights.begin();
				uint32_t spot_light_count = light_data.x + light_data.y + light_data.z;
				light_index = spot_light_count;
				lights.insert(it + spot_light_count, *_light);
				light_data.z++;
			}

			light_positions.insert(light_positions.begin() + light_index, _light->position);
			light_directions.insert(light_directions.begin() + light_index, _light->direction);
			light_colors.insert(light_colors.begin() + light_index, _light->color);
			light_params1s.insert(light_params1s.begin() + light_index, _light->params1);
			light_params2s.insert(light_params2s.begin() + light_index, _light->params2);
		};

		cmd_list._commands.push_back(cmd);
	}

	void Renderer::DrawDebugLine(CommandList& cmd_list, glm::vec3 from, glm::vec3 to, uint32_t color)
	{
		Command cmd;
		uint32_t data_size = (sizeof(glm::vec3) * 2) + sizeof(uint32_t);
		cmd.params.data = (char*)MemoryManager::get_instance()->FrameAlloc(data_size);
		memcpy(cmd.params.data, &from, sizeof(glm::vec3));
		memcpy((cmd.params.data + sizeof(glm::vec3)), &to, sizeof(glm::vec3));
		memcpy((cmd.params.data + (data_size - sizeof(uint32_t))), &color, sizeof(uint32_t));
		cmd.func = [](CommandParams params) {

			Debugger* debugger = Debugger::get_instance();
			if (!debugger->IsInitialized())
			{
				return;
			}
			glm::vec3& from = *(glm::vec3*)(params.data);
			glm::vec3& to = *(glm::vec3*)(params.data + sizeof(glm::vec3));
			uint32_t& color = *(uint32_t*)(params.data + sizeof(glm::vec3) + sizeof(glm::vec3));

			debugger->AddDebugLine(from, to, color);


		};
		cmd_list._commands.emplace_back(cmd);
	}

	void Renderer::DrawDebugLine(CommandList& cmd_list, glm::vec3 p0, glm::vec3 p1, glm::mat4 transform, uint32_t color)
	{
		Command cmd;
		uint32_t data_size = (sizeof(glm::vec3) * 2) + sizeof(glm::mat4) + sizeof(uint32_t);
		cmd.params.data = (char*)MemoryManager::get_instance()->FrameAlloc(data_size);
		memcpy(cmd.params.data, &p0, sizeof(glm::vec3));
		memcpy((cmd.params.data + sizeof(glm::vec3)), &p1, sizeof(glm::vec3));
		memcpy((cmd.params.data + (sizeof(glm::vec3) * 2)), &transform, sizeof(glm::mat4));
		memcpy((cmd.params.data + (data_size - sizeof(uint32_t))), &color, sizeof(uint32_t));
		cmd.func = [](CommandParams params) {
			Debugger* debugger = Debugger::get_instance();
			if (!debugger->IsInitialized())
			{
				return;
			}
			glm::vec3& p0 = *(glm::vec3*)(params.data);
			glm::vec3& p1 = *(glm::vec3*)(params.data + sizeof(glm::vec3));
			glm::mat4& transform = *(glm::mat4*)(params.data + (sizeof(glm::vec3) * 2));
			uint32_t& color = *(uint32_t*)(params.data + sizeof(glm::vec3) + sizeof(glm::vec3) + sizeof(glm::mat4));
			debugger->AddDebugLine(p0, p1, transform, color);

		};
	}

	void Renderer::DrawDebugCircle(CommandList& cmd_list, float radius, uint32_t steps, glm::mat4 transform, uint32_t color)
	{
		float ar = (glm::pi<float>() * 2.0f) / (float)steps;
		float theta = ar * (float)0;
		float x = radius * glm::cos(theta);
		float y = radius * glm::sin(theta);

		glm::vec4 previous_point = glm::vec4(x, y, 0.0f, 1.0f);
		for (uint32_t i = 1; i <= steps; i++)
		{
			float theta = ar * (float)i;
			float x = radius * glm::cos(theta);
			float y = radius * glm::sin(theta);

			glm::vec4 point = glm::vec4(x, y, 0.0f, 1.0f);
			DrawDebugLine(cmd_list, glm::vec3(transform * previous_point), glm::vec3(transform * point), color);
			previous_point = point;
		}
	}

	void Renderer::DrawDebugSphere(CommandList& cmd_list, float radius, uint32_t steps, glm::mat4 transform, uint32_t color)
	{
		DrawDebugCircle(cmd_list, radius, steps, transform, color);
		glm::mat4 new_transform = glm::rotate(transform, glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		DrawDebugCircle(cmd_list, radius, steps, new_transform, color);
		new_transform = glm::rotate(transform, glm::radians(-45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		DrawDebugCircle(cmd_list, radius, steps, new_transform, color);
		new_transform = glm::rotate(transform, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		DrawDebugCircle(cmd_list, radius, steps, new_transform, color);
	}

	void Renderer::DrawString(CommandList& cmd_list, Ref<Font> font, const std::string& text, glm::vec3 color, glm::mat4 _transform)
	{

		if (!font) return;
		
		if (text_verts) DrawString_(font.get(), text, color, _transform);
		else DrawString(font.get(), text, _transform);

	}

	void Renderer::DrawImage(CommandList& cmd_list, Ref<GTexture> texture, glm::mat4 _transform)
	{
		DrawQuad_(_transform, texture);
	}

	void Renderer::DrawImage(CommandList& cmd_list, Ref<GTexture> texture, glm::mat4 _transform, uint32_t color)
	{
		DrawQuad_(_transform, texture, color);
	}

	std::string Renderer::GetRenderPassName(GRenderPass* pass)
	{
		std::string result;

		for (auto& i : m_avaliablePasses)
		{
			if (i.second == pass)
			{
				result = i.first;
			}
		}

		return result;
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
		static Renderer* s_instance = new Renderer;
		return s_instance;
	}



}