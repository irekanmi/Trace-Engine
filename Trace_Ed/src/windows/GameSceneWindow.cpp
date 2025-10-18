
#include "GameSceneWindow.h"
#include "../TraceEditor.h"
#include "backends/UIutils.h"
#include "resource/GenericAssetManager.h"

#include "scene/Components.h"
#include "core/input/Input.h"
#include "core/memory/StackAllocator.h"
#include "serialize/SceneSerializer.h"
#include "serialize/AnimationsSerializer.h"
#include "core/Utils.h"
#include "scripting/ScriptEngine.h"
#include "scene/Scene.h"
#include "../panels/HierachyPanel.h"
#include "../panels/InspectorPanel.h"
#include "scene/Entity.h"
#include "debug/Debugger.h"
#include "external_utils.h"
#include "../utils/ImGui_utils.h"
#include "core/utils/RingBuffer.h"
#include "serialize/GenericSerializer.h"
#include "particle_effects/particle_renderers/BillboardRender.h"
#include "networking/NetworkManager.h"

#include "imgui.h"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/matrix_decompose.hpp"
#include "imgui_internal.h"
#include "imgui_stdlib.h"
#include "ImGuizmo.h"
#include "portable-file-dialogs.h"
#include "spdlog/fmt/fmt.h"
#include "serialize/yaml_util.h"

int translateKeyTrace_ImGui(trace::Keys key);
int translateButtonTrace_ImGui(trace::Buttons button);

namespace trace {

	void draw_entity_debug_view(Entity entity)
	{
		Entity selected_entity = entity;
		if (!selected_entity)
		{
			return;
		}

		Debugger* debugger = Debugger::get_instance();

		TransformComponent& pose = selected_entity.GetComponent<TransformComponent>();

		if (selected_entity.HasComponent<CharacterControllerComponent>())
		{
			CharacterControllerComponent& controller = selected_entity.GetComponent<CharacterControllerComponent>();
			float scale_x = pose._transform.GetScale().x;
			float scale_y = pose._transform.GetScale().y;
			float scale_z = pose._transform.GetScale().z;
			float height = controller.character.height * pose._transform.GetScale().y;
			float radius = controller.character.radius * ((scale_x + scale_z) / 2.0f);
			glm::mat4 controller_transform = pose._transform.GetLocalMatrix();
			controller_transform = glm::translate(controller_transform, controller.character.offset);
			debugger->DrawDebugCapsule(radius, height, controller_transform, TRC_COL32(255, 145, 255, 255));

			radius += controller.character.contact_offset;
			debugger->DrawDebugCapsule(radius, height, controller_transform, TRC_COL32(0, 255, 255, 25));
		}

		if (selected_entity.HasComponent<BoxColliderComponent>())
		{
			BoxColliderComponent& box = selected_entity.GetComponent<BoxColliderComponent>();
			glm::vec3 extent = box.shape.box.half_extents * pose._transform.GetScale();
			extent += glm::vec3(0.00005f);//NOTE: Added offset to allow the collider visible
			Transform local;
			local.SetPosition(pose._transform.GetPosition() + box.shape.offset);
			local.SetRotation(pose._transform.GetRotation());
			debugger->DrawDebugBox(extent.x, extent.y, extent.z, local.GetLocalMatrix(), TRC_COL32(222, 74, 247, 255));
		}
	}

	bool GameSceneWindow::OnCreate(TraceEditor* editor, const std::string& window_name)
	{
		m_name = window_name;
		
		can_close = false;

		m_hierachyPanel = new HierachyPanel;//TODO: Use custom allocator
		m_inspectorPanel = new InspectorPanel;//TODO: Use custom allocator

		m_editSceneDuplicate = GenericAssetManager::get_instance()->CreateAssetHandle<Scene>("Duplicate Edit Scene");
		m_editorCamera.SetCameraType(CameraType::PERSPECTIVE);
		m_editorCamera.SetPosition(glm::vec3(109.72446f, 95.70557f, -10.92075f));
		m_editorCamera.SetLookDir(glm::vec3(-0.910028f, -0.4126378f, 0.039738327f));
		m_editorCamera.SetUpDir(glm::vec3(0.0f, 1.0f, 0.0f));
		m_editorCamera.SetScreenWidth(800.0f);
		m_editorCamera.SetScreenHeight(600.0f);
		m_editorCamera.SetFov(60.0f);
		m_editorCamera.SetNear(0.1f);
		m_editorCamera.SetFar(15000.0f);

		m_viewportSize = { 800.0f, 600.0f };

		Renderer* renderer = Renderer::get_instance();
		EditorRenderComposer* composer = (EditorRenderComposer*)renderer->GetRenderComposer();
		RenderGraphController scene_render_controller = {};
		scene_render_controller.should_render = [this]()->bool { return m_isOpen; };
		scene_render_controller.build_graph = [composer, this](RenderGraph& graph, RGBlackBoard& black_board, FrameSettings frame_settings, int32_t render_graph_index)
		{
			composer->FullFrameGraph(graph, black_board, frame_settings, m_viewportSize, render_graph_index);
		};

		scene_render_graph_index = composer->BindRenderGraphController(scene_render_controller, "SceneRenderGraph");

		return true;
	}
	void GameSceneWindow::OnDestroy(TraceEditor* editor)
	{
		if (m_currentState == EditorState::ScenePlay)
		{
			OnSceneStop();
			OnGameStop();
		}

		m_currentScene.free();
		m_editScene.free();
		m_editSceneDuplicate.free();


		delete m_hierachyPanel;
		delete m_inspectorPanel;
	}

	void GameSceneWindow::OnUpdate(float deltaTime)
	{
		Renderer* renderer = Renderer::get_instance();
		if (m_stopCurrentScene)
		{
			OnSceneStop();
			OnGameStop();
			m_currentState = EditorState::SceneEdit;

			m_stopCurrentScene = false;

		}

		switch (m_currentState)
		{
		case SceneEdit:
		{
			if (!m_sceneToOpen.empty())
			{
				OpenScene(m_sceneToOpen);
				m_sceneToOpen.clear();
			}

			if (m_viewportFocused || m_viewportHovered)
			{
				m_editorCamera.Update(deltaTime);
			}

			if (m_isOpen)
			{
				CommandList cmd_list = renderer->BeginCommandList();
				renderer->BeginScene(cmd_list, &m_editorCamera, scene_render_graph_index);
				if (m_currentScene)
				{
					m_currentScene->BeginFrame();
					m_currentScene->ResolveHierachyTransforms();
					m_currentScene->OnRender(cmd_list, scene_render_graph_index);
					m_currentScene->EndFrame();
				}
				DrawGrid(cmd_list, 15.0f, 75, scene_render_graph_index);
				renderer->EndScene(cmd_list, scene_render_graph_index);
				renderer->SubmitCommandList(cmd_list);

				if (m_currentScene)
				{
					HandleEntityDebugDraw();
				}
			}

			break;
		}
		case ScenePlay:
		{
			if (m_currentScene)
			{
				if (m_nextScene)
				{
					m_hierachyPanel->SetSelectedEntity(Entity());
					stop_current_scene();
					m_currentScene.free();

					m_currentScene = m_nextScene;
					start_current_scene();
					m_nextScene.free();
				}

				m_currentScene->BeginFrame();

				m_currentScene->OnAnimationUpdate(deltaTime);
				m_currentScene->OnScriptUpdate(deltaTime);
				m_currentScene->OnPhysicsUpdate(deltaTime);
				m_currentScene->OnUpdate(deltaTime);
				if (m_isOpen)
				{
					m_currentScene->OnRender(scene_render_graph_index);
				}



				m_currentScene->EndFrame();
			}
			break;
		}
		}
	}
	void GameSceneWindow::OnRender(float deltaTime)
	{
		Scene* scene = m_currentScene ? m_currentScene.get() : nullptr;
		std::string tree_name = m_currentScene ? m_currentSceneName : "None(Scene)";
		m_hierachyPanel->Render(scene, tree_name, "Scene Hierachy", deltaTime);

		//Inspector
		ImGui::Begin("Inspector");
		if (m_hierachyPanel->GetSelectedEntity())
		{
			m_inspectorPanel->DrawEntityComponent(m_hierachyPanel->GetSelectedEntity());
		}
		ImGui::End();

		RenderSceneToolBar();

	}
	void GameSceneWindow::OnWindowOpen()
	{
	}
	void GameSceneWindow::OnWindowLeave()
	{
	}
	void GameSceneWindow::OnWindowClose()
	{
	}

	void GameSceneWindow::DockChildWindows()
	{
		ImGuiID first_left;
		ImGuiID first_right;
		ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Left, 0.1f, &first_left, &first_right);
		ImGui::DockBuilderDockWindow("Scene Hierachy", first_left);
		ImGuiID second_left;
		ImGuiID second_right;
		ImGui::DockBuilderSplitNode(first_right, ImGuiDir_Left, 0.6f, &second_left, &second_right);
		ImGui::DockBuilderDockWindow("Inspector", second_right);
		ImGuiID third_top;
		ImGuiID third_bottom;
		ImGui::DockBuilderSplitNode(second_left, ImGuiDir_Up, 0.05f, &third_top, &third_bottom);
		ImGui::DockBuilderDockWindow("##Scene Toolbar", third_top);
		ImGui::DockBuilderDockWindow("Scene Viewport", third_bottom);
	}

	void GameSceneWindow::RenderViewport(std::vector<void*>& texture_handles)
	{
		void* texture = texture_handles[scene_render_graph_index];
		if (m_fullScreen)
		{
			ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->WorkPos);
			ImGui::SetNextWindowSize(viewport->WorkSize);
			ImGui::SetNextWindowViewport(viewport->ID);

			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::Begin("FullScreen", 0, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);
			ImVec2 view_size = ImGui::GetContentRegionAvail();
			glm::vec2 v_size = { view_size.x, view_size.y };
			if (m_viewportSize != v_size)
			{
				m_viewportSize = v_size;
				m_editorCamera.SetScreenWidth(m_viewportSize.x);
				m_editorCamera.SetScreenHeight(m_viewportSize.y);
				if (m_currentScene) m_currentScene->OnViewportChange(m_viewportSize.x, m_viewportSize.y);
			}
			m_viewportFocused = ImGui::IsWindowFocused();
			m_viewportHovered = ImGui::IsWindowHovered();
			ImGui::Image(texture, view_size);
			if (m_currentState == SceneEdit)
			{

			}

			ImGui::End();

			ImGui::PopStyleVar(2);
		}
		else
		{
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
			ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
			ImGui::Begin("Scene Viewport", 0, ImGuiWindowFlags_NoCollapse);
			ImVec2 view_size = ImGui::GetContentRegionAvail();
			glm::vec2 v_size = { view_size.x, view_size.y };
			if (m_viewportSize != v_size)
			{
				m_viewportSize.x = v_size.x > 0.0f ? v_size.x : m_viewportSize.x;
				m_viewportSize.y = v_size.y > 0.0f ? v_size.y : m_viewportSize.y;
				m_editorCamera.SetScreenWidth(m_viewportSize.x);
				m_editorCamera.SetScreenHeight(m_viewportSize.y);
				if (m_currentScene) m_currentScene->OnViewportChange(m_viewportSize.x, m_viewportSize.y);
			}
			m_viewportFocused = ImGui::IsWindowFocused();
			m_viewportHovered = ImGui::IsWindowHovered();
			ImGui::Image(texture, view_size);
			if (m_currentState == SceneEdit)
			{
				if (ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(".trscn"))
					{
						static char buf[1024] = { 0 };
						memcpy_s(buf, 1024, payload->Data, payload->DataSize);
						std::string path = buf;
						CloseCurrentScene();
						LoadScene(path);
						m_currentScenePath = path;
					}
					ImGui::EndDragDropTarget();
				}
			}
			if (m_hierachyPanel->GetSelectedEntity())
			{
				DrawGizmo(gizmo_mode, m_currentScene.get(), m_hierachyPanel->GetSelectedEntity().GetID(), &m_editorCamera);
			}
			ImGui::End();

			ImGui::PopStyleVar();
			ImGui::PopStyleColor();
		}
	}

	void GameSceneWindow::RenderSceneToolBar()
	{
		ImGui::Begin("##Scene Toolbar", false, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoTitleBar);

		bool playing = (m_currentState == EditorState::ScenePlay);
		bool stimulating = (m_currentState == EditorState::SceneStimulate);

		if (ImGui::Button(playing ? "Stop" : "Play"))
		{
			if (m_currentScene)
			{
				if (playing)
				{
					m_stopCurrentScene = true;
				}
				else
				{
					OnGameStart();
					OnScenePlay();
					m_currentState = EditorState::ScenePlay;
					m_currentSceneName = m_editScene->GetName();
				}
			}
		}
		ImGui::SameLine();
		/*if (ImGui::Button(!stimulating ? "Stimulate" : "Stop Stimulation"))
		{
			if (m_currentScene)
			{
				if (stimulating)
				{
					OnSceneStop();
					m_currentState = EditorState::SceneEdit;
				}
				else
				{
					OnSceneStimulate();
					m_currentState = EditorState::SceneStimulate;
				}
			}
		}*/

		ImGui::End();
	}

	void GameSceneWindow::HandleEntityDebugDraw()
	{
		
		draw_entity_debug_view(m_hierachyPanel->GetSelectedEntity());
	}

	void GameSceneWindow::CloseCurrentScene()
	{
		m_currentScene.free();
		m_editScene.free();
		m_hierachyPanel->SetSelectedEntity(Entity());
		m_currentScenePath = "";
	}
	void GameSceneWindow::LoadScene(const std::string& file_path)
	{
		if (m_currentState == EditorState::ScenePlay)
		{
			return;
		}

		m_sceneToOpen = file_path;
		
	}
	void GameSceneWindow::NewScene()
	{
		/*if (m_currentScene) CloseCurrentScene();
		auto result = pfd::save_file("New Scene", m_currentProject_path.string(), { "Trace Scene", "*.trscn" }, pfd::opt::force_path).result();
		if (!result.empty())
		{
			std::filesystem::path p = result;
			m_currentScene = SceneManager::get_instance()->CreateScene(p.filename().string());
			m_editScene = m_currentScene;
			m_currentScenePath = result;
		}*/
	}
	void GameSceneWindow::SaveScene()
	{
		if (m_currentState == EditorState::ScenePlay)
		{
			return;
		}

		if (m_currentScenePath.empty())
		{
			return;
		}
		if (m_currentScene)
		{
			SceneSerializer::Serialize(m_currentScene, m_currentScenePath);
		}
	}
	bool GameSceneWindow::CreateScene(const std::string& file_path)
	{
		std::filesystem::path p = file_path;
		Ref<Scene> res = GenericAssetManager::get_instance()->CreateAssetHandle<Scene>(p.filename().string());
		SceneSerializer::Serialize(res, file_path);
		return true;
	}
	std::string GameSceneWindow::SaveSceneAs()
	{
		/*if (m_currentScene)
		{
			auto result = pfd::save_file("Save Scene As", m_currentProject_path.string(), { "Trace Scene", "*.trscn" }, pfd::opt::force_path).result();
			if (!result.empty())
			{
				SceneSerializer::Serialize(m_currentScene, result);
			}
			return result;
		}*/
		return std::string();
	}

	void GameSceneWindow::OnScenePlay()
	{
		if ((m_currentState == EditorState::ScenePlay))
		{
			return;
		}

		Scene::Copy(m_editScene, m_editSceneDuplicate);
		m_currentScene = m_editSceneDuplicate;
		/*if (m_hierachyPanel->GetSelectedEntity())
		{
			m_hierachyPanel->SetSelectedEntity(m_currentScene->GetEntity(m_hierachyPanel->GetSelectedEntity().GetID()));
		}*/
		m_hierachyPanel->SetSelectedEntity(Entity());
		start_current_scene();

	}
	void GameSceneWindow::OnSceneStimulate()
	{
		if ((m_currentState == EditorState::SceneStimulate))
		{
			return;
		}

		Scene::Copy(m_editScene, m_editSceneDuplicate);
		m_currentScene = m_editSceneDuplicate;
		m_currentScene->OnStart();
		if (m_hierachyPanel->GetSelectedEntity())
		{
			m_hierachyPanel->GetSelectedEntity() = m_currentScene->GetEntity(m_hierachyPanel->GetSelectedEntity().GetID());
		}
	}
	void GameSceneWindow::OnSceneStop()
	{
		if ((m_currentState == EditorState::SceneEdit)) return;

		stop_current_scene();
		m_hierachyPanel->SetSelectedEntity(Entity());
		/*if (m_hierachyPanel->GetSelectedEntity() && m_currentScene == m_editScene)
		{
			m_hierachyPanel->SetSelectedEntity(m_currentScene->GetEntity(m_hierachyPanel->GetSelectedEntity().GetID()));
		}*/
		m_currentScene = m_editScene;
		m_nextScene.free();
	}

	void GameSceneWindow::OnGameStart()
	{
		Network::NetworkManager::get_instance()->OnGameStart();
	}

	void GameSceneWindow::OnGameStop()
	{
		Network::NetworkManager::get_instance()->OnGameStop();
		m_currentSceneName = m_editScene->GetName();
	}

	bool GameSceneWindow::SetNextScene(Ref<Scene> scene)
	{
		if (!scene)
		{
			return false;
		}

		m_nextScene = scene;
		m_currentScenePath = GetPathFromUUID(m_nextScene->GetUUID()).string();
		m_currentSceneName = m_nextScene->GetName();

		return true;
	}

	void GameSceneWindow::start_current_scene()
	{
		m_currentScene->OnStart();
		m_currentScene->OnPhysicsStart();
		m_currentScene->OnNetworkStart();
		m_currentScene->OnScriptStart();


		if (m_currentScene)
		{
			m_currentScene->OnViewportChange(m_viewportSize.x, m_viewportSize.y);
		}
		m_currentSceneName = m_currentScene->GetName();
	}

	void GameSceneWindow::stop_current_scene()
	{
		m_currentScene->OnScriptStop();
		m_currentScene->OnNetworkStop();
		m_currentScene->OnPhysicsStop();
		m_currentScene->OnStop();
	}

	void GameSceneWindow::OpenScene(std::string& path)
	{
		if (m_currentState == EditorState::ScenePlay)
		{
			return;
		}
		

		Ref<Scene> scene = SceneSerializer::Deserialize(path);

		if (!scene)
		{
			TRC_ERROR("Unable to load Scene, Path: {}, Function: {}", path, __FUNCTION__);
			return;
		}

		m_currentScene = scene;
		m_editScene = m_currentScene;
		m_currentScenePath = path;
		m_currentSceneName = m_currentScene->GetName();
		m_hierachyPanel->SetSelectedEntity(Entity());

		//m_currentScene = m_currentScene;
	}

	void GameSceneWindow::OnEvent(Event* p_event)
	{


		switch (p_event->GetEventType())
		{
		case TRC_KEY_PRESSED:
		{
			trace::KeyPressed* press = reinterpret_cast<trace::KeyPressed*>(p_event);
			HandleKeyPressed(press);


			break;
		}
		case TRC_KEY_RELEASED:
		{
			trace::KeyReleased* release = reinterpret_cast<trace::KeyReleased*>(p_event);
			HandleKeyRelesed(release);

			break;
		}

		}

	}

	void GameSceneWindow::HandleKeyPressed(KeyPressed* p_event)
	{
		InputSystem* input = InputSystem::get_instance();
		bool control = input->GetKey(KEY_LCONTROL) || input->GetKey(KEY_RCONTROL) || input->GetKey(Keys::KEY_CONTROL);
		bool shift = input->GetKey(KEY_LSHIFT) || input->GetKey(KEY_RSHIFT) || input->GetKey(Keys::KEY_SHIFT);
		switch (p_event->GetKeyCode())
		{
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
		case KEY_T:
		{

			break;
		}
		case KEY_S:
		{
			if (m_currentState != SceneEdit) break;

			if (control && shift)
			{
				//SaveSceneAs();
			}
			else if (control)
			{
				SaveScene();
			}

			break;
		}
		case KEY_O:
		{
			if (m_currentState != SceneEdit) break;

			if (control)
			{
				//OpenScene();
			}

			break;
		}
		case KEY_N:
		{
			/*if (m_currentState != SceneEdit)
			{
				break;
			}

			if (control)
			{

			}*/

			break;
		}
		case KEY_D:
		{
			if (m_currentState != SceneEdit)
			{
				break;
			}

			if (control && m_hierachyPanel->GetSelectedEntity())
			{
				m_currentScene->DuplicateEntity(m_hierachyPanel->GetSelectedEntity());
			}

			break;
		}

		case KEY_G:
		{

			break;
		}

		case KEY_H:
		{

			break;
		}

		case KEY_Z:
		{
			m_editorCamera.SetCameraType(CameraType::PERSPECTIVE);
			m_editorCamera.SetFov(60.0f);
			m_editorCamera.SetNear(0.1f);
			m_editorCamera.SetFar(15000.0f);

			break;
		}

		case KEY_X:
		{
			m_editorCamera.SetCameraType(CameraType::ORTHOGRAPHIC);
			m_editorCamera.SetFov(60.0f);
			m_editorCamera.SetNear(-100.0f);
			m_editorCamera.SetFar(200.0f);
			m_editorCamera.SetOrthographicSize(65.0f);
			break;
		}
		case KEY_L:
		{
			//TEMP
			return;
			/*if (!m_currentScene || m_currentState != EditorState::SceneEdit)
			{
				return;
			}

			Entity entity = m_currentScene->GetMainCamera();
			if (!entity)
			{
				return;
			}

			Camera& camera = entity.GetComponent<CameraComponent>()._camera;

			camera.SetFov(m_editorCamera.GetFov());
			camera.SetNear(m_editorCamera.GetNear());
			camera.SetFar(m_editorCamera.GetFar());
			camera.SetOrthographicSize(m_editorCamera.GetOrthographicSize());

			m_currentScene->SetEntityWorldPosition(entity, m_editorCamera.GetPosition());

			glm::vec3 look_dir = m_editorCamera.GetLookDir();
			glm::vec3 up_dir = m_editorCamera.GetUpDir();
			glm::quat look_quat = glm::quatLookAt(-look_dir, glm::vec3(0.0f, 1.0f, 0.0f));

			m_currentScene->SetEntityWorldRotation(entity, look_quat);*/

			break;
		}

		}

	}
	void GameSceneWindow::HandleKeyRelesed(KeyReleased* p_event)
	{

		InputSystem* input = InputSystem::get_instance();
		bool control = input->GetKey(KEY_LCONTROL) || input->GetKey(KEY_RCONTROL);
		bool shift = input->GetKey(KEY_LSHIFT) || input->GetKey(KEY_RSHIFT);

		switch (p_event->GetKeyCode())
		{
		case KEY_K:
		{
			if (control && shift)
			{
				m_fullScreen = true;

			}
			break;
		}
		case KEY_ESCAPE:
		{
			if (m_fullScreen)
			{
				m_fullScreen = false;

			}
			break;
		}
		case KEY_G:
		{

			break;
		}

		case KEY_H:
		{
			break;
		}
		case KEY_SPACE:
		{
			/*if (m_currentState != SceneEdit)
			{
				break;
			}*/


			break;
		}
		case KEY_4:
		{
			break;
		}
		case KEY_5:
		{
			break;
		}
		case KEY_6:
		{
			break;
		}
		case KEY_7:
		{
			break;
		}
		case KEY_8:
		{
			break;
		}
		}

	}
}