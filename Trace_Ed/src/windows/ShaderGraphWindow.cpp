
#include "ShaderGraphWindow.h"
#include "serialize/GenericSerializer.h"
#include "render/Renderer.h"
#include "../EditorRenderComposer.h"
#include "../panels/GenericGraphEditor.h"
#include "../panels/InspectorPanel.h"
#include "../utils/ImGui_utils.h"
#include "core/input/Input.h"
#include "resource/DefaultAssetsManager.h"


#include "ImGuizmo.h"


namespace trace {



	bool ShaderGraphWindow::OnCreate(TraceEditor* editor, const std::string& name, const std::string& file_path)
	{
		Ref<ShaderGraph> graph = GenericSerializer::Deserialize<ShaderGraph>(file_path);
		if (!graph)
		{
			return false;
		}
		std::string asset_name = graph->GetName();
		Renderer* renderer = Renderer::get_instance();
		EditorRenderComposer* composer = (EditorRenderComposer*)renderer->GetRenderComposer();
		RenderGraphController scene_render_controller = {};
		scene_render_controller.should_render = [this]()->bool { return m_isOpen && can_render; };
		scene_render_controller.build_graph = [composer, this](RenderGraph& graph, RGBlackBoard& black_board, FrameSettings frame_settings, int32_t render_graph_index)
		{
			composer->FullFrameGraph(graph, black_board, frame_settings, m_viewportSize, render_graph_index);
		};

		view_index = composer->BindRenderGraphController(scene_render_controller, asset_name);
		if (view_index < 0)
		{
			TRC_ERROR("{} asset is already opened for editing, Function: {}", graph->GetName(), __FUNCTION__);
			return false;
		}

		m_camera.SetCameraType(CameraType::PERSPECTIVE);
		m_camera.SetPosition(glm::vec3(109.72446f, 95.70557f, -10.92075f));
		m_camera.SetLookDir(glm::vec3(-0.910028f, -0.4126378f, 0.039738327f));
		m_camera.SetUpDir(glm::vec3(0.0f, 1.0f, 0.0f));
		m_camera.SetScreenWidth(800.0f);
		m_camera.SetScreenHeight(600.0f);
		m_camera.SetFov(60.0f);
		m_camera.SetNear(0.1f);
		m_camera.SetFar(15000.0f);

		m_viewportSize = { 800.0f, 600.0f };




		m_shaderGraph = graph;
		m_editor = new GenericGraphEditor;//TODO: Use custom allocator
		m_inspector = new InspectorPanel;//TODO: Use custom allocator
		m_scene = new Scene;//TODO: Use custom allocator
		m_scene->m_path = asset_name;
		m_scene->Create();

		graph_instance.CreateInstance(m_shaderGraph);

		Ref<GPipeline> pipeline = graph_instance.CompileGraph();

		m_material = GenericAssetManager::get_instance()->CreateAssetHandle_<MaterialInstance>(asset_name + MATERIAL_FILE_EXTENSION, pipeline);

		m_material->SetType(m_shaderGraph->GetType());

		Entity model = m_scene->CreateEntity();
		model.AddComponent<ModelComponent>()._model = DefaultAssetsManager::Sphere;
		model.AddComponent<ModelRendererComponent>()._material = m_material;
		visual_id = model.GetID();
		m_scene->EnableEntity(model);
		model.GetComponent<TransformComponent>()._transform.Scale(7.0f);
		
		Entity floor = m_scene->CreateEntity();
		floor.AddComponent<ModelComponent>()._model = DefaultAssetsManager::Cube;
		floor.AddComponent<ModelRendererComponent>()._material = DefaultAssetsManager::default_material;
		m_scene->EnableEntity(floor);

		Transform& transform = floor.GetComponent<TransformComponent>()._transform;
		transform.SetPosition(glm::vec3(0.0f, -15.0f, 0.0f));
		transform.SetScale(glm::vec3(100.0f, 1.0f, 100.0f));
		

		graph_editor_name = "Graph Editor###" + asset_name + std::to_string(0);
		inspector_name = "Material Data###" + asset_name + std::to_string(1);
		viewport_name = "Viewport###" + asset_name + std::to_string(2);

		m_editor->Init();
		m_editor->SetGraph(m_shaderGraph.get(), asset_name);
		if (GenericNode* final_node = m_shaderGraph->GetFragmentShaderNode())
		{
			m_editor->SetCurrentNode(final_node);
		}

		graph_path = file_path;
		m_name = asset_name;
		return true;
	}

	void ShaderGraphWindow::OnDestroy(TraceEditor* editor)
	{
		m_editor->Shutdown();


		graph_instance.DestroyInstance();

		m_scene->Destroy();
		m_shaderGraph.free();
		delete m_editor;
		delete m_scene;

		m_material->m_refCount = 1;
		m_material.free();

		Renderer* renderer = Renderer::get_instance();
		EditorRenderComposer* composer = (EditorRenderComposer*)renderer->GetRenderComposer();
		composer->UnBindRenderGraphController(m_name);

	}

	void ShaderGraphWindow::OnUpdate(float deltaTime)
	{

		if (!m_isOpen)
		{
			return;
		}

		

		m_scene->ResolveHierachyTransforms();
		m_editor->Update(deltaTime);

		if (!can_render)
		{
			return;
		}

		glm::vec3 light_dir = glm::normalize(glm::vec3(0.5f, -0.5f, 0.0f));
		Renderer* renderer = Renderer::get_instance();
		CommandList cmd_list = renderer->BeginCommandList(view_index);
		renderer->BeginScene(cmd_list, &m_camera, view_index);
		Light light_data = {};
		light_data.position = glm::vec4(0.0f);
		light_data.direction = glm::vec4(light_dir, 0.0f);
		light_data.color = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);
		light_data.params1 = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
		light_data.params2 = glm::vec4(0.0f, 2.5f, 0.0f, 0.0f);


		renderer->AddLight(cmd_list, light_data, LightType::DIRECTIONAL, view_index);
		m_scene->OnRender(cmd_list, view_index);
		renderer->EndScene(cmd_list, view_index);

		renderer->SubmitCommandList(cmd_list, view_index);

		m_camera.Update(deltaTime);


	}

	void ShaderGraphWindow::OnRender(float deltaTime)
	{
		ImGui::Begin(graph_editor_name.c_str());
		m_editor->Render(deltaTime);

		ImVec2 min = ImGui::GetWindowPos();
		ImVec2 max = min + ImGui::GetWindowSize();
		is_focused = is_focused || ImGui::IsMouseHoveringRect(min, max);
		ImGui::End();

		ImGui::Begin(inspector_name.c_str());
		m_inspector->DrawEditMaterial(m_material, m_material->GetMaterialData());
		ImGui::End();

	}

	void ShaderGraphWindow::DockChildWindows()
	{
		ImGuiID first_left;
		ImGuiID first_right;
		ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Left, 0.225f, &first_left, &first_right);
		ImGui::DockBuilderDockWindow(viewport_name.c_str(), first_left);
		ImGuiID second_left;
		ImGuiID second_right;
		ImGui::DockBuilderSplitNode(first_right, ImGuiDir_Left, 0.75f, &second_left, &second_right);
		ImGui::DockBuilderDockWindow(graph_editor_name.c_str(), second_left);
		ImGui::DockBuilderDockWindow(inspector_name.c_str(), second_right);
	}

	void ShaderGraphWindow::RenderViewport(std::vector<void*>& texture_handles)
	{
		void* texture = texture_handles[view_index];

		if (!texture)
		{
			return;
		}


		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
		ImGui::Begin(viewport_name.c_str(), 0, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
		ImVec2 view_size = ImGui::GetContentRegionAvail();
		glm::vec2 v_size = { view_size.x, view_size.y };
		if (m_viewportSize != v_size)
		{
			m_viewportSize.x = v_size.x > 0.0f ? v_size.x : m_viewportSize.x;
			m_viewportSize.y = v_size.y > 0.0f ? v_size.y : m_viewportSize.y;
			m_camera.SetScreenWidth(m_viewportSize.x);
			m_camera.SetScreenHeight(m_viewportSize.y);
		}
		ImGui::Image(texture, view_size);
		ImGui::End();

		ImGui::PopStyleVar();
		ImGui::PopStyleColor();
	}

	void ShaderGraphWindow::OnEvent(Event* p_event)
	{
		bool ctrl = InputSystem::get_instance()->GetKey(Keys::KEY_CONTROL) || InputSystem::get_instance()->GetKey(Keys::KEY_LCONTROL) || InputSystem::get_instance()->GetKey(Keys::KEY_RCONTROL);
		bool shift = InputSystem::get_instance()->GetKey(Keys::KEY_SHIFT) || InputSystem::get_instance()->GetKey(Keys::KEY_LSHIFT) || InputSystem::get_instance()->GetKey(Keys::KEY_RSHIFT);
		switch (p_event->GetEventType())
		{
		case EventType::TRC_KEY_PRESSED:
		{
			KeyPressed* press = (KeyPressed*)p_event;
			switch (press->GetKeyCode())
			{
			case Keys::KEY_S:
			{
				if (ctrl && !shift)
				{
					GenericSerializer::Serialize<ShaderGraph>(m_shaderGraph, graph_path);
				}
				else if (ctrl && shift)
				{
					Entity entity = m_scene->GetEntity(visual_id);
					entity.GetComponent<ModelComponent>()._model = DefaultAssetsManager::Sphere;
				}
				break;
			}
			case Keys::KEY_B:
			{
				if (ctrl)
				{
					can_render = false;
					m_scene->DisableEntity(m_scene->GetEntity(visual_id));
					graph_instance.DestroyInstance();
					graph_instance.CreateInstance(m_shaderGraph);
					if (Ref<GPipeline> pipeline = graph_instance.CompileGraph())
					{
						m_material->RecreateMaterial(pipeline);
					}
					m_scene->EnableEntity(m_scene->GetEntity(visual_id));
					can_render = true;
				}
				break;
			}
			case Keys::KEY_C:
			{
				if (ctrl && shift)
				{
					Entity entity = m_scene->GetEntity(visual_id);
					entity.GetComponent<ModelComponent>()._model = DefaultAssetsManager::Cube;					
				}
				break;
			}
			case Keys::KEY_P:
			{
				if (ctrl && shift)
				{
					Entity entity = m_scene->GetEntity(visual_id);
					entity.GetComponent<ModelComponent>()._model = DefaultAssetsManager::Plane;					
				}
				break;
			}
			}
			break;
		}
		}

		m_editor->OnEvent(p_event);
	}



}