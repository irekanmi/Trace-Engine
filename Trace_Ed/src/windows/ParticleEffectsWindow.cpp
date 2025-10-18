
#include "ParticleEffectsWindow.h"
#include "serialize/GenericSerializer.h"
#include "render/Renderer.h"
#include "../EditorRenderComposer.h"
#include "../panels/GenericGraphEditor.h"
#include "../panels/InspectorPanel.h"
#include "../utils/ImGui_utils.h"
#include "core/input/Input.h"
#include "resource/DefaultAssetsManager.h"
#include "external_utils.h"



#include "ImGuizmo.h"


namespace trace {



	bool ParticleEffectsWindow::OnCreate(TraceEditor* editor, const std::string& name, const std::string& file_path)
	{
		Ref<ParticleEffect> particle_effect = GenericSerializer::Deserialize<ParticleEffect>(file_path);
		if (!particle_effect)
		{
			return false;
		}
		std::string asset_name = particle_effect->GetName();
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
			TRC_ERROR("{} asset is already opened for editing, Function: {}", particle_effect->GetName(), __FUNCTION__);
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




		m_particleEffect = particle_effect;
		m_inspector = new InspectorPanel;//TODO: Use custom allocator
		m_scene = new Scene;//TODO: Use custom allocator
		m_scene->m_path = asset_name;
		m_scene->Create();



		Entity model = m_scene->CreateEntity();
		m_scene->EnableEntity(model);

		visual_id = model.GetID();

		ParticleEffectController& controller = model.AddComponent<ParticleEffectController>();
		controller.particle_effect.SetParticleEffect(m_particleEffect);
		particle_effect_instance = &controller.particle_effect;

		Entity floor = m_scene->CreateEntity();
		floor.AddComponent<ModelComponent>()._model = DefaultAssetsManager::Cube;
		floor.AddComponent<ModelRendererComponent>()._material = DefaultAssetsManager::default_material;
		m_scene->EnableEntity(floor);

		Transform& transform = floor.GetComponent<TransformComponent>()._transform;
		transform.SetPosition(glm::vec3(0.0f, -15.0f, 0.0f));
		transform.SetScale(glm::vec3(100.0f, 1.0f, 100.0f));

		editor_name = "Editor###" + asset_name + std::to_string(0);
		inspector_name = "Material Data###" + asset_name + std::to_string(1);
		viewport_name = "Viewport###" + asset_name + std::to_string(2);
		tool_bar_name = "Tool Bar###" + asset_name + std::to_string(3);

		particle_effect_path = file_path;
		m_name = asset_name;
		return true;
	}

	void ParticleEffectsWindow::OnDestroy(TraceEditor* editor)
	{



		m_scene->Destroy();
		m_particleEffect.free();
		delete m_scene;


		Renderer* renderer = Renderer::get_instance();
		EditorRenderComposer* composer = (EditorRenderComposer*)renderer->GetRenderComposer();
		composer->UnBindRenderGraphController(m_name);

	}

	void ParticleEffectsWindow::OnUpdate(float deltaTime)
	{
		
		if (!m_isOpen)
		{
			return;
		}



		m_scene->ResolveHierachyTransforms();
		m_scene->OnAnimationUpdate(deltaTime);


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

	void ParticleEffectsWindow::OnRender(float deltaTime)
	{

		ImGui::Begin(editor_name.c_str());

		float life_time = m_particleEffect->GetLifeTime();
		if (ImGui::DragFloat("Lifetime", &life_time, 0.25f))
		{
			m_particleEffect->SetLifeTime(life_time);
		}
		
		
		auto& generators = m_particleEffect->GetGenerators();

		for (Ref<ParticleGenerator>& gen : generators)
		{
			std::string name = gen->GetName();
			if (ImGui::Button(name.c_str()))
			{
				std::string file_path = GetPathFromUUID(gen->GetUUID()).string();
				TraceEditor::get_instance()->OpenParticleGenerator(file_path);
			}
		}

		ImGui::Dummy(ImVec2(0.0f, 7.0f));

		ImGui::Button("Add");

		if (Ref<ParticleGenerator> gen = ImGuiDragDropResource<ParticleGenerator>(PARTICLE_GENERATOR_FILE_EXTENSION))
		{
			generators.push_back(gen);
		}
		

		ImGui::End();

		ImGui::Begin(inspector_name.c_str());



		ImGui::End();

		ImGui::Begin(tool_bar_name.c_str(), false, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

		if (ImGui::Button(is_playing ? "Stop" : "Play"))
		{
			if (is_playing)
			{
				particle_effect_instance->Stop();
				particle_effect_instance->DestroyInstance();
				is_playing = false;
			}
			else
			{
				particle_effect_instance->CreateInstance(m_particleEffect, visual_id, m_scene);
				particle_effect_instance->Start();
				is_playing = true;
			}
		}
		ImGui::End();

	}

	void ParticleEffectsWindow::DockChildWindows()
	{
		ImGuiID first_left;
		ImGuiID first_right;
		ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Left, 0.225f, &first_left, &first_right);
		ImGui::DockBuilderDockWindow(viewport_name.c_str(), first_left);
		ImGuiID second_left;
		ImGuiID second_right;
		ImGui::DockBuilderSplitNode(first_right, ImGuiDir_Left, 0.75f, &second_left, &second_right);
		ImGui::DockBuilderDockWindow(inspector_name.c_str(), second_right);

		ImGuiID third_top;
		ImGuiID third_bottom;
		ImGui::DockBuilderSplitNode(second_left, ImGuiDir_Up, 0.15f, &third_top, &third_bottom);
		ImGui::DockBuilderDockWindow(tool_bar_name.c_str(), third_top);
		ImGui::DockBuilderDockWindow(editor_name.c_str(), third_bottom);
	}

	void ParticleEffectsWindow::RenderViewport(std::vector<void*>& texture_handles)
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

	void ParticleEffectsWindow::OnEvent(Event* p_event)
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
					GenericSerializer::Serialize<ParticleEffect>(m_particleEffect, particle_effect_path);
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
					
				}
				break;
			}
			case Keys::KEY_C:
			{
				if (ctrl && shift)
				{
				}
				break;
			}
			case Keys::KEY_P:
			{
				if (ctrl && shift)
				{
				}
				break;
			}
			}
			break;
		}
		}

	}



}