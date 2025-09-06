
#include "SequenceWindow.h"


#include "serialize/AnimationsSerializer.h"
#include "render/Renderer.h"
#include "../EditorRenderComposer.h"
#include "../panels/HierachyPanel.h"
#include "../panels/AnimationSequencer.h"
#include "resource/PrefabManager.h"
#include "../utils/ImGui_utils.h"
#include "core/input/Input.h"


#include "ImGuizmo.h"


namespace trace {



	bool SequenceWindow::OnCreate(TraceEditor* editor, const std::string& name, const std::string& file_path)
	{
		Ref<Animation::Sequence> sequence = AnimationsSerializer::DeserializeSequence(file_path);
		if (!sequence)
		{
			return false;
		}
		std::string asset_name = sequence->GetName();
		Renderer* renderer = Renderer::get_instance();
		EditorRenderComposer* composer = (EditorRenderComposer*)renderer->GetRenderComposer();
		RenderGraphController scene_render_controller = {};
		scene_render_controller.should_render = [this]()->bool { return m_isOpen; };
		scene_render_controller.build_graph = [composer, this](RenderGraph& graph, RGBlackBoard& black_board, FrameSettings frame_settings, int32_t render_graph_index)
		{
			composer->FullFrameGraph(graph, black_board, frame_settings, m_viewportSize, render_graph_index);
		};

		view_index = composer->BindRenderGraphController(scene_render_controller, asset_name);
		if (view_index < 0)
		{
			TRC_ERROR("{} asset is already opened for editing, Function: {}", sequence->GetName(), __FUNCTION__);
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




		m_sequence = sequence;
		m_hierachy = new HierachyPanel;//TODO: Use custom allocator
		m_editor = new AnimationSequencer;//TODO: Use custom allocator
		m_scene = new Scene;//TODO: Use custom allocator
		m_scene->m_path = asset_name;
		m_scene->Create();
		hierachy_name = "Hierachy###" + asset_name;
		sequence_editor_name = "Graph Editor###" + asset_name + std::to_string(0);
		viewport_name = "Viewport###" + asset_name + std::to_string(1);
		has_prefab = false;

		m_editor->Init();
		m_editor->SetAnimationSequence(m_sequence, m_scene);

		sequence_path = file_path;
		m_name = asset_name;
		return true;
	}

	void SequenceWindow::OnDestroy(TraceEditor* editor)
	{
		m_editor->Shutdown();

		m_scene->Destroy();
		m_sequence.free();
		delete m_hierachy;
		delete m_editor;
		delete m_scene;

		Renderer* renderer = Renderer::get_instance();
		EditorRenderComposer* composer = (EditorRenderComposer*)renderer->GetRenderComposer();
		composer->UnBindRenderGraphController(m_name);

	}

	void SequenceWindow::OnUpdate(float deltaTime)
	{

		if (!m_isOpen)
		{
			return;
		}

		if (!has_prefab)
		{
			return;
		}

		m_scene->ResolveHierachyTransforms();
		m_editor->Update(deltaTime);

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
		DrawGrid(cmd_list, 5.0f, 50, view_index);

		renderer->SubmitCommandList(cmd_list, view_index);

		m_camera.Update(deltaTime);


	}

	void SequenceWindow::OnRender(float deltaTime)
	{

		if (has_prefab)
		{
			m_hierachy->Render(m_scene, "View", hierachy_name, deltaTime);
		}
		else
		{
			ImGui::Begin(hierachy_name.c_str());
			ImGui::Button("Load View");
			if (ImGui::BeginTooltip())
			{
				ImGui::Text("Drag and Drop Prefab/Scene Asset to be able to visualize sequence");
				ImGui::EndTooltip();
			}
			if (Ref<Prefab> character = ImGuiDragDropResource<Prefab>(PREFAB_FILE_EXTENSION))
			{
				Entity entity = m_scene->InstanciatePrefab(character);
				m_editor->SetAnimationSequence(m_sequence, m_scene);				
				has_prefab = true;
			}
			else if (Ref<Scene> scene = ImGuiDragDropResource<Scene>(SCENE_FILE_EXTENSION))
			{
				Scene::Copy(scene.get(), m_scene);
				m_editor->SetAnimationSequence(m_sequence, m_scene);
				has_prefab = true;
			}
			ImGui::End();

		}

		
		ImGui::Begin(sequence_editor_name.c_str());
		m_editor->Render(deltaTime, m_scene);
		ImVec2 min = ImGui::GetWindowPos();
		ImVec2 max = min + ImGui::GetWindowSize();
		if (ImGui::IsMouseHoveringRect(min, max))
		{
			int x = 0;
		}
		is_focused = is_focused || ImGui::IsMouseHoveringRect(min, max);
		ImGui::End();

	}

	void SequenceWindow::DockChildWindows()
	{
		ImGuiID top;
		ImGuiID bottom;
		ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Up, 0.5f, &top, &bottom);
		ImGui::DockBuilderDockWindow(sequence_editor_name.c_str(), bottom);
		ImGuiID first_left;
		ImGuiID first_right;
		ImGui::DockBuilderSplitNode(top, ImGuiDir_Left, 0.75f, &first_left, &first_right);
		ImGui::DockBuilderDockWindow(viewport_name.c_str(), first_left);
		ImGui::DockBuilderDockWindow(hierachy_name.c_str(), first_right);
	}

	void SequenceWindow::RenderViewport(std::vector<void*>& texture_handles)
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
		if (m_hierachy->GetSelectedEntity())
		{
			DrawGizmo(gizmo_mode, m_scene, m_hierachy->GetSelectedEntity().GetID(), &m_camera);
		}
		ImGui::End();

		ImGui::PopStyleVar();
		ImGui::PopStyleColor();
	}

	void SequenceWindow::OnEvent(Event* p_event)
	{
		bool ctrl = InputSystem::get_instance()->GetKey(Keys::KEY_CONTROL) || InputSystem::get_instance()->GetKey(Keys::KEY_LCONTROL) || InputSystem::get_instance()->GetKey(Keys::KEY_RCONTROL);
		switch (p_event->GetEventType())
		{
		case EventType::TRC_KEY_PRESSED:
		{
			KeyPressed* press = (KeyPressed*)p_event;
			switch (press->GetKeyCode())
			{
			case Keys::KEY_S:
			{
				if (ctrl)
				{
					AnimationsSerializer::SerializeSequence(m_sequence, sequence_path);
				}
				break;
			}
			case KEY_Q:
			{
				gizmo_mode = -1;
				break;
			}
			case KEY_W:
			{
				gizmo_mode = ImGuizmo::OPERATION::TRANSLATE;
				break;
			}
			case KEY_E:
			{
				gizmo_mode = ImGuizmo::OPERATION::ROTATE;
				break;
			}
			case KEY_R:
			{
				gizmo_mode = ImGuizmo::OPERATION::SCALE;
				break;
			}
			}
			break;
		}
		}

		m_editor->OnEvent(p_event);
	}



}