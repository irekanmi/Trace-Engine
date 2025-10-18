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
#include "resource/GenericAssetManager.h"
#include "core/memory/MemoryManager.h"
#include "debug/Debugger.h"
#include "core/defines.h"
#include "render/SkinnedModel.h"
#include "resource/DefaultAssetsManager.h"
#include "particle_effects/ParticleEffect.h"

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

	static Model quad_model;

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
		for (uint32_t i = 0; i < num_render_graphs; i++)
		{
			RenderGraphFrameData& graph_data = m_renderGraphsData[i];

			for (auto& tex_ins : graph_data.quad_instances)
			{
				tex_ins.second.transforms.clear();
				tex_ins.second.colors.clear();
			}
			
			for (auto& tex_ins : graph_data.text_instances)
			{
				tex_ins.second.positions.clear();
				tex_ins.second.tex_coords.clear();
			}

			graph_data.particle_effects.clear();
		}


		Debugger* debugger = Debugger::get_instance();
		if (debugger->IsInitialized())
		{
			Debugger::DebugRenderData& render_data = debugger->GetRenderData();
			render_data.positions.clear();
			render_data.vert_count = 0;
		}


		UIFunc::UIEndFrame();
		RenderFunc::EndFrame(&g_device);

		
	}


	void Renderer::Start()
	{



		EventsSystem::get_instance()->AddEventListener(EventType::TRC_WND_RESIZE, BIND_EVENT_FN(Renderer::OnEvent));
		// Temp _____
		EventsSystem::get_instance()->AddEventListener(EventType::TRC_KEY_PRESSED, BIND_EVENT_FN(Renderer::OnEvent));
		//_____________

		exposure = 0.9f;
		num_render_graphs = MAX_RENDER_GRAPH;
		m_renderGraphsData.resize(num_render_graphs);

		if (m_composer)
		{
			m_composer->Init(this);
		}
		else
		{
			m_composer = new RenderComposer();
			m_composer->Init(this);
		}

		for (uint32_t i = 0; i < num_render_graphs; i++)
		{
			RenderGraphFrameData& graph_data = m_renderGraphsData[i];

			std::vector<RenderObjectData>& m_opaqueObjects = graph_data.m_opaqueObjects;
			uint32_t& m_opaqueObjectsSize = graph_data.m_opaqueObjectsSize;

			m_opaqueObjects.reserve(1024);
		}

		// --------------------------------------------------------
				
		//---------------------------------------------------------------------------------------------



		std::vector<Vertex> vertices(4);
		vertices[0].pos = glm::vec3(-1.0f, -1.0f, 0.0f);
		vertices[0].texCoord = glm::vec2(0.0f, 0.0f);
		
		vertices[1].pos = glm::vec3(1.0f, -1.0f, 0.0f);
		vertices[1].texCoord = glm::vec2(1.0f, 0.0f);
		
		vertices[2].pos = glm::vec3(1.0f, 1.0f, 0.0f);
		vertices[2].texCoord = glm::vec2(1.0f, 1.0f);
		
		vertices[3].pos = glm::vec3(-1.0f, 1.0f, 0.0f);
		vertices[3].texCoord = glm::vec2(0.0f, 1.0f);

		std::vector<uint32_t> indices = {
			0, 1, 2,
			0, 2, 3
		};

		generateVertexTangent(vertices, indices);

		quad_model.Create(vertices, indices);


		if (AppSettings::is_editor)
		{
			UIFuncLoader::LoadImGuiFunc();
			UIFunc::InitUIRenderBackend(Application::get_instance(), Renderer::get_instance());
		}

	}

	void Renderer::End()
	{
		quad_model.Destroy();

		//Text Rendering -------------------------
		for (uint32_t i = 0; i < num_render_graphs; i++)
		{
			RenderGraphFrameData& graph_data = m_renderGraphsData[i];
		}

		// ---------------------------------------

		

		m_composer->Shutdowm();
		delete m_composer;
		m_composer = nullptr;
		RenderFunc::DestroySwapchain(&m_swapChain);

	}

	void Renderer::ShutDown()
	{
		if (AppSettings::is_editor)
		{
			UIFunc::ShutdownUIRenderBackend();
		}

		RenderFunc::DestroyDevice(&g_device);
		RenderFunc::DestroyContext(&g_context);
	}
	 
	void Renderer::OnEvent(Event* p_event)
	{

		switch (p_event->GetEventType())
		{
		case EventType::TRC_KEY_PRESSED:
		{
			KeyPressed* press = reinterpret_cast<KeyPressed*>(p_event);

			/*if (press->GetKeyCode() == KEY_O)
			{
				exposure += 0.4f;
			}
			else if (press->GetKeyCode() == KEY_P)
			{
				exposure -= 0.4f;
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
			}*/

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


	Model& Renderer::GetQuadModel()
	{
		return quad_model;
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

			m_composer->Render(deltaTime, frame_settings);

			EndFrame();
			RenderFunc::PresentSwapchain(&m_swapChain);

			for (uint32_t i = 0; i < num_render_graphs; i++)
			{
				RenderGraphFrameData& graph_data = m_renderGraphsData[i];

				graph_data.num_non_shadowed_point_lights = 0;
				graph_data.num_non_shadowed_sun_lights = 0;
				graph_data.num_non_shadowed_spot_lights = 0;

				graph_data.num_shadowed_sun_lights = 0;
				graph_data.num_shadowed_spot_lights = 0;
				graph_data.num_shadowed_point_lights = 0;

				graph_data.sun_lights.clear();
				graph_data.spot_lights.clear();
				graph_data.point_lights.clear();

				graph_data.shadow_casters.clear();
			}
			m_composer->DestroyGraphs();
		}
		m_listCount = 0;

		for (uint32_t i = 0; i < num_render_graphs; i++)
		{
			RenderGraphFrameData& graph_data = m_renderGraphsData[i];
			graph_data.m_opaqueObjects.clear();
			graph_data.m_transparentUnLitObjects.clear();
			graph_data.m_opaqueSkinnedObjects.clear();
			graph_data.shadow_casters.clear();
			graph_data.skinned_shadow_casters.clear();

		}

		

	}

	void Renderer::DrawQuad(int32_t render_graph_index)
	{
		TRC_ASSERT(false, "Funtion {} has not been implemented", __FUNCTION__);
	}

	void Renderer::DrawQuad_(glm::mat4 _transform, Ref<GTexture> texture, int32_t render_graph_index)
	{	
		
		DrawQuad_(_transform, texture, TRC_COL32_BLACK, render_graph_index);

	}

	void Renderer::DrawQuad_(glm::mat4 _transform, Ref<GTexture> texture, uint32_t color, int32_t render_graph_index)
	{
		RenderGraphFrameData& graph_data = m_renderGraphsData[render_graph_index];
		

		graph_data.quad_instances[texture.get()].transforms.emplace_back(_transform);
		graph_data.quad_instances[texture.get()].colors.emplace_back(color);

		return;
	}

	void Renderer::DrawString_(Font* font, const std::string& text, glm::vec3 color, glm::mat4 _transform, int32_t render_graph_index)
	{
		RenderGraphFrameData& graph_data = m_renderGraphsData[render_graph_index];
		

		GTexture* texture = font->GetAtlas().get();

		FontFunc::ComputeTextString(font, text, graph_data.text_instances[texture].positions, graph_data.text_instances[texture].tex_coords, _transform, color);

		return;
	}

	void Renderer::RenderOpaqueObjects(int32_t render_graph_index)
	{
		RenderGraphFrameData& graph_data = m_renderGraphsData[render_graph_index];

		std::vector<RenderObjectData>& m_opaqueObjects = graph_data.m_opaqueObjects;
		uint32_t& m_opaqueObjectsSize = graph_data.m_opaqueObjectsSize;
		Camera* _camera = graph_data._camera;

		if (!_camera)
		{
			return;
		}

		glm::mat4 proj = _camera->GetProjectionMatix();
		glm::mat4 view = _camera->GetViewMatrix();
		glm::vec3 view_position = _camera->GetPosition();
		glm::mat4 view_proj = proj * view;

		for (RenderObjectData& data : m_opaqueObjects)
		{

			glm::mat4* M_model = &data.transform;
			Model* _model = data.object;
			MaterialInstance* _mi = data.material? data.material : DefaultAssetsManager::default_material.get();
			Ref<GPipeline> sp = _mi->GetRenderPipline();

			RenderFunc::OnDrawStart(&g_device, sp.get());
			RenderFunc::ApplyMaterial(_mi, render_graph_index);
			RenderFunc::SetPipelineData(sp.get(), "_projection", ShaderResourceStage::RESOURCE_STAGE_GLOBAL, &proj, sizeof(glm::mat4), 0, render_graph_index);
			RenderFunc::SetPipelineData(sp.get(), "_view", ShaderResourceStage::RESOURCE_STAGE_GLOBAL, &view, sizeof(glm::mat4), 0, render_graph_index);
			RenderFunc::SetPipelineData(sp.get(), "_view_position", ShaderResourceStage::RESOURCE_STAGE_GLOBAL, &view_position, sizeof(glm::vec3), 0, render_graph_index);
			RenderFunc::SetPipelineData(sp.get(), "_model", ShaderResourceStage::RESOURCE_STAGE_INSTANCE, M_model, sizeof(glm::mat4), 0, render_graph_index);
			RenderFunc::BindPipeline(&g_device, sp.get());
			RenderFunc::BindPipeline_(sp.get(), render_graph_index);
			RenderFunc::BindVertexBuffer(&g_device, _model->GetVertexBuffer());
			RenderFunc::BindIndexBuffer(&g_device, _model->GetIndexBuffer());

			RenderFunc::DrawIndexed(&g_device, 0, _model->GetIndexCount());
			RenderFunc::OnDrawEnd(&g_device, sp.get());
		}


		for (RenderSkinnedObjectData& data : graph_data.m_opaqueSkinnedObjects)
		{

			glm::mat4* M_model = &data.transform;
			SkinnedModel* _model = data.object;
			if (!data.material)
			{
				continue;
			}

			if (!_model)
			{
				continue;
			}

			// TODO: Find out why an invalid model will be a sent as a valid draw call
			if (_model->GetVertices().size() == 0 || _model->GetIndices().size() == 0)
			{
				continue;
			}

			MaterialInstance* _mi = data.material;
			Ref<GPipeline> sp = _mi->GetRenderPipline();

			RenderFunc::OnDrawStart(&g_device, sp.get());
			RenderFunc::ApplyMaterial(_mi, render_graph_index);
			RenderFunc::SetPipelineData(sp.get(), "_projection", ShaderResourceStage::RESOURCE_STAGE_GLOBAL, &proj, sizeof(glm::mat4), 0, render_graph_index);
			RenderFunc::SetPipelineData(sp.get(), "_view", ShaderResourceStage::RESOURCE_STAGE_GLOBAL, &view, sizeof(glm::mat4), 0, render_graph_index);
			RenderFunc::SetPipelineData(sp.get(), "_view_position", ShaderResourceStage::RESOURCE_STAGE_GLOBAL, &view_position, sizeof(glm::vec3), 0, render_graph_index);
			RenderFunc::SetPipelineData(sp.get(), "_bone_matrices", ShaderResourceStage::RESOURCE_STAGE_INSTANCE, data.bone_transforms, sizeof(glm::mat4) * data.bone_count, 0, render_graph_index);
			RenderFunc::BindPipeline(&g_device, sp.get());
			RenderFunc::BindPipeline_(sp.get(), render_graph_index);
			RenderFunc::BindVertexBuffer(&g_device, _model->GetVertexBuffer());
			RenderFunc::BindIndexBuffer(&g_device, _model->GetIndexBuffer());

			RenderFunc::DrawIndexed(&g_device, 0, _model->GetIndexCount());
			RenderFunc::OnDrawEnd(&g_device, sp.get());
		}

	}

	void Renderer::RenderTransparentUnLitObjects(int32_t render_graph_index, RenderGraph* render_graph, uint32_t screen_color_index)
	{
		RenderGraphFrameData& graph_data = m_renderGraphsData[render_graph_index];

		std::vector<RenderObjectData>& m_transparentUnLitObjects = graph_data.m_transparentUnLitObjects;
		Camera* _camera = graph_data._camera;

		if (!_camera)
		{
			return;
		}

		glm::mat4 proj = _camera->GetProjectionMatix();
		glm::mat4 view = _camera->GetViewMatrix();
		glm::vec3 view_position = _camera->GetPosition();
		glm::mat4 view_proj = proj * view;

		glm::vec4 _time_values(0.0f);
		_time_values.x = Application::get_instance()->GetClock().GetElapsedTime();

		for (RenderObjectData& data : m_transparentUnLitObjects)
		{

			glm::mat4* M_model = &data.transform;
			Model* _model = data.object;
			if (!data.material)
			{
				continue;
			}
			MaterialInstance* _mi = data.material;
			Ref<GPipeline> sp = _mi->GetRenderPipline();

			RenderFunc::OnDrawStart(&g_device, sp.get());
			RenderFunc::ApplyMaterial(_mi, render_graph_index);
			RenderFunc::SetPipelineData(sp.get(), "_projection", ShaderResourceStage::RESOURCE_STAGE_GLOBAL, &proj, sizeof(glm::mat4), 0, render_graph_index);
			RenderFunc::SetPipelineData(sp.get(), "_view", ShaderResourceStage::RESOURCE_STAGE_GLOBAL, &view, sizeof(glm::mat4), 0, render_graph_index);
			RenderFunc::SetPipelineData(sp.get(), "_time_values", ShaderResourceStage::RESOURCE_STAGE_GLOBAL, &_time_values, sizeof(glm::vec4), 0, render_graph_index);
			RenderFunc::SetPipelineData(sp.get(), "_view_position", ShaderResourceStage::RESOURCE_STAGE_GLOBAL, &view_position, sizeof(glm::vec3), 0, render_graph_index);
			RenderFunc::SetPipelineData(sp.get(), "_model", ShaderResourceStage::RESOURCE_STAGE_INSTANCE, M_model, sizeof(glm::mat4), 0, render_graph_index);
			RenderFunc::BindRenderGraphTexture(
				render_graph,
				sp.get(),
				"_screen_color",
				ShaderResourceStage::RESOURCE_STAGE_GLOBAL,
				render_graph->GetResource_ptr(screen_color_index),
				render_graph_index
			);
			RenderFunc::BindPipeline(&g_device, sp.get());
			RenderFunc::BindPipeline_(sp.get(), render_graph_index);
			RenderFunc::BindVertexBuffer(&g_device, _model->GetVertexBuffer());
			RenderFunc::BindIndexBuffer(&g_device, _model->GetIndexBuffer());

			RenderFunc::DrawIndexed(&g_device, 0, _model->GetIndexCount());
			RenderFunc::OnDrawEnd(&g_device, sp.get());
		}

	}

	void Renderer::RenderQuads(int32_t render_graph_index)
	{

		RenderGraphFrameData& graph_data = m_renderGraphsData[render_graph_index];

		Camera* _camera = graph_data._camera;

		if (!_camera)
		{
			return;
		}

		glm::mat4 proj = _camera->GetProjectionMatix() * _camera->GetViewMatrix();
		for (auto& tex_ins : graph_data.quad_instances)
		{
			if (tex_ins.second.transforms.empty())
			{
				continue;
			}

			int32_t remaining_particles = tex_ins.second.transforms.size();
			uint32_t offset = 0;

			while (remaining_particles > 0)
			{
				uint32_t particles_to_render = remaining_particles > MAX_QUAD_INSTANCE ? MAX_QUAD_INSTANCE : remaining_particles;

				RenderFunc::OnDrawStart(&g_device, DefaultAssetsManager::quad_pipeline.get());
				RenderFunc::SetPipelineTextureData(DefaultAssetsManager::quad_pipeline.get(), "u_textures", ShaderResourceStage::RESOURCE_STAGE_INSTANCE, tex_ins.first, render_graph_index);
				RenderFunc::SetPipelineData(DefaultAssetsManager::quad_pipeline.get(), "_projection", ShaderResourceStage::RESOURCE_STAGE_GLOBAL, &proj, sizeof(glm::mat4), 0, render_graph_index);
				RenderFunc::SetPipelineData(DefaultAssetsManager::quad_pipeline.get(), "transforms", ShaderResourceStage::RESOURCE_STAGE_INSTANCE, offset + tex_ins.second.transforms.data(), particles_to_render * sizeof(glm::mat4), 0, render_graph_index);
				RenderFunc::SetPipelineData(DefaultAssetsManager::quad_pipeline.get(), "colors", ShaderResourceStage::RESOURCE_STAGE_INSTANCE, offset + tex_ins.second.colors.data(), particles_to_render * sizeof(uint32_t), 0, render_graph_index);
				RenderFunc::BindPipeline_(DefaultAssetsManager::quad_pipeline.get(), render_graph_index);
				RenderFunc::BindPipeline(&g_device, DefaultAssetsManager::quad_pipeline.get());
				RenderFunc::BindVertexBuffer(&g_device, quad_model.GetVertexBuffer());
				RenderFunc::BindIndexBuffer(&g_device, quad_model.GetIndexBuffer());
				RenderFunc::DrawIndexedInstanced(&g_device, 0, quad_model.GetIndexCount(), particles_to_render);
				RenderFunc::OnDrawEnd(&g_device, DefaultAssetsManager::quad_pipeline.get());

				remaining_particles -= MAX_QUAD_INSTANCE;
				offset += MAX_QUAD_INSTANCE;
			}

		}
	}

	void Renderer::RenderTextVerts(int32_t render_graph_index)
	{
		RenderGraphFrameData& graph_data = m_renderGraphsData[render_graph_index];

		Camera* _camera = graph_data._camera;

		if (!_camera)
		{
			return;
		}

		glm::mat4 proj = _camera->GetProjectionMatix() * _camera->GetViewMatrix();

		for (auto& tex_ins : graph_data.text_instances)
		{
			
			if (tex_ins.second.positions.empty())
			{
				continue;
			}

			int32_t remaining_particles = tex_ins.second.positions.size() / 4;
			uint32_t offset = 0;

			while (remaining_particles > 0)
			{
				uint32_t particles_to_render = remaining_particles > (MAX_QUAD_INSTANCE / 4) ? (MAX_QUAD_INSTANCE / 4): remaining_particles;

				RenderFunc::OnDrawStart(&g_device, DefaultAssetsManager::text_pipeline.get());
				RenderFunc::SetPipelineTextureData(DefaultAssetsManager::text_pipeline.get(), "u_texture", ShaderResourceStage::RESOURCE_STAGE_INSTANCE, tex_ins.first, render_graph_index);
				RenderFunc::SetPipelineData(DefaultAssetsManager::text_pipeline.get(), "_projection", ShaderResourceStage::RESOURCE_STAGE_GLOBAL, &proj, sizeof(glm::mat4), 0, render_graph_index);
				RenderFunc::SetPipelineData(DefaultAssetsManager::text_pipeline.get(), "positions", ShaderResourceStage::RESOURCE_STAGE_INSTANCE, (offset * 4) + tex_ins.second.positions.data(), particles_to_render * 4 * sizeof(glm::vec4), 0, render_graph_index);
				RenderFunc::SetPipelineData(DefaultAssetsManager::text_pipeline.get(), "tex_coords", ShaderResourceStage::RESOURCE_STAGE_INSTANCE, (offset * 4) + tex_ins.second.tex_coords.data(), particles_to_render * 4 * sizeof(glm::vec4), 0, render_graph_index);
				RenderFunc::BindPipeline_(DefaultAssetsManager::text_pipeline.get(), render_graph_index);
				RenderFunc::BindPipeline(&g_device, DefaultAssetsManager::text_pipeline.get());
				RenderFunc::BindVertexBuffer(&g_device, quad_model.GetVertexBuffer());
				RenderFunc::BindIndexBuffer(&g_device, quad_model.GetIndexBuffer());
				RenderFunc::DrawIndexedInstanced(&g_device, 0, quad_model.GetIndexCount(), particles_to_render);
				RenderFunc::OnDrawEnd(&g_device, DefaultAssetsManager::text_pipeline.get());

				remaining_particles -= (MAX_QUAD_INSTANCE / 4);
				offset += (MAX_QUAD_INSTANCE / 4);
			}
		}

	}

	void Renderer::RenderDebugData(int32_t render_graph_index)
	{
		RenderGraphFrameData* graph_data = GetRenderGraphData(render_graph_index);
		Debugger* debugger = Debugger::get_instance();
		if (debugger->IsInitialized())
		{
			Debugger::DebugRenderData& render_data = debugger->GetRenderData();
			if (render_data.vert_count <= 0) return;
			Ref<GPipeline> render_pipeline = DefaultAssetsManager::debug_line_pipeline;

			if (!graph_data->_camera)
			{
				return;
			}

			glm::mat4 proj = graph_data->_camera->GetProjectionMatix() * graph_data->_camera->GetViewMatrix();
			RenderFunc::OnDrawStart(&g_device, render_pipeline.get());
			RenderFunc::BindLineWidth(&g_device, 1.0f);
			RenderFunc::SetPipelineData(render_pipeline.get(), "_projection", ShaderResourceStage::RESOURCE_STAGE_GLOBAL, &proj, sizeof(glm::mat4), 0, render_graph_index);
			RenderFunc::SetPipelineData(render_pipeline.get(), "positions", ShaderResourceStage::RESOURCE_STAGE_INSTANCE, render_data.positions.data(), render_data.vert_count * sizeof(glm::vec4), 0, render_graph_index);
			RenderFunc::BindPipeline_(render_pipeline.get(), render_graph_index);
			RenderFunc::BindPipeline(&g_device, render_pipeline.get());
			RenderFunc::Draw(&g_device, 0, render_data.vert_count);
			RenderFunc::OnDrawEnd(&g_device, render_pipeline.get());
		}
	}

	void Renderer::RenderParticles(int32_t render_graph_index)
	{
		RenderGraphFrameData& graph_data = m_renderGraphsData[render_graph_index];

		for (auto& i : graph_data.particle_effects)
		{
			i->Render(graph_data._camera, render_graph_index);
		}
	}


	void Renderer::draw_mesh(CommandParams& params, int32_t render_graph_index)
	{
		Mesh* _mesh = (Mesh*)params.ptrs[0];

		glm::mat4* M_model = (glm::mat4*)(params.data);
		//glm::mat4 proj = _camera->GetProjectionMatix();
		//glm::mat4 view = _camera->GetViewMatrix();
		//glm::vec3 view_position = _camera->GetPosition();

		//TODO: Implement draw mesh

	}

	void Renderer::draw_skybox(CommandParams& params)
	{
		/*if (current_sky_box)
		{
			TRC_WARN("Only sky can be drawn per frame {}", __FUNCTION__);
			return;
		}
		current_sky_box = (SkyBox*)params.ptrs[0];*/

	}

	void Renderer::BeginScene(CommandList& cmd_list, Camera* camera, int32_t render_graph_index)
	{
		Command cmd;
		cmd.params.ptrs[0] = camera;
		cmd.params.val[0] = render_graph_index;
		RenderGraphFrameData& graph_data = m_renderGraphsData[render_graph_index];
		graph_data._camera = camera;
		//NOTE: Remove it because it affects render functions that doesn't send commands
		/*cmd.func = [&](CommandParams& params) 
		{
			RenderGraphFrameData& graph_data = m_renderGraphsData[params.val[0]];
			graph_data._camera = (Camera*)params.ptrs[0]; 
		};*/
		
		//cmd_list._commands.emplace_back(cmd);
	}

	void Renderer::EndScene(CommandList& cmd_list, int32_t render_graph_index)
	{
		Command cmd;
		cmd.func = [&](CommandParams& params) { return; }; //TODO: Add Logic to End Scene
		cmd_list._commands.emplace_back(cmd);
	}

	void Renderer::DrawMesh(CommandList& cmd_list, Ref<Mesh> _mesh, glm::mat4 model, int32_t render_graph_index)
	{
		if (!_mesh) return;
		Command cmd;
		cmd.params.ptrs[0] = _mesh.get();
		cmd.func = BIND_RENDER_COMMAND_FN(Renderer::draw_mesh);
		cmd.params.data = (char*)MemoryManager::get_instance()->FrameAlloc(sizeof(glm::mat4));
		cmd.params.val[0] = render_graph_index;
		memcpy(cmd.params.data, &model, sizeof(glm::mat4));
		cmd_list._commands.emplace_back(cmd);
	}

	void Renderer::DrawModel(CommandList& cmd_list, Ref<Model> _model, glm::mat4 transform, int32_t render_graph_index)
	{
		if (!_model) return;
		Command cmd;
		cmd.params.ptrs[0] = _model.get();
		cmd.params.data = (char*)MemoryManager::get_instance()->FrameAlloc(sizeof(glm::mat4));
		cmd.params.val[0] = render_graph_index;

		cmd.func = [&](CommandParams params) {
			RenderGraphFrameData& graph_data = m_renderGraphsData[params.val[0]];

			Model* model = (Model*)params.ptrs[0];
			RenderObjectData data;
			data.transform = *(glm::mat4*)(params.data);
			data.material = nullptr;
			data.object = _model.get();

			graph_data.m_opaqueObjects.push_back(data);
		};
		memcpy(cmd.params.data, &transform, sizeof(glm::mat4));
		cmd_list._commands.emplace_back(cmd);
	}

	void Renderer::DrawModel(CommandList& cmd_list, Ref<Model> _model, Ref<MaterialInstance> material, glm::mat4 transform, bool cast_shadow, int32_t render_graph_index)
	{
		if (!_model || !material)
		{
			return;
		}
		Command cmd;
		cmd.params.ptrs[0] = _model.get();
		cmd.params.ptrs[1] = material.get();
		cmd.params.val[0] = render_graph_index;
		cmd.params.val[1] = cast_shadow ? 1 : 0;
		cmd.func = [&](CommandParams params) {
			RenderGraphFrameData& graph_data = m_renderGraphsData[params.val[0]];

			Model* model = (Model*)params.ptrs[0];
			MaterialInstance* mat = (MaterialInstance*)params.ptrs[1];
			RenderObjectData data;
			data.transform = *(glm::mat4*)(params.data);
			data.material = mat;
			data.object = model;

			switch (mat->GetType())
			{
			case MaterialType::OPAQUE_LIT:
			{
				graph_data.m_opaqueObjects.push_back(data);
				break;
			}
			case MaterialType::TRANSPARENT_UNLIT:
			{
				graph_data.m_transparentUnLitObjects.push_back(data);
				break;
			}
			}


			bool cast_shadow = (params.val[1] == 1);

			if (cast_shadow)
			{
				graph_data.shadow_casters.push_back(data);
			}
		};
		cmd.params.data = (char*)MemoryManager::get_instance()->FrameAlloc(sizeof(glm::mat4));
		memcpy(cmd.params.data, &transform, sizeof(glm::mat4));
		cmd_list._commands.emplace_back(cmd);
	}

	void Renderer::DrawSkinnedModel(CommandList& cmd_list, Ref<SkinnedModel> _model, Ref<MaterialInstance> material, glm::mat4 transform, glm::mat4* bone_transforms, uint32_t bone_count, bool cast_shadow, int32_t render_graph_index)
	{
		if (!_model || !material)
		{
			return;
		}
		Command cmd;
		cmd.params.ptrs[0] = _model.get();
		cmd.params.ptrs[1] = material.get();
		cmd.params.ptrs[2] = bone_transforms;
		cmd.params.val[0] = render_graph_index;
		cmd.params.val[1] = cast_shadow ? 1 : 0;
		cmd.params.val[2] = bone_count;
		cmd.func = [&](CommandParams params) {
			RenderGraphFrameData& graph_data = m_renderGraphsData[params.val[0]];

			SkinnedModel* model = (SkinnedModel*)params.ptrs[0];
			MaterialInstance* mat = (MaterialInstance*)params.ptrs[1];
			RenderSkinnedObjectData data;
			data.transform = *(glm::mat4*)(params.data);
			data.material = mat;
			data.object = model;
			data.bone_count = params.val[2];
			data.bone_transforms = (glm::mat4*)params.ptrs[2];

			switch (mat->GetType())
			{
			case MaterialType::OPAQUE_LIT:
			{
				graph_data.m_opaqueSkinnedObjects.push_back(data);
				break;
			}
			}

			bool cast_shadow = (params.val[1] == 1);

			if (cast_shadow)
			{
				graph_data.skinned_shadow_casters.push_back(data);
			}
		};
		cmd.params.data = (char*)MemoryManager::get_instance()->FrameAlloc(sizeof(glm::mat4));
		memcpy(cmd.params.data, &transform, sizeof(glm::mat4));
		cmd_list._commands.emplace_back(cmd);
	}

	void Renderer::DrawSky(CommandList& cmd_list, SkyBox* sky, int32_t render_graph_index)
	{

		Command cmd;
		cmd.params.ptrs[0] = sky;
		cmd.func = BIND_RENDER_COMMAND_FN(Renderer::draw_skybox);
		cmd_list._commands.emplace_back(cmd);
	}

	//FIX: Find a way if fit the types light in an array {DIRECTIONAL - POINT - SPOT}
	void Renderer::DrawLight(CommandList& cmd_list, Ref<Mesh> _mesh, Light& _light, LightType light_type, int32_t render_graph_index)
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
	void Renderer::AddLight(CommandList& cmd_list, Light& _light, LightType light_type, int32_t render_graph_index)
	{
		Command cmd;
		cmd.params.val[0] = (uint32_t)light_type;
		cmd.params.val[1] = render_graph_index;
		cmd.params.val[2] = static_cast<uint32_t>(_light.params2.z);
		cmd.params.data = (char*)MemoryManager::get_instance()->FrameAlloc(sizeof(Light));
		memcpy(cmd.params.data, &_light, sizeof(Light));

		cmd.func = [&](CommandParams& params) {
			RenderGraphFrameData& graph_data = m_renderGraphsData[params.val[1]];

			bool cast_shadows = (params.val[2] == 1);

			uint32_t light_type_ = params.val[0];
			Light* _light = (Light*)params.data;
			if (light_type_ == (uint32_t)LightType::DIRECTIONAL)
			{
				
				if (cast_shadows && graph_data.num_shadowed_sun_lights < MAX_SHADOW_SUN_LIGHTS)
				{
					auto it = graph_data.sun_lights.begin();
					graph_data.sun_lights.insert(it + graph_data.num_shadowed_sun_lights, *_light);

					graph_data.num_shadowed_sun_lights++;
				}
				else
				{
					graph_data.sun_lights.push_back(*_light);
					graph_data.num_non_shadowed_sun_lights++;
				}
			}
			else if (light_type_ == (uint32_t)LightType::POINT)
			{

				if (cast_shadows && graph_data.num_shadowed_point_lights < MAX_SHADOW_POINT_LIGHTS)
				{
					auto it = graph_data.point_lights.begin();
					graph_data.point_lights.insert(it + graph_data.num_shadowed_point_lights, *_light);

					graph_data.num_shadowed_point_lights++;
				}
				else
				{
					graph_data.point_lights.push_back(*_light);
					graph_data.num_non_shadowed_point_lights++;
				}
			}
			else if (light_type_ == (uint32_t)LightType::SPOT)
			{

				if (cast_shadows && graph_data.num_shadowed_spot_lights < MAX_SHADOW_SPOT_LIGHTS)
				{
					auto it = graph_data.spot_lights.begin();
					graph_data.spot_lights.insert(it + graph_data.num_shadowed_spot_lights, *_light);

					graph_data.num_shadowed_spot_lights++;
				}
				else
				{
					graph_data.spot_lights.push_back(*_light);
					graph_data.num_non_shadowed_spot_lights++;
				}
			}

		};

		cmd_list._commands.push_back(cmd);
	}

	void Renderer::DrawDebugLine(CommandList& cmd_list, glm::vec3 from, glm::vec3 to, uint32_t color, int32_t render_graph_index)
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

	void Renderer::DrawDebugLine(CommandList& cmd_list, glm::vec3 p0, glm::vec3 p1, glm::mat4 transform, uint32_t color, int32_t render_graph_index)
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

	void Renderer::DrawDebugCircle(CommandList& cmd_list, float radius, uint32_t steps, glm::mat4 transform, uint32_t color, int32_t render_graph_index)
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

	void Renderer::DrawDebugSphere(CommandList& cmd_list, float radius, uint32_t steps, glm::mat4 transform, uint32_t color, int32_t render_graph_index)
	{
		DrawDebugCircle(cmd_list, radius, steps, transform, color, render_graph_index);
		glm::mat4 new_transform = glm::rotate(transform, glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		DrawDebugCircle(cmd_list, radius, steps, new_transform, color, render_graph_index);
		new_transform = glm::rotate(transform, glm::radians(-45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		DrawDebugCircle(cmd_list, radius, steps, new_transform, color, render_graph_index);
		new_transform = glm::rotate(transform, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		DrawDebugCircle(cmd_list, radius, steps, new_transform, color, render_graph_index);
	}

	void Renderer::DrawString(CommandList& cmd_list, Ref<Font> font, const std::string& text, glm::vec3 color, glm::mat4 _transform, int32_t render_graph_index)
	{
		if (!font)
		{
			return;
		}		
		DrawString_(font.get(), text, color, _transform, render_graph_index);

	}

	void Renderer::DrawImage(CommandList& cmd_list, Ref<GTexture> texture, glm::mat4 _transform, int32_t render_graph_index)
	{
		DrawQuad_(_transform, texture, render_graph_index);
	}

	void Renderer::DrawImage(CommandList& cmd_list, Ref<GTexture> texture, glm::mat4 _transform, uint32_t color, int32_t render_graph_index)
	{
		DrawQuad_(_transform, texture, color, render_graph_index);
	}

	void Renderer::DrawParticleEffect(CommandList& cmd_list, ParticleEffectInstance* particle_effect, int32_t render_graph_index)
	{
		RenderGraphFrameData& graph_data = m_renderGraphsData[render_graph_index];

		graph_data.particle_effects.push_back(particle_effect);
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

	CommandList Renderer::BeginCommandList(int32_t render_graph_index)
	{
		return {};
	}

	void Renderer::SubmitCommandList(CommandList& list, int32_t render_graph_index)
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