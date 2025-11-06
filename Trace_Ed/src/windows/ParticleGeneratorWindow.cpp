
#include "ParticleGeneratorWindow.h"
#include "serialize/GenericSerializer.h"
#include "render/Renderer.h"
#include "../EditorRenderComposer.h"
#include "../panels/GenericGraphEditor.h"
#include "../panels/InspectorPanel.h"
#include "../utils/ImGui_utils.h"
#include "core/input/Input.h"
#include "resource/DefaultAssetsManager.h"
#include "particle_effects/particle_renderers/BillboardRender.h"


#include "ImGuizmo.h"
#include "imgui_stdlib.h"


namespace trace {

	extern std::string get_class_display_name(uint64_t class_id);

	

	bool ParticleGeneratorWindow::OnCreate(TraceEditor* editor, const std::string& name, const std::string& file_path)
	{
		Ref<ParticleGenerator> particle_generator = GenericSerializer::Deserialize<ParticleGenerator>(file_path);
		if (!particle_generator)
		{
			return false;
		}
		std::string asset_name = particle_generator->GetName();
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
			TRC_ERROR("{} asset is already opened for editing, Function: {}", particle_generator->GetName(), __FUNCTION__);
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




		m_particleGenerator = particle_generator;
		m_editor = new GenericGraphEditor;//TODO: Use custom allocator
		m_inspector = new InspectorPanel;//TODO: Use custom allocator
		m_scene = new Scene;//TODO: Use custom allocator
		m_scene->m_path = asset_name;
		m_scene->Create();

		Entity model = m_scene->CreateEntity();
		m_scene->EnableEntity(model);
		visual_id = model.GetID();

		Ref<ParticleEffect> particle_effect = GenericAssetManager::get_instance()->CreateAssetHandle_<ParticleEffect>(asset_name + PARTICLE_EFFECT_FILE_EXTENSION);
		particle_effect->GetGenerators().push_back(m_particleGenerator);
		particle_effect->SetLifeTime(1000000000.f);

		ParticleEffectController& controller = model.AddComponent<ParticleEffectController>();
		controller.particle_effect.SetParticleEffect(particle_effect);
		particle_effect_instance = &controller.particle_effect;

		Entity floor = m_scene->CreateEntity();
		floor.AddComponent<ModelComponent>()._model = DefaultAssetsManager::Cube;
		floor.AddComponent<ModelRendererComponent>()._material = DefaultAssetsManager::default_material;
		m_scene->EnableEntity(floor);

		Transform& transform = floor.GetComponent<TransformComponent>()._transform;
		transform.SetPosition(glm::vec3(0.0f, -15.0f, 0.0f));
		transform.SetScale(glm::vec3(100.0f, 1.0f, 100.0f));


		graph_editor_name = "Graph Editor###" + asset_name + std::to_string(0);
		inspector_name = "Inspector###" + asset_name + std::to_string(1);
		viewport_name = "Viewport###" + asset_name + std::to_string(2);
		tool_bar_name = "Tool Bar###" + asset_name + std::to_string(3);

		m_editor->Init();
		m_editor->SetGraph(m_particleGenerator.get(), asset_name);
		if (GenericNode* final_node = m_particleGenerator->GetNode(m_particleGenerator->GetEffectRoot()))
		{
			m_editor->SetCurrentNode(final_node);
		}

		generator_path = file_path;
		m_name = asset_name;
		return true;
	}

	void ParticleGeneratorWindow::OnDestroy(TraceEditor* editor)
	{
		m_editor->Shutdown();

		m_particleGenerator.free();

		m_scene->Destroy();
		delete m_editor;
		delete m_scene;
		Renderer* renderer = Renderer::get_instance();
		EditorRenderComposer* composer = (EditorRenderComposer*)renderer->GetRenderComposer();
		composer->UnBindRenderGraphController(m_name);

	}

	void ParticleGeneratorWindow::OnUpdate(float deltaTime)
	{
		if (!m_isOpen)
		{
			return;
		}



		m_scene->ResolveHierachyTransforms();

		m_scene->OnAnimationUpdate(deltaTime);

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

		renderer->SubmitCommandList(cmd_list, view_index);

		m_camera.Update(deltaTime);


	}

	void ParticleGeneratorWindow::OnRender(float deltaTime)
	{

		ImGui::Begin(graph_editor_name.c_str());
		m_editor->Render(deltaTime);

		ImVec2 min = ImGui::GetWindowPos();
		ImVec2 max = min + ImGui::GetWindowSize();
		is_focused = is_focused || ImGui::IsMouseHoveringRect(min, max);
		ImGui::End();

		ImGui::Begin(inspector_name.c_str());

		int capacity = m_particleGenerator->GetCapacity();
		if (ImGui::DragInt("Capacity", &capacity))
		{
			m_particleGenerator->SetCapacity(capacity);
		}

		std::vector<StringID>& custom_data = m_particleGenerator->GetCustomData();

		ImGui::Text("Custom Particle Data");

		uint32_t index = 0;
		for (StringID& i : custom_data)
		{
			
			std::string custom_name = STRING_FROM_ID(i);
			
			ImGui::PushID(index);

			if (ImGui::InputText("##Custom_Name", &custom_name))
			{
				i = STR_ID(custom_name);
			}

			ImGui::PopID();
			

			index++;
		}

		ImGui::Dummy(ImVec2(0.0f, 3.0f));
		if (ImGui::Button("Add Custom Data"))
		{
			std::string data_name = "Custom" + std::to_string(index);
			custom_data.push_back(STR_ID(data_name));
		}

		ImGui::Separator();
		
		void* user_data = m_editor->GetUserData();
		if (user_data)
		{
			render_class_elements(user_data);
		}

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
				Ref<ParticleEffect> particle_effect = particle_effect_instance->GetParticleEffect();
				particle_effect_instance->CreateInstance(particle_effect, visual_id, m_scene);
				particle_effect_instance->Start();
				is_playing = true;
			}
		}
		ImGui::End();

	}

	void ParticleGeneratorWindow::DockChildWindows()
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
		ImGui::DockBuilderSplitNode(second_left, ImGuiDir_Up, 0.1f, &third_top, &third_bottom);
		ImGui::DockBuilderDockWindow(tool_bar_name.c_str(), third_top);
		ImGui::DockBuilderDockWindow(graph_editor_name.c_str(), third_bottom);
	}

	void ParticleGeneratorWindow::RenderViewport(std::vector<void*>& texture_handles)
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

	void ParticleGeneratorWindow::OnEvent(Event* p_event)
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
					GenericSerializer::Serialize<ParticleGenerator>(m_particleGenerator, generator_path);
				}
				else if (ctrl && shift)
				{
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

		m_editor->OnEvent(p_event);
	}


	void ParticleGeneratorWindow::render_class_elements(void* ptr)
	{
		ParticleBase* base_type = (ParticleBase*)ptr;

		ImGui::Text(get_class_display_name(base_type->GetTypeID()).c_str());

		switch (base_type->GetTypeID())
		{
		case Reflection::TypeID<RateSpawner>():
		{
			RateSpawner* spawner = (RateSpawner*)base_type;

			EmissionVolume* volume = spawner->GetEmissionVolume();

			std::string volume_name = volume ? get_class_display_name(volume->GetTypeID()) : "None";

			ImGui::Text("Emission Volume: ");
			ImGui::SameLine();
			if (ImGui::Button(volume_name.c_str()))
			{
				ImGui::OpenPopup("Emission Volume Popup");
			}

			if (ImGui::BeginPopup("Emission Volume Popup"))
			{

				if (ImGui::MenuItem("Point"))
				{
					if (volume)
					{
						delete volume;//TODO: Use custom allocator
					}

					PointVolume* new_volume = new PointVolume;
					spawner->SetEmissionVolume(new_volume);
				}
				if (ImGui::MenuItem("Circle"))
				{
					if (volume)
					{
						delete volume;//TODO: Use custom allocator
					}

					CircleVolume* new_volume = new CircleVolume;
					spawner->SetEmissionVolume(new_volume);
				}
				if (ImGui::MenuItem("Sphere"))
				{
					if (volume)
					{
						delete volume;//TODO: Use custom allocator
					}

					SphereVolume* new_volume = new SphereVolume;
					spawner->SetEmissionVolume(new_volume);
				}
				if (ImGui::MenuItem("Box"))
				{
					if (volume)
					{
						delete volume;//TODO: Use custom allocator
					}

					BoxVolume* new_volume = new BoxVolume;
					spawner->SetEmissionVolume(new_volume);
				}

				ImGui::EndPopup();
			}


			if (volume)
			{
				switch (volume->GetTypeID())
				{

				case Reflection::TypeID<PointVolume>():
				{
					PointVolume* point = (PointVolume*)volume;

					break;
				}
				case Reflection::TypeID<SphereVolume>():
				{
					SphereVolume* shape = (SphereVolume*)volume;

					glm::vec3 center = shape->GetCenter();
					if (DrawVec3("Center", center))
					{
						shape->SetCenter(center);
					}

					float radius = shape->GetRadius();
					if (ImGui::DragFloat("Radius", &radius, 0.25f, 0.0f))
					{
						shape->SetRadius(radius);
					}

					break;
				}
				case Reflection::TypeID<BoxVolume>():
				{
					BoxVolume* shape = (BoxVolume*)volume;

					glm::vec3 center = shape->GetCenter();
					if (DrawVec3("Center", center))
					{
						shape->SetCenter(center);
					}

					glm::vec3 extents = shape->GetExtents();
					if (DrawVec3("Extents", extents))
					{
						shape->SetExtents(extents);
					}

					glm::vec3* axis = shape->GetAxis();
					ImGui::Dummy(ImVec2(0.0f, 3.0f));
					ImGui::Text("Axis: ");

					if (DrawVec3("X", axis[0]))
					{
						shape->SetAxis(axis);
					}
					if (DrawVec3("Y", axis[1]))
					{
						shape->SetAxis(axis);
					}
					if (DrawVec3("Z", axis[2]))
					{
						shape->SetAxis(axis);
					}
					
					break;
				}
				case Reflection::TypeID<CircleVolume>():
				{
					CircleVolume* shape = (CircleVolume*)volume;

					glm::vec3 center = shape->GetCenter();
					if (DrawVec3("Center", center))
					{
						shape->SetCenter(center);
					}
					
					glm::vec3 normal = shape->GetNormal();
					if (DrawVec3("Normal", normal))
					{
						shape->SetNormal(normal);
					}

					float radius = shape->GetRadius();
					if (ImGui::DragFloat("Radius", &radius, 0.25f, 0.0f))
					{
						shape->SetRadius(radius);
					}
					break;
				}
				}
			}

			float rate = spawner->GetSpawnRate();
			if (ImGui::DragFloat("Rate", &rate, 0.25f, 0.0f))
			{
				spawner->SetSpawnRate(rate);
			}
			break;
		}
		case Reflection::TypeID<VelocityInitializer>():
		{
			VelocityInitializer* init = (VelocityInitializer*)base_type;

			glm::vec3 min_velocity = init->GetMinVelocity();
			if (DrawVec3("Min Velocity", min_velocity))
			{
				init->SetMinVelocity(min_velocity);
			}

			glm::vec3 max_velocity = init->GetMaxVelocity();
			if (DrawVec3("Max Velocity", max_velocity))
			{
				init->SetMaxVelocity(max_velocity);
			}

			break;
		}
		case Reflection::TypeID<LifetimeInitializer>():
		{
			LifetimeInitializer* init = (LifetimeInitializer*)base_type;

			float min = init->GetMin();
			if (ImGui::DragFloat("Min", &min, 0.05f, 0.0f))
			{
				init->SetMin(min);
			}

			float max = init->GetMax();
			if (ImGui::DragFloat("Max", &max, 0.05f, 0.0f))
			{
				init->SetMax(max);
			}

			break;
		}
		case Reflection::TypeID<CustomParticleInitializer>():
		{
			CustomParticleInitializer* init = (CustomParticleInitializer*)base_type;

			std::string node_name = STRING_FROM_ID(init->GetName());

			if (ImGui::InputText("Name", &node_name) && !node_name.empty())
			{
				init->SetName(STR_ID(node_name));
			}
			

			break;
		}
		case Reflection::TypeID<CustomParticleUpdate>():
		{
			CustomParticleUpdate* update = (CustomParticleUpdate*)base_type;

			std::string node_name = STRING_FROM_ID(update->GetName());

			if (ImGui::InputText("Name", &node_name) && !node_name.empty())
			{
				update->SetName(STR_ID(node_name));
			}
			

			break;
		}
		case Reflection::TypeID<GravityUpdate>():
		{
			GravityUpdate* update = (GravityUpdate*)base_type;

			glm::vec3 gravity = update->GetGravity();
			if (DrawVec3("Gravity", gravity))
			{
				update->SetGravity(gravity);
			}

			break;
		}
		case Reflection::TypeID<DragUpdate>():
		{
			DragUpdate* update = (DragUpdate*)base_type;

			float co_eff = update->GetDragCoefficient();
			if (ImGui::DragFloat("Drag", &co_eff, 0.01f))
			{
				update->SetDragCoefficient(co_eff);
			}
			break;
		}
		case Reflection::TypeID<VelocityUpdate>():
		{
			
			break;
		}
		case Reflection::TypeID<WindUpdate>():
		{
			WindUpdate* update = (WindUpdate*)base_type;

			glm::vec3 force = update->GetWindForce();
			if (DrawVec3("Wind Force", force))
			{
				update->SetWindForce(force);
			}

			float turbulence = update->GetTurbulence();
			if (ImGui::DragFloat("Turbulence", &turbulence, 0.05f))
			{
				update->SetTurbulence(turbulence);
			}

			break;
		}
		case Reflection::TypeID<BillBoardRender>():
		{
			BillBoardRender* render = (BillBoardRender*)base_type;

			std::string tex_name = "None(Texture)";
			Ref<GTexture> tex = render->GetTexture();
			if (tex)
			{
				tex_name = tex->GetName();
			}

			ImGui::Text("Render Texture: ");
			ImGui::SameLine();
			ImGui::Button(tex_name.c_str());

			if (Ref<GTexture> new_tex = ImGuiDragDropTexture())
			{
				render->SetTexture(new_tex);
			}

			TraceEditor* editor = TraceEditor::get_instance();
			static bool image_tex_modified = false;
			if (ImGui::IsItemClicked())
			{
				image_tex_modified = true;
			}
			if (image_tex_modified)
			{
				std::string tex_res;
				if (editor->DrawTexturesPopup(tex_res))
				{
					if (!tex_res.empty())
					{
						std::filesystem::path p = tex_res;
						UUID id = editor->GetContentBrowser()->GetAllFilesID()[p.filename().string()];
						Ref<GTexture> tex_r = LoadTexture(id);
						if (tex_r)
						{
							render->SetTexture(tex_r);
						}
						image_tex_modified = false;
					}
				}
				else image_tex_modified = false;
			}

			bool velocity_align = render->GetVelocityAligned();
			if (ImGui::Checkbox("Velocity Aligned", &velocity_align))
			{
				render->SetVelocityAligned(velocity_align);
			}

			std::string mat_name = "None(Material)";
			if (Ref<MaterialInstance> mat = render->GetMaterial())
			{
				mat_name = mat->GetName();
			}

			ImGui::Text("Material: ");
			ImGui::SameLine();
			ImGui::Button(mat_name.c_str());

			if (Ref<MaterialInstance> new_mat = ImGuiDragDropResource<MaterialInstance>(MATERIAL_FILE_EXTENSION))
			{
				render->SetMaterial(new_mat);
			}

			break;
		}
		}

	}






}