#include "TraceEditor.h"
#include "imgui.h"
#include "backends/UIutils.h"
#include "resource/GenericAssetManager.h"

#include "scene/Components.h"
#include "core/input/Input.h"
#include "core/memory/StackAllocator.h"
#include "serialize/SceneSerializer.h"
#include "serialize/PipelineSerializer.h"
#include "serialize/ProjectSerializer.h"
#include "serialize/AnimationsSerializer.h"
#include "serialize/MaterialSerializer.h"
#include "core/Utils.h"
#include "scripting/ScriptEngine.h"
#include "scene/Scene.h"
#include "panels/HierachyPanel.h"
#include "panels/InspectorPanel.h"
#include "panels/ContentBrowser.h"
#include "panels/AnimationPanel.h"
#include "panels/AnimationGraphEditor.h"
#include "panels/AnimationSequencer.h"
#include "EditorRenderComposer.h"
#include "builder/ProjectBuilder.h"
#include "scene/Entity.h"
#include "import/Importer.h"
#include "animation/AnimationNode.h"
#include "animation/AnimationSequenceTrack.h"
#include "animation/SequenceTrackChannel.h"
#include "debug/Debugger.h"
#include "external_utils.h"
#include "animation/HumanoidRig.h"
#include "utils/ImGui_utils.h"
#include "core/maths/Dampers.h"
#include "core/utils/RingBuffer.h"
#include "serialize/GenericSerializer.h"
#include "orange_duck/spring.h"
#include "core/maths/Conversion.h"
#include "networking/NetworkManager.h"
#include "particle_effects/particle_renderers/BillboardRender.h"
#include "windows/GameSceneWindow.h"
#include "windows/PrefabWindow.h"
#include "windows/AnimationGraphWindow.h"
#include "windows/SequenceWindow.h"
#include "windows/AnimationWindow.h"
#include "resource/PrefabManager.h"
 

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

// Style and Themes
void light_yellow();
void dark_green();
void dark_red();
void embraceTheDarkness();
void dark_purple();
void custom_solid();



namespace trace {

	//TEMP ---------------------------------
	static bool show_humanoid_window = false;
	static bool show_feature_db_window = false;
	static bool show_mmt_info_window = false;
	static Ref<Animation::Skeleton> skeleton;
	static Ref<Animation::HumanoidRig> rig;
	static Ref<MotionMatching::FeatureDatabase> feature_db;
	static Ref<MotionMatching::MotionMatchingInfo> mmt_info;

	// Temp ......................
	void GenerateParticleEffect(Ref<Project> project)
	{
		std::string effect_filename = "particle_effect_t0" + std::string(PARTICLE_EFFECT_FILE_EXTENSION);
		Ref<ParticleEffect> particle_effect = GenericAssetManager::get_instance()->CreateAssetHandle<ParticleEffect>(effect_filename);
		std::string generator_filename = "particle_generator_t0" + std::string(PARTICLE_GENERATOR_FILE_EXTENSION);
		Ref<ParticleGenerator> particle_generator = GenericAssetManager::get_instance()->CreateAssetHandle<ParticleGenerator>(generator_filename);

		particle_effect->SetLifeTime(300.0f);
		particle_effect->GetGenerators().push_back(particle_generator);

		particle_generator->SetCapacity(3000);
		RateSpawner* spawner = new RateSpawner;
		spawner->SetSpawnRate(450.0f);
		PointVolume* emission_volume = new PointVolume(glm::vec3(0.0f));
		spawner->SetEmissionVolume(emission_volume);
		particle_generator->SetSpawner(spawner);

		VelocityInitializer* velocity_init = new VelocityInitializer(glm::vec3(7.5f, 15.0f, 5.0f), glm::vec3(-10.5f, 17.5f, -2.0f));
		particle_generator->GetInitializers().push_back(velocity_init);
		
		LifetimeInitializer* lifetime_init = new LifetimeInitializer;
		lifetime_init->SetMin(3.0f);
		lifetime_init->SetMax(5.0f);
		particle_generator->GetInitializers().push_back(lifetime_init);

		GravityUpdate* gravity_update = new GravityUpdate;
		particle_generator->GetUpdates().push_back(gravity_update);
		
		VelocityUpdate* velocity_update = new VelocityUpdate;
		particle_generator->GetUpdates().push_back(velocity_update);

		BillBoardRender* renderer = new BillBoardRender;
		//renderer->SetVelocityAligned(true);
		std::string texture_path = GetPathFromUUID(STR_ID("Bullet_1.png")).string();
		Ref<GTexture> texture = GenericAssetManager::get_instance()->CreateAssetHandle_<GTexture>(texture_path, texture_path);
		renderer->SetTexture(texture);
		particle_generator->GetRenderers().push_back(renderer);
	

		std::string file_path = project->GetAssetsDirectory() + "/Particle_Effects/";
		GenericSerializer::Serialize<ParticleEffect>(particle_effect, file_path + effect_filename);
		GenericSerializer::Serialize<ParticleGenerator>(particle_generator, file_path + generator_filename);
	}
	// .............................

	void SerializationTest();
	void DeserializationTest();

	static int32_t swapchain_graph = -1;

	bool TraceEditor::Init()
	{
		
		embraceTheDarkness();
		//m_hierachyPanel = new HierachyPanel;//TODO: Use custom allocator
		//m_inspectorPanel = new InspectorPanel;//TODO: Use custom allocator
		m_contentBrowser = new ContentBrowser;//TODO: Use custom allocator
		m_importer = new Importer;//TODO: Use custom allocator

		m_contentBrowser->Init();

		// Register Events
		{
			trace::EventsSystem::get_instance()->AddEventListener(trace::EventType::TRC_WND_RESIZE, BIND_EVENT_FN(TraceEditor::OnEvent));
			trace::EventsSystem::get_instance()->AddEventListener(trace::EventType::TRC_KEY_RELEASED, BIND_EVENT_FN(TraceEditor::OnEvent));
			trace::EventsSystem::get_instance()->AddEventListener(trace::EventType::TRC_KEY_PRESSED, BIND_EVENT_FN(TraceEditor::OnEvent));
			trace::EventsSystem::get_instance()->AddEventListener(trace::EventType::TRC_WND_CLOSE, BIND_EVENT_FN(TraceEditor::OnEvent));
			trace::EventsSystem::get_instance()->AddEventListener(trace::EventType::TRC_BUTTON_PRESSED, BIND_EVENT_FN(TraceEditor::OnEvent));
			trace::EventsSystem::get_instance()->AddEventListener(trace::EventType::TRC_BUTTON_RELEASED, BIND_EVENT_FN(TraceEditor::OnEvent));
			trace::EventsSystem::get_instance()->AddEventListener(trace::EventType::TRC_MOUSE_MOVE, BIND_EVENT_FN(TraceEditor::OnEvent));
			trace::EventsSystem::get_instance()->AddEventListener(trace::EventType::TRC_MOUSE_DB_CLICK, BIND_EVENT_FN(TraceEditor::OnEvent));
			trace::EventsSystem::get_instance()->AddEventListener(trace::EventType::TRC_KEY_TYPED, BIND_EVENT_FN(TraceEditor::OnEvent));
			trace::EventsSystem::get_instance()->AddEventListener(trace::EventType::TRC_MOUSE_WHEEL, BIND_EVENT_FN(TraceEditor::OnEvent));
		};

		/*m_editSceneDuplicate = GenericAssetManager::get_instance()->CreateAssetHandle<Scene>("Duplicate Edit Scene");
		m_editorCamera.SetCameraType(CameraType::PERSPECTIVE);
		m_editorCamera.SetPosition(glm::vec3(109.72446f, 95.70557f, -10.92075f));
		m_editorCamera.SetLookDir(glm::vec3(-0.910028f, -0.4126378f, 0.039738327f));
		m_editorCamera.SetUpDir(glm::vec3(0.0f, 1.0f, 0.0f));
		m_editorCamera.SetScreenWidth(800.0f);
		m_editorCamera.SetScreenHeight(600.0f);
		m_editorCamera.SetFov(60.0f);
		m_editorCamera.SetNear(0.1f);
		m_editorCamera.SetFar(15000.0f);

		m_viewportSize = { 800.0f, 600.0f };*/

		// Temp
		m_allAssets.models.emplace("Cube");
		m_allAssets.models.emplace("Sphere");
		m_allAssets.models.emplace("Plane");

		m_allAssets.textures.emplace("albedo_map");
		m_allAssets.textures.emplace("specular_map");
		m_allAssets.textures.emplace("normal_map");
		m_allAssets.textures.emplace("black_texture");
		m_allAssets.textures.emplace("transparent_texture");

		m_allAssets.materials.emplace("default");

		m_allAssets.pipelines.emplace("gbuffer_pipeline");


		//SerializationTest();
		//DeserializationTest();
		Renderer* renderer = Renderer::get_instance();
		EditorRenderComposer* composer = (EditorRenderComposer*)renderer->GetRenderComposer();
		RenderGraphController swapchain_controller = {};
		swapchain_controller.should_render = []()->bool { return true; };
		swapchain_controller.build_graph = [composer](RenderGraph& graph, RGBlackBoard& black_board, FrameSettings frame_settings, int32_t render_graph_index)
		{
			composer->FinalSwapchainGraph(graph, black_board, frame_settings, render_graph_index);
		};

		swapchain_graph = composer->BindRenderGraphController(swapchain_controller, "FinalSwapchainGraph");

		scene_window = new GameSceneWindow;
		scene_window->OnCreate(this, "Game Level Editor");
		m_windows.push_back(scene_window);

		return true;
	}

	void TraceEditor::Shutdown()
	{
		for (auto& i : m_windows)
		{
			i->OnDestroy(this);
			delete i;
		}

		m_contentBrowser->Shutdown();
		


		delete m_importer;
		delete m_contentBrowser;
	}

	void TraceEditor::Update(float deltaTime)
	{
		Renderer* renderer = Renderer::get_instance();
		
		PrefabManager::get_instance()->GetScene()->ResolveHierachyTransforms();


		for(auto& i : m_windows)
		{
			i->OnUpdate(deltaTime);
		}

		//implement window removal
		int32_t prev_last_index = m_windows.size() - 1;
		int32_t last_index = m_windows.size() - 1;
		int32_t index = 0;
		while (index <= last_index)
		{
			EditorWindow* data = m_windows[index];

			if (data->Closed())
			{
				data->OnDestroy(this);
				delete data;
				m_windows[index] = m_windows[last_index];
				last_index--;
			}

			index++;
		}

		if (last_index < prev_last_index)
		{
			++last_index;
			auto it = m_windows.begin() + last_index;
			m_windows.erase(it, m_windows.end());
		}

		m_deltaTime = deltaTime;

	}

	//TEMP =============
	static float speed = 10.0f;
	static RingBuffer<glm::mat4> debug_buffer(5);
	

	void TraceEditor::Render(float deltaTime)
	{
		static bool show_demo = false;
		static float seek_time = 1.0f;
		static float elapsed = 0.0f;
		static float FPS = 0.0f;
		

		// DockSpace
		{
			ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

			const ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->WorkPos);
			ImGui::SetNextWindowSize(viewport->WorkSize);
			ImGui::SetNextWindowViewport(viewport->ID);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
			window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
			ImGui::Begin("Main_DockSpace", false, window_flags);
			ImGui::PopStyleVar();

			ImGui::PopStyleVar(2);

			ImGuiStyle& style = ImGui::GetStyle();
			float min_window_size = style.WindowMinSize.x;
			style.WindowMinSize.x = 165.0f;
			main_dockspace = ImGui::GetID("Main_DockSpace_ID");
			ImGui::DockSpace(main_dockspace);
			style.WindowMinSize.x = min_window_size;

			static bool first_run = true;

			if (first_run)
			{
				ImGui::DockBuilderRemoveNode(main_dockspace);
				ImGui::DockBuilderAddNode(main_dockspace, ImGuiDockNodeFlags_DockSpace);
				ImGui::DockBuilderSetNodeSize(main_dockspace, viewport->WorkSize);

				ImGui::DockBuilderSplitNode(main_dockspace, ImGuiDir_Up, 0.75f, &main_dockspace_Top, &main_dockspace_Bottom);
				
				ImGui::DockBuilderDockWindow("Sample Tab", main_dockspace_Bottom);
				ImGui::DockBuilderDockWindow("Content Browser", main_dockspace_Bottom);
				ImGui::DockBuilderDockWindow("Game Level Editor", main_dockspace_Top);

				ImGui::DockBuilderFinish(main_dockspace);

				first_run = false;
			}

			// Menu Bar
			if (ImGui::BeginMenuBar())
			{
				if (ImGui::BeginMenu("File"))
				{
					
					if (ImGui::MenuItem("Save Scene", "Crtl+S"))
					{
						scene_window->SaveScene();
					}
					ImGui::Separator();
					//if (ImGui::MenuItem("Close Scene"))
					ImGui::Separator();
					if (ImGui::MenuItem("New project"))
					{
						p_createProject = true;
					}
					if (ImGui::MenuItem("Open project"))
					{
						OpenProject();
					}

					ImGui::EndMenu();
				}
				if (ImGui::BeginMenu("Project"))
				{
					if (ImGui::MenuItem("Reload Assembly"))
					{
						ReloadProjectAssembly();
					}
					if (ImGui::MenuItem("Settings")) p_projectSettings = true;
					if (ImGui::MenuItem("Build Project")) BuildProject();
					ImGui::EndMenu();
				}
				if (ImGui::BeginMenu("Settings"))
				{
					if (ImGui::BeginMenu("Select theme"))
					{
						if (ImGui::MenuItem("Dark Theme")) { ImGui::StyleColorsDark(); }
						if (ImGui::MenuItem("Light Theme")) { ImGui::StyleColorsLight(); }
						if (ImGui::MenuItem("Classic Theme")) { ImGui::StyleColorsClassic(); }
						ImGui::Separator();
						if (ImGui::MenuItem("Light Yellow Theme")) { light_yellow(); }
						if (ImGui::MenuItem("Dark Green Theme")) { dark_green(); }
						if (ImGui::MenuItem("Dark Red Theme")) { dark_red(); }
						if (ImGui::MenuItem("Embrace The Darkness")) { embraceTheDarkness(); }
						if (ImGui::MenuItem("Dark Purple Theme")) { dark_purple(); }
						if (ImGui::MenuItem("Custom Solid Theme")) { custom_solid(); }

						ImGui::EndMenu();
					}
					ImGui::EndMenu();
				}

				ImGui::EndMenuBar();
			}

			ImGui::End();
		};
    
		for (auto& i : m_windows)
		{
			i->Render(deltaTime);
		}

		if (show_demo) ImGui::ShowDemoWindow(&show_demo);
		bool sample_tab = true;
		if (ImGui::Begin("Sample Tab", &sample_tab))
		{
			ImVec4 text_color = { .0f, .29f, .40f, 1.0f };
			ImGui::TextColored(text_color,"Delta Time : %.4f", deltaTime);
			ImGui::TextColored(text_color,"FPS : %.4f", FPS);
			if (elapsed >= seek_time)
			{
				FPS = 1.0f / deltaTime;
				elapsed = 0.0f;
			}
			ImGui::Text("Trace Engine");
			ImGui::SameLine();
			if (ImGui::Button("Show Demo Window"))
			{
				show_demo = true;
			}
			ImGui::TextColored({ .0f, .59f, .40f, 1.0f }, "Coker Ayanfe");
			ImGui::DragFloat("Seek Time", &seek_time, 0.05f, 0.0f, 5.0f, "%.4f");

			Network::NetworkManager* net_manager = Network::NetworkManager::get_instance();
			float rtt = net_manager->GetServerAverageRTT();

			ImGui::DragFloat("RTT", &rtt, 0.05f);

		}
		ImGui::End();
		elapsed += deltaTime;
		
		// -------------------/////////////////-----------------------------------


		// Content Browser
		m_contentBrowser->Render(deltaTime);

		//Create Project
		if (p_createProject)
		{
			ImGui::Begin("Create Project", &p_createProject);
			static std::string project_dir;
			ImGui::InputText("Project Directory", &project_dir);
			ImGui::SameLine();
			if (ImGui::Button("..."))
			{
				std::string result = pfd::select_folder("Select Directory").result();
				if (!result.empty())
				{
					project_dir = result;
				}
			}
			static std::string project_name;
			ImGui::InputText("Project Name", &project_name);

			if (ImGui::Button("Create Project"))
			{
				CreateProject(project_dir, project_name);

				p_createProject = false;
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel"))
			{
				project_dir.clear();
				project_name.clear();
				p_createProject = false;
			}

			ImGui::End();
		}

		//Project settings window
		ProjectSettings();

		RenderUtilsWindows(deltaTime);

	}

	void TraceEditor::RenderViewport(std::vector<void*>& texture_handles)
	{

		for (auto& i : m_windows)
		{
			i->RenderViewport(texture_handles);
		}
		
	}

	RenderComposer* TraceEditor::GetRenderComposer()
	{
		if (!m_renderComposer)
		{
			m_renderComposer = new EditorRenderComposer; // TODO: Trace Builtin memory system
		}
		return m_renderComposer;
	}
	void TraceEditor::OnEvent(Event* p_event)
	{

		ImGuiIO& io = ImGui::GetIO(); (void)io;

		switch (p_event->GetEventType())
		{
		case TRC_KEY_PRESSED:
		{
			trace::KeyPressed* press = reinterpret_cast<trace::KeyPressed*>(p_event);
			io.AddKeyEvent((ImGuiKey)translateKeyTrace_ImGui(press->GetKeyCode()), true);
			HandleKeyPressed(press);
			

			break;
		}
		case TRC_WND_CLOSE:
		{
			CloseProject();
			break;
		}
		case TRC_KEY_RELEASED:
		{
			trace::KeyReleased* release = reinterpret_cast<trace::KeyReleased*>(p_event);
			io.AddKeyEvent((ImGuiKey)translateKeyTrace_ImGui(release->GetKeyCode()), false);
			HandleKeyRelesed(release);

			break;
		}

		case TRC_WND_RESIZE:
		{
			trace::WindowResize* wnd = reinterpret_cast<trace::WindowResize*>(p_event);
			io.DisplaySize = ImVec2(static_cast<float>(wnd->GetWidth()), static_cast<float>(wnd->GetHeight()));
			io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);

			break;
		}
		case TRC_BUTTON_PRESSED:
		{
			trace::MousePressed* press = reinterpret_cast<trace::MousePressed*>(p_event);
			io.AddMouseButtonEvent(translateButtonTrace_ImGui(press->GetButton()), true);
			break;
		}
		case TRC_BUTTON_RELEASED:
		{
			trace::MouseReleased* release = reinterpret_cast<trace::MouseReleased*>(p_event);
			io.AddMouseButtonEvent(translateButtonTrace_ImGui(release->GetButton()), false);
			break;
		}
		case TRC_MOUSE_MOVE:
		{
			trace::MouseMove* move = reinterpret_cast<trace::MouseMove*>(p_event);
			io.AddMousePosEvent(move->GetMouseX(), move->GetMouseY());
			break;
		}
		case TRC_MOUSE_DB_CLICK:
		{
			trace::MouseDBClick* click = reinterpret_cast<trace::MouseDBClick*>(p_event);
			io.AddMouseButtonEvent(translateButtonTrace_ImGui(click->GetButton()), true);
			break;
		}

		case TRC_KEY_TYPED:
		{
			trace::KeyTyped* typed = reinterpret_cast<trace::KeyTyped*>(p_event);
			unsigned int c = typed->GetKeyCode();
			if (c > 0 && c < 0x10000)
				io.AddInputCharacter((unsigned short)c);

			break;
		}

		case TRC_MOUSE_WHEEL:
		{
			MouseWheel* wheel = reinterpret_cast<MouseWheel*>(p_event);
			io.AddMouseWheelEvent(wheel->GetMouseX(), wheel->GetMouseY());
			break;
		}

		}

		for (auto& i : m_windows)
		{
			if (i->CanReceiveEvent())
			{
				i->OnEvent(p_event);
			}
		}

	}

	std::string TraceEditor::DrawModelsPopup()
	{
		std::string res;
		ImGui::SetNextWindowSize({ 280.0f, 320.0f });
		ImGuiWindowFlags pop_flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar;
		if (ImGui::BeginPopup("ALL_MODELS"))
		{
			ImVec2 avail = ImGui::GetContentRegionAvail();
			float padding = 16.0f;
			float cell_size = 64.0f + padding;
			int column_count = (int)(avail.x / cell_size);
			if (column_count <= 0) column_count = 1;



			ImGui::Columns(column_count, nullptr, false);

			for (auto& i : m_allAssets.models)
			{
				std::string filename = i.filename().string();
				void* textureID = nullptr;
				UIFunc::GetDrawTextureHandle(m_contentBrowser->GetDefaultIcon().get(), textureID);
				if (ImGui::ImageButton(filename.c_str(), textureID, { 64.0f, 64.0f }, { 0, 1 }, { 1, 0 }))
				{
					res = i.string();
					ImGui::CloseCurrentPopup();
				}
				ImGui::TextWrapped(filename.c_str());

				ImGui::NextColumn();
			}
			ImGui::Columns(1);
			ImGui::EndPopup();
		}
		return res;
	}

	std::string TraceEditor::DrawMaterialsPopup()
	{
		std::string res;
		ImGui::SetNextWindowSize({ 280.0f, 320.0f });
		ImGuiWindowFlags pop_flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar;
		if (ImGui::BeginPopup("ALL_MATERIALS"))
		{
			ImVec2 avail = ImGui::GetContentRegionAvail();
			float padding = 16.0f;
			float cell_size = 64.0f + padding;
			int column_count = (int)(avail.x / cell_size);
			if (column_count <= 0) column_count = 1;

			ImGui::Columns(column_count, nullptr, false);

			for (auto& i : m_allAssets.materials)
			{
				std::string filename = i.filename().string();
				void* textureID = nullptr;
				UIFunc::GetDrawTextureHandle(m_contentBrowser->GetDefaultIcon().get(), textureID);
				if (ImGui::ImageButton(filename.c_str(), textureID, { 64.0f, 64.0f }, { 0, 1 }, { 1, 0 }))
				{
					res = i.string();
					ImGui::CloseCurrentPopup();
				}
				ImGui::TextWrapped(filename.c_str());

				ImGui::NextColumn();
			}
			ImGui::Columns(1);
			ImGui::EndPopup();
		}
		return res;
	}

	bool TraceEditor::DrawTexturesPopup(std::string& result)
	{
		bool res = true;
		ImGui::SetNextWindowSize({ 280.0f, 320.0f });
		ImGui::SetNextWindowPos(ImGui::GetMousePos(), ImGuiCond_Appearing);

		ImGuiWindowFlags pop_flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar;

		ImGui::Begin("ALL_TEXTURES", nullptr, ImGuiWindowFlags_NoDocking);
		ImVec2 avail = ImGui::GetContentRegionAvail();
		float padding = 16.0f;
		float cell_size = 64.0f + padding;
		int column_count = (int)(avail.x / cell_size);
		if (column_count <= 0) column_count = 1;


		ImGui::Columns(column_count, nullptr, false);

		for (auto& i : m_allAssets.textures)
		{
			std::string filename = i.filename().string();
			void* textureID = nullptr;
			UIFunc::GetDrawTextureHandle(m_contentBrowser->GetDefaultIcon().get(), textureID);
			if (ImGui::ImageButton(filename.c_str(), textureID, { 64.0f, 64.0f }, { 0, 1 }, { 1, 0 }))
			{
				result = i.string();
			}
			ImGui::TextWrapped(filename.c_str());

			ImGui::NextColumn();
		}
		ImGui::Columns(1);

		if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGui::IsWindowFocused())
		{
			res = false;
		}
		ImGui::End();


		return res;
	}

	bool TraceEditor::DrawPipelinesPopup(std::string& result)
	{
		bool res = true;
		ImGui::SetNextWindowSize({ 280.0f, 320.0f });
		ImGui::SetNextWindowPos(ImGui::GetMousePos(), ImGuiCond_Appearing);

		ImGuiWindowFlags pop_flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar;

		ImGui::Begin("ALL_PIPELINES", nullptr, ImGuiWindowFlags_NoDocking);
		ImVec2 avail = ImGui::GetContentRegionAvail();
		float padding = 16.0f;
		float cell_size = 64.0f + padding;
		int column_count = (int)(avail.x / cell_size);
		if (column_count <= 0) column_count = 1;


		ImGui::Columns(column_count, nullptr, false);

		for (auto& i : m_allAssets.pipelines)
		{
			std::string filename = i.filename().string();
			void* textureID = nullptr;
			UIFunc::GetDrawTextureHandle(m_contentBrowser->GetDefaultIcon().get(), textureID);
			if (ImGui::ImageButton(filename.c_str(), textureID, { 64.0f, 64.0f }, { 0, 1 }, { 1, 0 }))
			{
				result = i.string();
			}
			ImGui::TextWrapped(filename.c_str());

			ImGui::NextColumn();
		}
		ImGui::Columns(1);

		if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGui::IsWindowFocused())
		{
			res = false;
		}
		ImGui::End();


		return res;
	}

	bool TraceEditor::DrawShadersPopup(std::string& result)
	{
		bool res = true;
		ImGui::SetNextWindowSize({ 280.0f, 320.0f });
		ImGui::SetNextWindowPos(ImGui::GetMousePos(), ImGuiCond_Appearing);

		ImGuiWindowFlags pop_flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar;

		ImGui::Begin("ALL_SHADERS", nullptr, ImGuiWindowFlags_NoDocking);
		ImVec2 avail = ImGui::GetContentRegionAvail();
		float padding = 16.0f;
		float cell_size = 64.0f + padding;
		int column_count = (int)(avail.x / cell_size);
		if (column_count <= 0) column_count = 1;


		ImGui::Columns(column_count, nullptr, false);

		for (auto& i : m_allAssets.shaders)
		{
			std::string filename = i.filename().string();
			void* textureID = nullptr;
			UIFunc::GetDrawTextureHandle(m_contentBrowser->GetDefaultIcon().get(), textureID);
			if (ImGui::ImageButton(filename.c_str(), textureID, { 64.0f, 64.0f }, { 0, 1 }, { 1, 0 }))
			{
				result = i.string();
			}
			ImGui::TextWrapped(filename.c_str());

			ImGui::NextColumn();
		}
		ImGui::Columns(1);

		if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGui::IsWindowFocused())
		{
			res = false;
		}
		ImGui::End();


		return res;
	}

	bool TraceEditor::InputTextPopup(const std::string& label, std::string& result)
	{
		bool res = true;

		ImGui::SetNextWindowPos(ImGui::GetMousePos(), ImGuiCond_Appearing);

		if (ImGui::Begin(label.c_str(), nullptr, ImGuiWindowFlags_NoDocking))
		{
			static std::string value;
			ImGui::InputText(("##" + label).c_str(), &value);
			if (ImGui::IsKeyReleased(ImGuiKey_Enter))
			{
				result = value;
				value.clear();
			}

			if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGui::IsWindowFocused())
			{
				res = false;
			}

			ImGui::End();
		}

		return res;
	}


	TraceEditor* TraceEditor::get_instance()
	{
		static TraceEditor* s_instance = new TraceEditor;
		return s_instance;
	}
	
	
	void TraceEditor::OpenSkeleton(std::string& path)
	{
		if (Ref<Animation::Skeleton> result = AnimationsSerializer::DeserializeSkeleton(path))
		{
			skeleton = result;
			rig = skeleton->GetHumanoidRig();
			show_humanoid_window = true;
		}
	}
	void TraceEditor::OpenFeatureDB(std::string& path)
	{
		if (Ref<MotionMatching::FeatureDatabase> new_db = GenericSerializer::Deserialize<MotionMatching::FeatureDatabase>(path))
		{
			feature_db = new_db;
			show_feature_db_window = true;
		}
	}
	void TraceEditor::OpenMMTInfo(std::string& path)
	{
		if (Ref<MotionMatching::MotionMatchingInfo> new_info = GenericSerializer::Deserialize<MotionMatching::MotionMatchingInfo>(path))
		{
			mmt_info = new_info;
			show_mmt_info_window = true;
		}
	}
	void TraceEditor::OpenPrefab(std::string& path)
	{
		CreateEditorWindow<PrefabWindow>(path, path);
	}
	void TraceEditor::OpenScene(std::string& path)
	{
		Ref<Scene> scene = SceneSerializer::Deserialize(path);
		if (scene)
		{
			SetNextScene(scene);
		}
	}
	void TraceEditor::OpenAnimationGraph(std::string& path)
	{
		CreateEditorWindow<AnimationGraphWindow>(path, path);
	}
	void TraceEditor::OpenAnimationSequence(std::string& path)
	{
		CreateEditorWindow<SequenceWindow>(path, path);
	}
	void TraceEditor::OpenAnimationClip(std::string& path)
	{
		CreateEditorWindow<AnimationWindow>(path, path);
	}
	void TraceEditor::HandleKeyPressed(KeyPressed* p_event)
	{
		InputSystem* input = InputSystem::get_instance();
		bool control = input->GetKey(KEY_LCONTROL) || input->GetKey(KEY_RCONTROL) || input->GetKey(Keys::KEY_CONTROL);
		bool shift = input->GetKey(KEY_LSHIFT) || input->GetKey(KEY_RSHIFT) || input->GetKey(Keys::KEY_SHIFT);
		switch (p_event->GetKeyCode())
		{
		case KEY_T:
		{

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

		}

	}
	void TraceEditor::HandleKeyRelesed(KeyReleased* p_event)
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

			}
			break;
		}
		}

	}
	

	bool TraceEditor::SetNextScene(Ref<Scene> scene)
	{
		scene_window->SetNextScene(scene);
		return true;
	}

	void TraceEditor::AddWindowToDockspace(const std::string& window_name)
	{
		ImGui::DockBuilderDockWindow(window_name.c_str(), main_dockspace_Top);
	}

	void TraceEditor::Update_Tester(float deltaTime)
	{
		/*if (!m_currentScene)
		{
			return;
		}

		glm::vec2 gamepad(0.0f);
		if (InputSystem::get_instance()->GetKey(Keys::KEY_W))
		{
			gamepad.y -= 1.0f;
		}
		if (InputSystem::get_instance()->GetKey(Keys::KEY_S))
		{
			gamepad.y += 1.0f;
		}
		if (InputSystem::get_instance()->GetKey(Keys::KEY_A))
		{
			gamepad.x -= 1.0f;
		}
		if (InputSystem::get_instance()->GetKey(Keys::KEY_D))
		{
			gamepad.x += 1.0f;
			
		}

		Entity x_bot = m_currentScene->GetEntityByName("X_bot");
		if (x_bot)
		{
			if (SpringMotionMatchingController* spring_ctrl = x_bot.TryGetComponent<SpringMotionMatchingController>())
			{
				spring_ctrl->target_dir = glm::vec3(gamepad.x * speed, 0.0f, gamepad.y * speed);
			}
		}*/
		

	}

	bool TraceEditor::CreateProject(const std::string& dir, const std::string& name)
	{
		if (!std::filesystem::exists(dir))
		{
			TRC_ERROR("Directory dosen't exits, dir:{}", dir);
			return false;
		}

		if(!std::filesystem::is_empty(dir))
		{
			TRC_ERROR("Directory is not empty, Please select an empty directory, dir:{}", dir);
			return false;
		}

		if (name.empty())
		{
			TRC_ERROR("Please enter a name for the project");
			return false;
		}


		std::string premake_data = R"(
workspace "{}"
	architecture "x64"

	configurations
	{{
		"Debug",
		"Release"
	}}
project "{}"
	location "Scripts"
	kind "SharedLib"
	language "C#"
	dotnetframework "4.5"

	targetdir ("Data/Assembly")
	objdir ("Data/bin-int")

	files "Scripts/**.cs"
	
	links
	{{
		"TraceScriptLib.dll"
	}}

	libdirs
	{{
		"{}"
	}}

)";

		std::string lib_dir;
		std::string premake5_location;
		FindDirectory(AppSettings::exe_path, "Data/Assembly", lib_dir);
		FindDirectory(AppSettings::exe_path, "externals/premake/premake5.exe", premake5_location);
		premake_data = fmt::format(premake_data, name, name, lib_dir);

		FileHandle out_handle;
		if (FileSystem::open_file(dir + "/premake5.lua", FileMode::WRITE, out_handle))
		{
			FileSystem::writestring(out_handle, premake_data);
			FileSystem::close_file(out_handle);
		}

		std::filesystem::create_directory(dir + "/Assets");
		std::filesystem::create_directory(dir + "/Scripts");
		std::filesystem::create_directory(dir + "/Data");
		std::filesystem::create_directory(dir + "/Data/Assembly");

		std::string create_cmd = fmt::format("cd {} && {} vs2022", dir, premake5_location);
		system(create_cmd.c_str());

		Ref<Project> result;
		Project* res = new Project(); // TODO: Use custom allocator
		res->SetName(name);

		result = { res, [](Resource* res) { delete res;/*TODO: Use custom allocator*/ } };
		ProjectSerializer::Serialize(result, dir + "/" + name + ".trproj");

		

		return true;
	}

	bool TraceEditor::OpenProject()
	{
		std::vector<std::string> result = pfd::open_file("Open Project", "", {"Trace Project", "*.trproj"}).result();
		if (!result.empty())
		{
			CloseProject();
			return LoadProject(result[0]);
		}

		return false;
	}

	bool TraceEditor::LoadProject(const std::string& file_path)
	{
		m_currentProject = ProjectSerializer::Deserialize(file_path);
		if (!m_currentProject)
		{
			return false;
		}
		m_contentBrowser->SetDirectory(m_currentProject->GetAssetsDirectory());
		ReloadProjectAssembly();

		//GenerateParticleEffect(m_currentProject);

		UUID id = m_currentProject->GetStartScene();
		if (id != 0)
		{
			std::filesystem::path file_path = GetPathFromUUID(id);
			scene_window->LoadScene(file_path.string());
		}
		

		return true;
	}

	bool TraceEditor::CloseProject()
	{
		if (!m_currentProject)
		{
			return false;
		}
		/*m_currentScene.free();
		m_currentScenePath = "";*/
		m_contentBrowser->ProcessAllDirectory();
		m_contentBrowser->SerializeImportedAssets();
		m_contentBrowser->GetAllFilesID().clear();
		m_contentBrowser->GetUUIDName().clear();
		m_contentBrowser->GetUUIDPath().clear();
		m_currentProject.free();

		return true;
	}

	void TraceEditor::ReloadProjectAssembly()
	{

		if (m_currentProject)
		{
			if (scene_window->GetEditorState() != EditorState::SceneEdit)
			{
				return;
			}

			bool exists = std::filesystem::exists(m_currentProject->GetAssemblyPath());
			if (exists)
			{
				ScriptEngine::get_instance()->ReloadAssembly(m_currentProject->GetAssemblyPath());
				if (scene_window->GetCurrentScene())
				{
					scene_window->GetCurrentScene()->m_scriptRegistry.ReloadScripts();
				}
			}
		}
	}

	void TraceEditor::ProjectSettings()
	{
		enum class ProjectSettings
		{
			GENERAL,
			SETTINGS_MAX
		};
		static int current_setting = -1;
		static auto get_settings_string = [](ProjectSettings type) -> const char*
		{
			switch (type)
			{
			case ProjectSettings::GENERAL:
			{
				return "General";
			}
			}

			return "";
		};

		if (p_projectSettings && m_currentProject)
		{

			ImGui::Begin("Project Settings", &p_projectSettings);
			ImGui::Columns(2);

			for (int i = 0; i < (int)ProjectSettings::SETTINGS_MAX; i++)
			{
				bool selected = (current_setting == i);
				if (ImGui::Selectable(get_settings_string((ProjectSettings)i), selected))
					current_setting = i;
			}

			ImGui::NextColumn();

			ProjectSettings type = (ProjectSettings)current_setting;

			switch (type)
			{
			case ProjectSettings::GENERAL:
			{
				// Start Scene
				ImGui::Text("Start Scene");
				ImGui::SameLine();
				std::string scene_name = "None(Scene)";
				UUID id = m_currentProject->GetStartScene();
				if (id != 0)
				{
					std::filesystem::path file_path = GetPathFromUUID(id);
					scene_name = file_path.filename().string();
					if (scene_name.empty())
					{
						scene_name = "None(Scene)";
					}
				}
				ImGui::Button(scene_name.c_str());
				if (ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(".trscn"))
					{
						char buf[1024] = { 0 };
						memcpy_s(buf, 1024, payload->Data, payload->DataSize);
						std::filesystem::path file_path = buf;
						UUID id = GetUUIDFromName(file_path.filename().string());
						m_currentProject->SetStartScene(id);
					}
					ImGui::EndDragDropTarget();
				}
				if (ImGui::BeginItemTooltip())
				{
					ImGui::Text("Drag and drop the scene on the button");
					ImGui::EndTooltip();
				}


				break;
			}
			}

			ImGui::Columns(1);

			if (ImGui::Button("Apply"))
			{
				ProjectSerializer::Serialize(m_currentProject, m_currentProject->m_path.string());
			}
			ImGui::End();

		}

	}

	void TraceEditor::BuildProject()
	{
		if (!m_currentProject) return;

		std::string out_dir;
		std::string result = pfd::select_folder("Select Directory").result();
		if (!result.empty())
		{
			out_dir = result;

			if (!ProjectBuilder::BuildProject(m_currentProject, out_dir, m_allAssets.scenes))
			{
				TRC_ERROR("Failed to build project");
				return;
			}
			else
			{
				TRC_INFO("Project successfully built");
			}
		}
		

	}

	void TraceEditor::RenderUtilsWindows(float deltaTime)
	{
		if (show_humanoid_window)
		{
			if(ImGui::Begin("Skeleton Viewer", &show_humanoid_window))
			{
				std::string skeleton_name = "None(Skeleton)";

				if (skeleton)
				{
					skeleton_name = skeleton->GetName();
				}

				ImGui::Text("Skeleton: ");
				ImGui::SameLine();
				ImGui::Button(skeleton_name.c_str());

				if (Ref<Animation::Skeleton> new_skeleton = ImGuiDragDropResource<Animation::Skeleton>(SKELETON_FILE_EXTENSION))
				{
					skeleton = new_skeleton;
					rig = skeleton->GetHumanoidRig();
				}

				if (skeleton)
				{

					std::string rig_name = "None(Humanoid Rig)";
					if (rig)
					{
						rig_name = rig->GetName();
					}
					ImGui::Text("Humanoid Rig: ");
					ImGui::SameLine();
					ImGui::Button(rig_name.c_str());

					/*if (ImGui::BeginTooltip())
					{
						ImGui::Text("Drag and Drop Humanoid Rig Asset");
						ImGui::EndTooltip();
					}*/

					if (Ref<Animation::HumanoidRig> new_rig = ImGuiDragDropResource<Animation::HumanoidRig>(HUMANOID_RIG_FILE_EXTENSION))
					{
						rig = new_rig;
						skeleton->SetHumanoidRig(rig);
					}

					ImGui::Separator();
					int32_t index = 0;
					for (Animation::Bone& bone : skeleton->GetBones())
					{
						ImGui::PushID(index);
						std::string& bone_name = STRING_FROM_ID(bone.GetStringID());
						ImGui::Button(bone_name.c_str());

						if (ImGui::BeginDragDropSource())
						{
							ImGui::SetDragDropPayload("Bone_IDX", &index, sizeof(int32_t));
							ImGui::EndDragDropSource();
						}

						ImGui::PopID();

						++index;
					}
				}
				if (ImGui::Button("Save") && skeleton)
				{
					std::string skeleton_name = skeleton->GetName();
					std::filesystem::path file_path = GetPathFromUUID(GetUUIDFromName(skeleton_name));
					AnimationsSerializer::SerializeSkeleton(skeleton, file_path.string());
				}


			
			
			

				ImGui::End();
			}

		
			if (!rig)
			{
				return;
			}

			ImGui::Begin("Humanoid Rig Modifier");

			if (rig)
			{
				auto& rig_bones = rig->GetHumanoidBones();

				int32_t index = 0;
				for (int32_t& i : rig_bones)
				{
					ImGui::PushID(index);
					ImGui::Text("%s : ", Animation::get_humanoid_bone_text((Animation::HumanoidBone)index));
					ImGui::SameLine();
					std::string bone_name = "None(Bone)";

					if (skeleton && i != -1)
					{
						bone_name = STRING_FROM_ID(skeleton->GetBone(i)->GetStringID());
					}

					ImGui::Button(bone_name.c_str());

					if (ImGui::BeginDragDropTarget())
					{
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Bone_IDX"))
						{
							int32_t bone_index = -1;
							memcpy_s(&bone_index, sizeof(int32_t), payload->Data, payload->DataSize);
							i = bone_index;
						}
						ImGui::EndDragDropTarget();
					}

					ImGui::PopID();

					++index;
				}
				if (ImGui::Button("Save Humanoid Rig"))
				{
					std::string rig_name = rig->GetName();
					std::filesystem::path file_path = GetPathFromUUID(GetUUIDFromName(rig_name));
					AnimationsSerializer::SerializeHumanoidRig(rig, file_path.string());
				}


			}

			ImGui::End();
		}

		if (show_feature_db_window)
		{
			if (ImGui::Begin("Feature Database", &show_feature_db_window))
			{
				std::string db_name = "None(Feature DB)";

				if (feature_db)
				{
					db_name = feature_db->GetName();
				}

				ImGui::Text("FeatureDB: ");
				ImGui::SameLine();
				ImGui::Button(db_name.c_str());

				if (Ref<MotionMatching::FeatureDatabase> new_feature_db = ImGuiDragDropResource<MotionMatching::FeatureDatabase>(FEATURE_DB_FILE_EXTENSION))
				{
					feature_db = new_feature_db;
				}

				// MMT Info --------------------------------------
				std::string mmt_info_name = "None(Motion Matching Info)";

				if (feature_db->GetMotionMatchingInfo())
				{
					mmt_info_name = feature_db->GetMotionMatchingInfo()->GetName();
				}

				ImGui::Text("Motion Matching Info: ");
				ImGui::SameLine();
				ImGui::Button(mmt_info_name.c_str());

				if (Ref<MotionMatching::MotionMatchingInfo> new_mmt_info = ImGuiDragDropResource<MotionMatching::MotionMatchingInfo>(MMT_INFO_FILE_EXTENSION))
				{
					feature_db->SetMotionMatchingInfo(new_mmt_info);
				}

				// --------------------------------------------------

				if (feature_db)
				{

					int32_t index = 0;
					for (auto& [idx, clip] : feature_db->GetAnimations())
					{
						ImGui::PushID(index);
						std::string clip_name = clip->GetName();
						ImGui::Button(clip_name.c_str());

						
						ImGui::PopID();

						++index;
					}
				}
				if (ImGui::Button("Add Animation") && feature_db)
				{
					
				}

				Ref<AnimationClip> new_clip = ImGuiDragDropResource<AnimationClip>(ANIMATION_CLIP_FILE_EXTENSION);
				if (new_clip && feature_db && skeleton)
				{
					feature_db->ExtractPoseData(new_clip, skeleton);
					feature_db->NormalizeDatabase();
				}
				ImGui::SameLine();
				if (ImGui::Button("Clear") && feature_db)
				{
					feature_db->Clear();

				}
				if (ImGui::Button("Save") && feature_db)
				{
					std::string feature_db_name = feature_db->GetName();
					std::filesystem::path file_path = GetPathFromUUID(GetUUIDFromName(feature_db_name));
					GenericSerializer::Serialize<MotionMatching::FeatureDatabase>(feature_db, file_path.string());
				}






				ImGui::End();
			}
		}

		if (show_mmt_info_window)
		{
			if (ImGui::Begin("Motion Matching Info", &show_mmt_info_window))
			{
				std::string info_name = "None(Motion Matching Info)";

				if (mmt_info)
				{
					info_name = mmt_info->GetName();
				}

				ImGui::Text("Motion Matching Info: ");
				ImGui::SameLine();
				ImGui::Button(info_name.c_str());

				if (Ref<MotionMatching::MotionMatchingInfo> new_info = ImGuiDragDropResource<MotionMatching::MotionMatchingInfo>(MMT_INFO_FILE_EXTENSION))
				{
					mmt_info = new_info;
				}

				if (mmt_info && skeleton)
				{
					ImGui::Text("Pose Features");

					int32_t index_to_remove = -1;
					int32_t i = 1;
					for (auto& bone : mmt_info->pose_features)
					{
						std::string bone_name = "None(Bone)";
						int32_t _index = (int32_t)bone;

						if (skeleton && _index != -1)
						{
							bone_name = STRING_FROM_ID(skeleton->GetBone(_index)->GetStringID());
						}

						ImGui::PushID(i);
						ImGui::Button(bone_name.c_str());

						if (ImGui::BeginDragDropTarget())
						{
							if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Bone_IDX"))
							{
								int32_t bone_index = -1;
								memcpy_s(&bone_index, sizeof(int32_t), payload->Data, payload->DataSize);
								bone = (Animation::HumanoidBone)bone_index;
							}
							ImGui::EndDragDropTarget();
						}

						ImGui::SameLine();
						if (ImGui::Button("-"))
						{
							index_to_remove = i - 1;
						}
						ImGui::PopID();

						++i;
					}

					if (index_to_remove >= 0)
					{
						mmt_info->pose_features[index_to_remove] = mmt_info->pose_features.back();
						mmt_info->pose_features.pop_back();
					}

					if (ImGui::Button("Add Bone"))
					{
						mmt_info->pose_features.emplace_back((Animation::HumanoidBone)-1);
					}


					ImGui::Text("Trajectory Features");

					index_to_remove = -1;
					i = 1;
					for (int32_t& data : mmt_info->trajectory_features)
					{
						ImGui::PushID(i);
						
						ImGui::InputInt("##Track_Data", &data);

						ImGui::SameLine();
						if (ImGui::Button("--"))
						{
							index_to_remove = i - 1;
						}
						ImGui::PopID();

						++i;
					}

					if (index_to_remove >= 0)
					{
						mmt_info->trajectory_features[index_to_remove] = mmt_info->trajectory_features.back();
						mmt_info->trajectory_features.pop_back();
					}

					if (ImGui::Button("+"))
					{
						mmt_info->trajectory_features.emplace_back(0);
					}

					ImGui::InputInt("Frames Per Second", &mmt_info->frames_per_second);
				}

				if (ImGui::Button("Save Motion Matching Info"))
				{
					std::string info_name = mmt_info->GetName();
					std::filesystem::path file_path = GetPathFromUUID(GetUUIDFromName(info_name));
					GenericSerializer::Serialize<MotionMatching::MotionMatchingInfo>(mmt_info, file_path.string());
				}

				ImGui::End();
			}
		}

	}	

}



// From https://github.com/procedural/gpulib/blob/master/gpulib_imgui.h
struct ImVec3 { float x, y, z; ImVec3(float _x = 0.0f, float _y = 0.0f, float _z = 0.0f) { x = _x; y = _y; z = _z; } };

void imgui_easy_theming(ImVec3 color_for_text, ImVec3 color_for_head, ImVec3 color_for_area, ImVec3 color_for_body, ImVec3 color_for_pops)
{
	ImGuiStyle& style = ImGui::GetStyle();

	style.Colors[ImGuiCol_Text] = ImVec4(color_for_text.x, color_for_text.y, color_for_text.z, 1.00f);
	style.Colors[ImGuiCol_TextDisabled] = ImVec4(color_for_text.x, color_for_text.y, color_for_text.z, 0.58f);
	style.Colors[ImGuiCol_WindowBg] = ImVec4(color_for_body.x, color_for_body.y, color_for_body.z, 0.95f);
	//style.Colors[ImGuiCol_ChildWindowBg] = ImVec4(color_for_area.x, color_for_area.y, color_for_area.z, 0.58f);
	style.Colors[ImGuiCol_Border] = ImVec4(color_for_body.x, color_for_body.y, color_for_body.z, 0.00f);
	style.Colors[ImGuiCol_BorderShadow] = ImVec4(color_for_body.x, color_for_body.y, color_for_body.z, 0.00f);
	style.Colors[ImGuiCol_FrameBg] = ImVec4(color_for_area.x, color_for_area.y, color_for_area.z, 1.00f);
	style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 0.78f);
	style.Colors[ImGuiCol_FrameBgActive] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 1.00f);
	style.Colors[ImGuiCol_TitleBg] = ImVec4(color_for_area.x, color_for_area.y, color_for_area.z, 1.00f);
	style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(color_for_area.x, color_for_area.y, color_for_area.z, 0.75f);
	style.Colors[ImGuiCol_TitleBgActive] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 1.00f);
	style.Colors[ImGuiCol_MenuBarBg] = ImVec4(color_for_area.x, color_for_area.y, color_for_area.z, 0.47f);
	style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(color_for_area.x, color_for_area.y, color_for_area.z, 1.00f);
	style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 0.21f);
	style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 0.78f);
	style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 1.00f);
	//style.Colors[ImGuiCol_ComboBg] = ImVec4(color_for_area.x, color_for_area.y, color_for_area.z, 1.00f);
	style.Colors[ImGuiCol_CheckMark] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 0.80f);
	style.Colors[ImGuiCol_SliderGrab] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 0.50f);
	style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 1.00f);
	style.Colors[ImGuiCol_Button] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 0.50f);
	style.Colors[ImGuiCol_ButtonHovered] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 0.86f);
	style.Colors[ImGuiCol_ButtonActive] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 1.00f);
	style.Colors[ImGuiCol_Header] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 0.76f);
	style.Colors[ImGuiCol_HeaderHovered] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 0.86f);
	style.Colors[ImGuiCol_HeaderActive] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 1.00f);
	//style.Colors[ImGuiCol_Column] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 0.32f);
	//style.Colors[ImGuiCol_ColumnHovered] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 0.78f);
	//style.Colors[ImGuiCol_ColumnActive] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 1.00f);
	style.Colors[ImGuiCol_ResizeGrip] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 0.15f);
	style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 0.78f);
	style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 1.00f);
	//style.Colors[ImGuiCol_CloseButton] = ImVec4(color_for_text.x, color_for_text.y, color_for_text.z, 0.16f);
	//style.Colors[ImGuiCol_CloseButtonHovered] = ImVec4(color_for_text.x, color_for_text.y, color_for_text.z, 0.39f);
	//style.Colors[ImGuiCol_CloseButtonActive] = ImVec4(color_for_text.x, color_for_text.y, color_for_text.z, 1.00f);
	style.Colors[ImGuiCol_PlotLines] = ImVec4(color_for_text.x, color_for_text.y, color_for_text.z, 0.63f);
	style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 1.00f);
	style.Colors[ImGuiCol_PlotHistogram] = ImVec4(color_for_text.x, color_for_text.y, color_for_text.z, 0.63f);
	style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 1.00f);
	style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(color_for_head.x, color_for_head.y, color_for_head.z, 0.43f);
	style.Colors[ImGuiCol_PopupBg] = ImVec4(color_for_pops.x, color_for_pops.y, color_for_pops.z, 0.92f);
	//style.Colors[ImGuiCol_ModalWindowDarkening] = ImVec4(color_for_area.x, color_for_area.y, color_for_area.z, 0.73f);
}

void SetupImGuiStyle2()
{
	static ImVec3 color_for_text = ImVec3(236.f / 255.f, 240.f / 255.f, 241.f / 255.f);
	static ImVec3 color_for_head = ImVec3(41.f / 255.f, 128.f / 255.f, 185.f / 255.f);
	static ImVec3 color_for_area = ImVec3(57.f / 255.f, 79.f / 255.f, 105.f / 255.f);
	static ImVec3 color_for_body = ImVec3(44.f / 255.f, 62.f / 255.f, 80.f / 255.f);
	static ImVec3 color_for_pops = ImVec3(33.f / 255.f, 46.f / 255.f, 60.f / 255.f);
	imgui_easy_theming(color_for_text, color_for_head, color_for_area, color_for_body, color_for_pops);
}

void light_yellow()
{
	// Setup style
	ImGuiStyle& style = ImGui::GetStyle();
	style.Colors[ImGuiCol_Text] = ImVec4(0.31f, 0.25f, 0.24f, 1.00f);
	style.Colors[ImGuiCol_WindowBg] = ImVec4(0.94f, 0.94f, 0.94f, 1.00f);
	style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.74f, 0.74f, 0.94f, 1.00f);
	//style.Colors[ImGuiCol_ChildWindowBg] = ImVec4(0.68f, 0.68f, 0.68f, 0.00f);
	style.Colors[ImGuiCol_Border] = ImVec4(0.50f, 0.50f, 0.50f, 0.60f);
	style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	style.Colors[ImGuiCol_FrameBg] = ImVec4(0.62f, 0.70f, 0.72f, 0.56f);
	style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.95f, 0.33f, 0.14f, 0.47f);
	style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.97f, 0.31f, 0.13f, 0.81f);
	style.Colors[ImGuiCol_TitleBg] = ImVec4(0.42f, 0.75f, 1.00f, 0.53f);
	style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.40f, 0.65f, 0.80f, 0.20f);
	style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.40f, 0.62f, 0.80f, 0.15f);
	style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.39f, 0.64f, 0.80f, 0.30f);
	style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.28f, 0.67f, 0.80f, 0.59f);
	style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.25f, 0.48f, 0.53f, 0.67f);
	//style.Colors[ImGuiCol_ComboBg] = ImVec4(0.89f, 0.98f, 1.00f, 0.99f);
	style.Colors[ImGuiCol_CheckMark] = ImVec4(0.48f, 0.47f, 0.47f, 0.71f);
	style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.31f, 0.47f, 0.99f, 1.00f);
	style.Colors[ImGuiCol_Button] = ImVec4(1.00f, 0.79f, 0.18f, 0.78f);
	style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.42f, 0.82f, 1.00f, 0.81f);
	style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.72f, 1.00f, 1.00f, 0.86f);
	style.Colors[ImGuiCol_Header] = ImVec4(0.65f, 0.78f, 0.84f, 0.80f);
	style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.75f, 0.88f, 0.94f, 0.80f);
	style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.55f, 0.68f, 0.74f, 0.80f);//ImVec4(0.46f, 0.84f, 0.90f, 1.00f);
	style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.60f, 0.60f, 0.80f, 0.30f);
	style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(1.00f, 1.00f, 1.00f, 0.60f);
	style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(1.00f, 1.00f, 1.00f, 0.90f);
	//style.Colors[ImGuiCol_CloseButton] = ImVec4(0.41f, 0.75f, 0.98f, 0.50f);
	//style.Colors[ImGuiCol_CloseButtonHovered] = ImVec4(1.00f, 0.47f, 0.41f, 0.60f);
	//style.Colors[ImGuiCol_CloseButtonActive] = ImVec4(1.00f, 0.16f, 0.00f, 1.00f);
	style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(1.00f, 0.99f, 0.54f, 0.43f);
	//style.Colors[ImGuiCol_TooltipBg] = ImVec4(0.82f, 0.92f, 1.00f, 0.90f);
	style.Alpha = 1.0f;
	//style.WindowFillAlphaDefault = 1.0f;
	style.FrameRounding = 4;
	style.IndentSpacing = 12.0f;
}

void dark_green()
{
	ImVec4* colors = ImGui::GetStyle().Colors;
	colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
	colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.06f, 0.06f, 0.94f);
	colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
	colors[ImGuiCol_Border] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
	colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_FrameBg] = ImVec4(0.44f, 0.44f, 0.44f, 0.60f);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.57f, 0.57f, 0.57f, 0.70f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.76f, 0.76f, 0.76f, 0.80f);
	colors[ImGuiCol_TitleBg] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
	colors[ImGuiCol_TitleBgActive] = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
	colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.60f);
	colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
	colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
	colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
	colors[ImGuiCol_CheckMark] = ImVec4(0.13f, 0.75f, 0.55f, 0.80f);
	colors[ImGuiCol_SliderGrab] = ImVec4(0.13f, 0.75f, 0.75f, 0.80f);
	colors[ImGuiCol_SliderGrabActive] = ImVec4(0.13f, 0.75f, 1.00f, 0.80f);
	colors[ImGuiCol_Button] = ImVec4(0.13f, 0.75f, 0.55f, 0.40f);
	colors[ImGuiCol_ButtonHovered] = ImVec4(0.13f, 0.75f, 0.75f, 0.60f);
	colors[ImGuiCol_ButtonActive] = ImVec4(0.13f, 0.75f, 1.00f, 0.80f);
	colors[ImGuiCol_Header] = ImVec4(0.13f, 0.75f, 0.55f, 0.40f);
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.13f, 0.75f, 0.75f, 0.60f);
	colors[ImGuiCol_HeaderActive] = ImVec4(0.13f, 0.75f, 1.00f, 0.80f);
	colors[ImGuiCol_Separator] = ImVec4(0.13f, 0.75f, 0.55f, 0.40f);
	colors[ImGuiCol_SeparatorHovered] = ImVec4(0.13f, 0.75f, 0.75f, 0.60f);
	colors[ImGuiCol_SeparatorActive] = ImVec4(0.13f, 0.75f, 1.00f, 0.80f);
	colors[ImGuiCol_ResizeGrip] = ImVec4(0.13f, 0.75f, 0.55f, 0.40f);
	colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.13f, 0.75f, 0.75f, 0.60f);
	colors[ImGuiCol_ResizeGripActive] = ImVec4(0.13f, 0.75f, 1.00f, 0.80f);
	colors[ImGuiCol_Tab] = ImVec4(0.13f, 0.75f, 0.55f, 0.80f);
	colors[ImGuiCol_TabHovered] = ImVec4(0.13f, 0.75f, 0.75f, 0.80f);
	colors[ImGuiCol_TabActive] = ImVec4(0.13f, 0.75f, 1.00f, 0.80f);
	colors[ImGuiCol_TabUnfocused] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
	colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.36f, 0.36f, 0.36f, 0.54f);
	colors[ImGuiCol_DockingPreview] = ImVec4(0.13f, 0.75f, 0.55f, 0.80f);
	colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.13f, 0.13f, 0.13f, 0.80f);
	colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
	colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
	colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
	colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
	colors[ImGuiCol_TableHeaderBg] = ImVec4(0.19f, 0.19f, 0.20f, 1.00f);
	colors[ImGuiCol_TableBorderStrong] = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);
	colors[ImGuiCol_TableBorderLight] = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);
	colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.07f);
	colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
	colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
	colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
	colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
	colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
}

void dark_red()
{
	ImVec4* colors = ImGui::GetStyle().Colors;
	colors[ImGuiCol_Text] = ImVec4(0.75f, 0.75f, 0.75f, 1.00f);
	colors[ImGuiCol_TextDisabled] = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
	colors[ImGuiCol_WindowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.94f);
	colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
	colors[ImGuiCol_Border] = ImVec4(0.00f, 0.00f, 0.00f, 0.50f);
	colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_FrameBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.54f);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.37f, 0.14f, 0.14f, 0.67f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.39f, 0.20f, 0.20f, 0.67f);
	colors[ImGuiCol_TitleBg] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
	colors[ImGuiCol_TitleBgActive] = ImVec4(0.48f, 0.16f, 0.16f, 1.00f);
	colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.48f, 0.16f, 0.16f, 1.00f);
	colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
	colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
	colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
	colors[ImGuiCol_CheckMark] = ImVec4(0.56f, 0.10f, 0.10f, 1.00f);
	colors[ImGuiCol_SliderGrab] = ImVec4(1.00f, 0.19f, 0.19f, 0.40f);
	colors[ImGuiCol_SliderGrabActive] = ImVec4(0.89f, 0.00f, 0.19f, 1.00f);
	colors[ImGuiCol_Button] = ImVec4(1.00f, 0.19f, 0.19f, 0.40f);
	colors[ImGuiCol_ButtonHovered] = ImVec4(0.80f, 0.17f, 0.00f, 1.00f);
	colors[ImGuiCol_ButtonActive] = ImVec4(0.89f, 0.00f, 0.19f, 1.00f);
	colors[ImGuiCol_Header] = ImVec4(0.33f, 0.35f, 0.36f, 0.53f);
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.76f, 0.28f, 0.44f, 0.67f);
	colors[ImGuiCol_HeaderActive] = ImVec4(0.47f, 0.47f, 0.47f, 0.67f);
	colors[ImGuiCol_Separator] = ImVec4(0.32f, 0.32f, 0.32f, 1.00f);
	colors[ImGuiCol_SeparatorHovered] = ImVec4(0.32f, 0.32f, 0.32f, 1.00f);
	colors[ImGuiCol_SeparatorActive] = ImVec4(0.32f, 0.32f, 0.32f, 1.00f);
	colors[ImGuiCol_ResizeGrip] = ImVec4(1.00f, 1.00f, 1.00f, 0.85f);
	colors[ImGuiCol_ResizeGripHovered] = ImVec4(1.00f, 1.00f, 1.00f, 0.60f);
	colors[ImGuiCol_ResizeGripActive] = ImVec4(1.00f, 1.00f, 1.00f, 0.90f);
	colors[ImGuiCol_Tab] = ImVec4(0.07f, 0.07f, 0.07f, 0.51f);
	colors[ImGuiCol_TabHovered] = ImVec4(0.86f, 0.23f, 0.43f, 0.67f);
	colors[ImGuiCol_TabActive] = ImVec4(0.19f, 0.19f, 0.19f, 0.57f);
	colors[ImGuiCol_TabUnfocused] = ImVec4(0.05f, 0.05f, 0.05f, 0.90f);
	colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.13f, 0.13f, 0.13f, 0.74f);
	colors[ImGuiCol_DockingPreview] = ImVec4(0.47f, 0.47f, 0.47f, 0.47f);
	colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
	colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
	colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
	colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
	colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
	colors[ImGuiCol_TableHeaderBg] = ImVec4(0.19f, 0.19f, 0.20f, 1.00f);
	colors[ImGuiCol_TableBorderStrong] = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);
	colors[ImGuiCol_TableBorderLight] = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);
	colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.07f);
	colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
	colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
	colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
	colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
	colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
}

void embraceTheDarkness()
{
	ImVec4* colors = ImGui::GetStyle().Colors;
	colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
	colors[ImGuiCol_WindowBg] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
	colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_PopupBg] = ImVec4(0.19f, 0.19f, 0.19f, 0.92f);
	colors[ImGuiCol_Border] = ImVec4(0.19f, 0.19f, 0.19f, 0.29f);
	colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.24f);
	colors[ImGuiCol_FrameBg] = ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.19f, 0.19f, 0.19f, 0.54f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.20f, 0.22f, 0.23f, 1.00f);
	colors[ImGuiCol_TitleBg] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
	colors[ImGuiCol_TitleBgActive] = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
	colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
	colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
	colors[ImGuiCol_ScrollbarBg] = ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
	colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.34f, 0.34f, 0.34f, 0.54f);
	colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.40f, 0.40f, 0.40f, 0.54f);
	colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.56f, 0.56f, 0.56f, 0.54f);
	colors[ImGuiCol_CheckMark] = ImVec4(0.33f, 0.67f, 0.86f, 1.00f);
	colors[ImGuiCol_SliderGrab] = ImVec4(0.34f, 0.34f, 0.34f, 0.54f);
	colors[ImGuiCol_SliderGrabActive] = ImVec4(0.56f, 0.56f, 0.56f, 0.54f);
	colors[ImGuiCol_Button] = ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
	colors[ImGuiCol_ButtonHovered] = ImVec4(0.19f, 0.19f, 0.19f, 0.54f);
	colors[ImGuiCol_ButtonActive] = ImVec4(0.20f, 0.22f, 0.23f, 1.00f);
	colors[ImGuiCol_Header] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.00f, 0.00f, 0.00f, 0.36f);
	colors[ImGuiCol_HeaderActive] = ImVec4(0.20f, 0.22f, 0.23f, 0.33f);
	colors[ImGuiCol_Separator] = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
	colors[ImGuiCol_SeparatorHovered] = ImVec4(0.44f, 0.44f, 0.44f, 0.29f);
	colors[ImGuiCol_SeparatorActive] = ImVec4(0.40f, 0.44f, 0.47f, 1.00f);
	colors[ImGuiCol_ResizeGrip] = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
	colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.44f, 0.44f, 0.44f, 0.29f);
	colors[ImGuiCol_ResizeGripActive] = ImVec4(0.40f, 0.44f, 0.47f, 1.00f);
	colors[ImGuiCol_Tab] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
	colors[ImGuiCol_TabHovered] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
	colors[ImGuiCol_TabActive] = ImVec4(0.20f, 0.20f, 0.20f, 0.36f);
	colors[ImGuiCol_TabUnfocused] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
	colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
	colors[ImGuiCol_DockingPreview] = ImVec4(0.33f, 0.67f, 0.86f, 1.00f);
	colors[ImGuiCol_DockingEmptyBg] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
	colors[ImGuiCol_PlotLines] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
	colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
	colors[ImGuiCol_PlotHistogram] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
	colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
	colors[ImGuiCol_TableHeaderBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
	colors[ImGuiCol_TableBorderStrong] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
	colors[ImGuiCol_TableBorderLight] = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
	colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
	colors[ImGuiCol_TextSelectedBg] = ImVec4(0.20f, 0.22f, 0.23f, 1.00f);
	colors[ImGuiCol_DragDropTarget] = ImVec4(0.33f, 0.67f, 0.86f, 1.00f);
	colors[ImGuiCol_NavHighlight] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
	colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 0.00f, 0.00f, 0.70f);
	colors[ImGuiCol_NavWindowingDimBg] = ImVec4(1.00f, 0.00f, 0.00f, 0.20f);
	colors[ImGuiCol_ModalWindowDimBg] = ImVec4(1.00f, 0.00f, 0.00f, 0.35f);

	ImGuiStyle& style = ImGui::GetStyle();
	style.WindowPadding = ImVec2(8.00f, 8.00f);
	style.FramePadding = ImVec2(5.00f, 2.00f);
	style.CellPadding = ImVec2(6.00f, 6.00f);
	style.ItemSpacing = ImVec2(6.00f, 6.00f);
	style.ItemInnerSpacing = ImVec2(6.00f, 6.00f);
	style.TouchExtraPadding = ImVec2(0.00f, 0.00f);
	style.IndentSpacing = 25;
	style.ScrollbarSize = 15;
	style.GrabMinSize = 10;
	style.WindowBorderSize = 1;
	style.ChildBorderSize = 1;
	style.PopupBorderSize = 1;
	style.FrameBorderSize = 1;
	style.TabBorderSize = 1;
	style.WindowRounding = 7;
	style.ChildRounding = 4;
	style.FrameRounding = 3;
	style.PopupRounding = 4;
	style.ScrollbarRounding = 9;
	style.GrabRounding = 3;
	style.LogSliderDeadzone = 4;
	style.TabRounding = 4;
}

void dark_purple()
{
	auto& colors = ImGui::GetStyle().Colors;
	colors[ImGuiCol_WindowBg] = ImVec4{ 0.1f, 0.1f, 0.13f, 1.0f };
	colors[ImGuiCol_MenuBarBg] = ImVec4{ 0.16f, 0.16f, 0.21f, 1.0f };

	// Border
	colors[ImGuiCol_Border] = ImVec4{ 0.44f, 0.37f, 0.61f, 0.29f };
	colors[ImGuiCol_BorderShadow] = ImVec4{ 0.0f, 0.0f, 0.0f, 0.24f };

	// Text
	colors[ImGuiCol_Text] = ImVec4{ 1.0f, 1.0f, 1.0f, 1.0f };
	colors[ImGuiCol_TextDisabled] = ImVec4{ 0.5f, 0.5f, 0.5f, 1.0f };

	// Headers
	colors[ImGuiCol_Header] = ImVec4{ 0.13f, 0.13f, 0.17f, 1.0f };
	colors[ImGuiCol_HeaderHovered] = ImVec4{ 0.19f, 0.2f, 0.25f, 1.0f };
	colors[ImGuiCol_HeaderActive] = ImVec4{ 0.16f, 0.16f, 0.21f, 1.0f };

	// Buttons
	colors[ImGuiCol_Button] = ImVec4{ 0.13f, 0.13f, 0.17f, 1.0f };
	colors[ImGuiCol_ButtonHovered] = ImVec4{ 0.19f, 0.2f, 0.25f, 1.0f };
	colors[ImGuiCol_ButtonActive] = ImVec4{ 0.16f, 0.16f, 0.21f, 1.0f };
	colors[ImGuiCol_CheckMark] = ImVec4{ 0.74f, 0.58f, 0.98f, 1.0f };

	// Popups
	colors[ImGuiCol_PopupBg] = ImVec4{ 0.1f, 0.1f, 0.13f, 0.92f };

	// Slider
	colors[ImGuiCol_SliderGrab] = ImVec4{ 0.44f, 0.37f, 0.61f, 0.54f };
	colors[ImGuiCol_SliderGrabActive] = ImVec4{ 0.74f, 0.58f, 0.98f, 0.54f };

	// Frame BG
	colors[ImGuiCol_FrameBg] = ImVec4{ 0.13f, 0.13f, 0.17f, 1.0f };
	colors[ImGuiCol_FrameBgHovered] = ImVec4{ 0.19f, 0.2f, 0.25f, 1.0f };
	colors[ImGuiCol_FrameBgActive] = ImVec4{ 0.16f, 0.16f, 0.21f, 1.0f };

	// Tabs
	colors[ImGuiCol_Tab] = ImVec4{ 0.16f, 0.16f, 0.21f, 1.0f };
	colors[ImGuiCol_TabHovered] = ImVec4{ 0.24f, 0.24f, 0.32f, 1.0f };
	colors[ImGuiCol_TabActive] = ImVec4{ 0.2f, 0.22f, 0.27f, 1.0f };
	colors[ImGuiCol_TabUnfocused] = ImVec4{ 0.16f, 0.16f, 0.21f, 1.0f };
	colors[ImGuiCol_TabUnfocusedActive] = ImVec4{ 0.16f, 0.16f, 0.21f, 1.0f };

	// Title
	colors[ImGuiCol_TitleBg] = ImVec4{ 0.16f, 0.16f, 0.21f, 1.0f };
	colors[ImGuiCol_TitleBgActive] = ImVec4{ 0.16f, 0.16f, 0.21f, 1.0f };
	colors[ImGuiCol_TitleBgCollapsed] = ImVec4{ 0.16f, 0.16f, 0.21f, 1.0f };

	// Scrollbar
	colors[ImGuiCol_ScrollbarBg] = ImVec4{ 0.1f, 0.1f, 0.13f, 1.0f };
	colors[ImGuiCol_ScrollbarGrab] = ImVec4{ 0.16f, 0.16f, 0.21f, 1.0f };
	colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4{ 0.19f, 0.2f, 0.25f, 1.0f };
	colors[ImGuiCol_ScrollbarGrabActive] = ImVec4{ 0.24f, 0.24f, 0.32f, 1.0f };

	// Seperator
	colors[ImGuiCol_Separator] = ImVec4{ 0.44f, 0.37f, 0.61f, 1.0f };
	colors[ImGuiCol_SeparatorHovered] = ImVec4{ 0.74f, 0.58f, 0.98f, 1.0f };
	colors[ImGuiCol_SeparatorActive] = ImVec4{ 0.84f, 0.58f, 1.0f, 1.0f };

	// Resize Grip
	colors[ImGuiCol_ResizeGrip] = ImVec4{ 0.44f, 0.37f, 0.61f, 0.29f };
	colors[ImGuiCol_ResizeGripHovered] = ImVec4{ 0.74f, 0.58f, 0.98f, 0.29f };
	colors[ImGuiCol_ResizeGripActive] = ImVec4{ 0.84f, 0.58f, 1.0f, 0.29f };

	// Docking
	colors[ImGuiCol_DockingPreview] = ImVec4{ 0.44f, 0.37f, 0.61f, 1.0f };

	auto& style = ImGui::GetStyle();
	style.TabRounding = 4;
	style.ScrollbarRounding = 9;
	style.WindowRounding = 7;
	style.GrabRounding = 3;
	style.FrameRounding = 3;
	style.PopupRounding = 4;
	style.ChildRounding = 4;
}

void custom_solid()
{
	ImGuiStyle* style = &ImGui::GetStyle();

	style->WindowPadding = ImVec2(15, 15);
	style->WindowRounding = 5.0f;
	style->FramePadding = ImVec2(5, 5);
	style->FrameRounding = 4.0f;
	style->ItemSpacing = ImVec2(12, 8);
	style->ItemInnerSpacing = ImVec2(8, 6);
	style->IndentSpacing = 25.0f;
	style->ScrollbarSize = 15.0f;
	style->ScrollbarRounding = 9.0f;
	style->GrabMinSize = 5.0f;
	style->GrabRounding = 3.0f;

	style->Colors[ImGuiCol_Text] = ImVec4(0.80f, 0.80f, 0.83f, 1.00f);
	style->Colors[ImGuiCol_TextDisabled] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
	style->Colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
	//style->Colors[ImGuiCol_ChildWindowBg] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
	style->Colors[ImGuiCol_PopupBg] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
	style->Colors[ImGuiCol_Border] = ImVec4(0.80f, 0.80f, 0.83f, 0.88f);
	style->Colors[ImGuiCol_BorderShadow] = ImVec4(0.92f, 0.91f, 0.88f, 0.00f);
	style->Colors[ImGuiCol_FrameBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	style->Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
	style->Colors[ImGuiCol_FrameBgActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	style->Colors[ImGuiCol_TitleBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	style->Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(1.00f, 0.98f, 0.95f, 0.75f);
	style->Colors[ImGuiCol_TitleBgActive] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
	style->Colors[ImGuiCol_MenuBarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	style->Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	style->Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
	style->Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	style->Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
	//style->Colors[ImGuiCol_ComboBg] = ImVec4(0.19f, 0.18f, 0.21f, 1.00f);
	style->Colors[ImGuiCol_CheckMark] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
	style->Colors[ImGuiCol_SliderGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
	style->Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
	style->Colors[ImGuiCol_Button] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	style->Colors[ImGuiCol_ButtonHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
	style->Colors[ImGuiCol_ButtonActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	style->Colors[ImGuiCol_Header] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	style->Colors[ImGuiCol_HeaderHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	style->Colors[ImGuiCol_HeaderActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
	//style->Colors[ImGuiCol_Column] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	//style->Colors[ImGuiCol_ColumnHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
	//style->Colors[ImGuiCol_ColumnActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	style->Colors[ImGuiCol_ResizeGrip] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	style->Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	style->Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
	//style->Colors[ImGuiCol_CloseButton] = ImVec4(0.40f, 0.39f, 0.38f, 0.16f);
	//style->Colors[ImGuiCol_CloseButtonHovered] = ImVec4(0.40f, 0.39f, 0.38f, 0.39f);
	//style->Colors[ImGuiCol_CloseButtonActive] = ImVec4(0.40f, 0.39f, 0.38f, 1.00f);
	style->Colors[ImGuiCol_PlotLines] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
	style->Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
	style->Colors[ImGuiCol_PlotHistogram] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
	style->Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
	style->Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.25f, 1.00f, 0.00f, 0.43f);
	//style->Colors[ImGuiCol_ModalWindowDarkening] = ImVec4(1.00f, 0.98f, 0.95f, 0.73f);
}