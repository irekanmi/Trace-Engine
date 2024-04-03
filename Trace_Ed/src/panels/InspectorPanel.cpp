
#include "InspectorPanel.h"
#include "scene/Components.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_stdlib.h"
#include "glm/gtc/type_ptr.hpp"
#include "backends/Renderutils.h"
#include "backends/UIutils.h"
#include "resource/MaterialManager.h"
#include "resource/ModelManager.h"
#include "resource/MeshManager.h"
#include "resource/FontManager.h"
#include "serialize/MaterialSerializer.h"
#include "serialize/AnimationsSerializer.h"
#include "scripting/ScriptEngine.h"
#include "../TraceEditor.h"
#include "resource/AnimationsManager.h"
#include "resource/TextureManager.h"
#include "AnimationPanel.h"
#include "../utils/ImGui_utils.h"


namespace trace {

	InspectorPanel::InspectorPanel()
	{
	}

	template<typename T, typename DrawFunc>
	static void DrawComponent(Entity entity, const char* placeholder, DrawFunc func)
	{
		if (entity.HasComponent<T>())
		{
			T& component = entity.GetComponent<T>();

			float line_height = GetLineHeight();
			ImVec2 content_region = ImGui::GetContentRegionAvail();
			//ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 4.0f, 4.0f });

			ImGuiTreeNodeFlags tree_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth;
			void* id = (void*)(uint64_t)(typeid(T).hash_code() + (uint32_t)entity);
			ImGui::Separator();
			bool opened = ImGui::TreeNodeEx(id, tree_flags, placeholder);
			//ImGui::PopStyleVar();
			ImGui::SameLine(content_region.x - line_height * 0.5f);
			ImVec2 button_size = { line_height + 3.0f , line_height };
			if (ImGui::Button("!", button_size))
			{
				ImGui::OpenPopup(placeholder);
			}

			bool deleted = false;
			if (ImGui::BeginPopup(placeholder))
			{
				if (ImGui::MenuItem("Remove Component")) { deleted = true; }
				ImGui::EndPopup();
			}

			if (opened)
			{
				func(entity, component);
				ImGui::TreePop();
			}
			if (deleted) entity.RemoveComponent<T>();
		}
	}

	

	void DrawMaterial(Ref<MaterialInstance> mat)
	{
		ImGui::Text("Material : %s",mat->m_path.string().c_str());
		ImGui::Columns(2);
		ImGui::Text("Render Pipeline");
		ImGui::NextColumn();
		Ref<GPipeline> sp = mat->GetRenderPipline();
		ImGui::Button(sp->m_path.string().c_str());
		ImGui::Columns(1);


		auto lambda = [&](trace::ShaderData type, std::any& dst,const std::string& name)
            {
			switch (type)
			{
			case trace::ShaderData::CUSTOM_DATA_BOOL:
			{
				bool* data = &std::any_cast<bool&>(dst);
				ImGui::Checkbox(name.c_str(), data);
				dst = *data;
				break;
			}
			case trace::ShaderData::CUSTOM_DATA_FLOAT:
			{
				float* data = &std::any_cast<float&>(dst);
				ImGui::DragFloat(name.c_str(), data);
				dst = *data;
				break;
			}
			case trace::ShaderData::CUSTOM_DATA_INT:
			{
				int* data = &std::any_cast<int&>(dst);
				ImGui::DragInt(name.c_str(), data);
				dst = *data;
				break;
			}
			case trace::ShaderData::CUSTOM_DATA_IVEC2:
			{
				glm::ivec2& data = std::any_cast<glm::ivec2&>(dst);
				ImGui::DragInt2(name.c_str(), glm::value_ptr(data));
				dst = data;
				break;
			}
			case trace::ShaderData::CUSTOM_DATA_IVEC3:
			{
				glm::ivec3& data = std::any_cast<glm::ivec3&>(dst);
				ImGui::DragInt3(name.c_str(), glm::value_ptr(data));
				dst = data;
				break;
			}
			case trace::ShaderData::CUSTOM_DATA_IVEC4:
			{
				glm::ivec4* data = &std::any_cast<glm::ivec4&>(dst);
				ImGui::DragInt4(name.c_str(), (int*)data);
				dst = data;
				break;
			}
			case trace::ShaderData::CUSTOM_DATA_MAT2:
			{
				glm::mat2& data = std::any_cast<glm::mat2&>(dst);
				ImGui::Text(name.c_str());
				ImGui::DragFloat2((name + "row_0").c_str(), glm::value_ptr(data[0]));
				ImGui::DragFloat2((name + "row_1").c_str(), glm::value_ptr(data[1]));
				dst = data;
				break;
				break;
			}
			case trace::ShaderData::CUSTOM_DATA_MAT3:
			{
				glm::mat3& data = std::any_cast<glm::mat3&>(dst);
				ImGui::Text(name.c_str());
				ImGui::DragFloat3((name + "row_0").c_str(), glm::value_ptr(data[0]));
				ImGui::DragFloat3((name + "row_1").c_str(), glm::value_ptr(data[1]));
				ImGui::DragFloat3((name + "row_2").c_str(), glm::value_ptr(data[2]));
				dst = data;
				break;
			}
			case trace::ShaderData::CUSTOM_DATA_MAT4:
			{
				glm::mat4& data = std::any_cast<glm::mat4&>(dst);
				ImGui::Text(name.c_str());
				ImGui::DragFloat4((name + "row_0").c_str(), glm::value_ptr(data[0]));
				ImGui::DragFloat4((name + "row_1").c_str(), glm::value_ptr(data[1]));
				ImGui::DragFloat4((name + "row_2").c_str(), glm::value_ptr(data[2]));
				ImGui::DragFloat4((name + "row_3").c_str(), glm::value_ptr(data[3]));
				dst = data;
				break;
			}
			case trace::ShaderData::CUSTOM_DATA_TEXTURE:
			{
				Ref<GTexture> tex = std::any_cast<Ref<GTexture>>(dst);
				ImGui::Columns(2);
				ImGui::Text(name.c_str());
				ImGui::Text(tex->GetName().c_str());
				ImGui::NextColumn();
				void* a = nullptr;
				UIFunc::GetDrawTextureHandle(tex.get(), a);
				ImGui::ImageButton(a, ImVec2(32.0f, 32.0f));
				ImGui::Columns(1);
				break;
			}
			case trace::ShaderData::CUSTOM_DATA_VEC2:
			{
				glm::vec2& data = std::any_cast<glm::vec2&>(dst);
				ImGui::DragFloat2(name.c_str(), glm::value_ptr(data));
				dst = data;
				break;
			}
			case trace::ShaderData::CUSTOM_DATA_VEC3:
			{
				glm::vec3& data = std::any_cast<glm::vec3&>(dst);
				ImGui::DragFloat3(name.c_str(), glm::value_ptr(data));
				dst = data;
				break;
			}
			case trace::ShaderData::CUSTOM_DATA_VEC4:
			{
				glm::vec4& data = std::any_cast<glm::vec4&>(dst);
				ImGui::DragFloat4(name.c_str(), glm::value_ptr(data));
				dst = data;
				break;
			}
			}
        };

		for (auto& m_data : mat->m_data)
		{
			trace::UniformMetaData& meta_data = mat->m_renderPipeline->Scene_uniforms[m_data.second.second];
			lambda(meta_data.data_type, m_data.second.first, m_data.first);
		}

	}

	void InspectorPanel::DrawEntityComponent(Entity entity)
	{
		bool recording = m_editor->m_animPanel->Recording();
		if (recording)
		{
			ImGui::PushStyleColor(ImGuiCol_Border, { 0.79, 0.12, 0.15, 0.35f });
			ImGui::PushStyleColor(ImGuiCol_FrameBg, { 0.79, 0.12, 0.15, 0.25f });
		}
		char anim_data[16] = { 0 };
		AnimationDataType type = AnimationDataType::NONE;
		bool anim_dirty = false;

		if (entity.HasComponent<TagComponent>())
		{
			TagComponent& val = entity.GetComponent<TagComponent>();
			std::string temp = val._tag;
			if (ImGui::InputText("Tag", &temp))
			{
				if (!temp.empty())
				{
					val._tag = std::move(temp);
				}

			}
		}

		ImGui::SameLine();
		ImGui::PushItemWidth(-1);
		if (ImGui::Button("Add Component"))
		{
			ImGui::OpenPopup("Add Component");
		}
		ImGui::PopItemWidth();
		if (ImGui::BeginPopup("Add Component"))
		{
			if (ImGui::MenuItem("Camera"))
			{
				entity.AddComponent<CameraComponent>();
			}
			if (ImGui::MenuItem("Light"))
			{
				entity.AddComponent<LightComponent>();
			}
			if (ImGui::MenuItem("Model"))
			{
				entity.AddComponent<ModelComponent>();
			}
			if (ImGui::MenuItem("Model Renderer"))
			{
				ModelRendererComponent& i = entity.AddComponent<ModelRendererComponent>();
				i._material = MaterialManager::get_instance()->GetMaterial("default");
			}
			if (ImGui::MenuItem("Text"))
			{
				entity.AddComponent<TextComponent>();
			}
			if (ImGui::MenuItem("Rigid Body"))
			{
				entity.AddComponent<RigidBodyComponent>();
			}
			if (ImGui::MenuItem("Box Coillder"))
			{
				entity.AddComponent<BoxColliderComponent>();
			}
			if (ImGui::MenuItem("Sphere Coillder"))
			{
				entity.AddComponent<SphereColliderComponent>();
			}
			if (ImGui::MenuItem("Animation"))
			{
				entity.AddComponent<AnimationComponent>();
			}
			if (ImGui::MenuItem("Image"))
			{
				entity.AddComponent<ImageComponent>();
			}

			for (auto& i : ScriptEngine::get_instance()->GetScripts())
			{
				if (ImGui::MenuItem(i.second.script_name.c_str()))
				{
					entity.AddScript(i.second.script_name);
				}
			}

			ImGui::EndPopup();
		}
		

		if (entity.HasComponent<TransformComponent>())
		{
			TransformComponent& comp = entity.GetComponent<TransformComponent>();
			ImGuiTreeNodeFlags tree_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth;
			void* id = (void*)(uint64_t)(typeid(TransformComponent).hash_code() + (uint32_t)entity);
			bool opened = ImGui::TreeNodeEx(id, tree_flags, "Transform");

			if (opened)
			{
				glm::vec3 pos = comp._transform.GetPosition();
				glm::vec3 scale = comp._transform.GetScale();
				glm::vec3 rotation = comp._transform.GetRotationEuler();
				if (DrawVec3("Position", pos))
				{
					comp._transform.SetPosition(pos);
					type = AnimationDataType::POSITION;
					memcpy(anim_data, glm::value_ptr(pos), 4 * 3);
					anim_dirty = true;
				}
				if (DrawVec3("Rotation", rotation))
				{
					comp._transform.SetRotationEuler(rotation);
					type = AnimationDataType::ROTATION;
					glm::quat rot = comp._transform.GetRotation();
					memcpy(anim_data, glm::value_ptr(rot), 4 * 4);
					anim_dirty = true;
				}
				if (DrawVec3("Scale", scale))
				{
					comp._transform.SetScale(scale);
					type = AnimationDataType::SCALE;
					memcpy(anim_data, glm::value_ptr(scale), 4 * 3);
					anim_dirty = true;
				}


				ImGui::TreePop();
			}
		}

		DrawComponent<CameraComponent>(entity, "Camera", [](Entity obj, CameraComponent& comp) {

			Camera& camera = comp._camera;
			CameraType cam_type = camera.GetCameraType();

			ImGui::Checkbox("Is Main", &comp.is_main);

			const char* type_string[] = { "Perspective", "Orthographic" };
			const char* current_type = type_string[(int)cam_type];
			if (ImGui::BeginCombo("Camera Type", current_type))
			{
				for (int i = 0; i < 2; i++)
				{
					bool selected = (current_type == type_string[i]);
					if (ImGui::Selectable(type_string[i], selected))
					{
						camera.SetCameraType((CameraType)i);
					}

					if (selected)
						ImGui::SetItemDefaultFocus();
				}

				ImGui::EndCombo();
			}

			if (cam_type == CameraType::PERSPECTIVE)
			{
				float fov = camera.GetFov();
				if (ImGui::DragFloat("Fov", &fov, 0.3f, 0.0f, 180.0f))
					camera.SetFov(fov);
				
			}
			else if (cam_type == CameraType::ORTHOGRAPHIC)
			{
				float ortho_size = camera.GetOrthographicSize();
				if (ImGui::DragFloat("Orthographic size", &ortho_size, 0.1f))
					camera.SetOrthographicSize(ortho_size);
			}

			float _near = camera.GetNear(), _far = camera.GetFar();
			if (ImGui::DragFloat("Near", &_near, 0.1f))
				camera.SetNear(_near);
			if (ImGui::DragFloat("Far", &_far, 0.1f))
				camera.SetFar(_far);


			});

		DrawComponent<LightComponent>(entity, "Light", [&](Entity obj, LightComponent& comp) {

			Light& light = comp._light;
			LightType light_type = comp.light_type;
			const char* type_string[] = { "Directional", "Point", "Spot"};
			const char* current_type = type_string[(int)light_type];
			if (ImGui::BeginCombo("Light Type", current_type))
			{
				for (int i = 0; i < 3; i++)
				{
					bool selected = (current_type == type_string[i]);
					if (ImGui::Selectable(type_string[i], selected))
					{
						comp.light_type = (LightType)i;
					}

					if (selected)
						ImGui::SetItemDefaultFocus();
				}

				ImGui::EndCombo();
			}

			//TODO: Compute Direction from TransformComponent
			if (light_type == LightType::DIRECTIONAL)
			{
				glm::vec3 direction = glm::vec3(light.direction);
				if (DrawVec3("Direction", direction))
					light.direction = glm::vec4(direction, 0.0f);

			}
			else if (light_type == LightType::POINT)
			{

				float constant = light.params1.x;
				if (ImGui::DragFloat("Constant", &constant, 0.001f))
					light.params1.x = constant;

				float linear = light.params1.y;
				if (ImGui::DragFloat("Linear", &linear, 0.001f))
					light.params1.y = linear;

				float quadratic = light.params1.z;
				if (ImGui::DragFloat("Quadratic", &quadratic, 0.001f))
					light.params1.z = quadratic;
			}
			else if (light_type == LightType::SPOT)
			{
				glm::vec3 direction = glm::vec3(light.direction);
				if (DrawVec3("Direction", direction))
					light.direction = glm::vec4(direction, 0.0f);

				ImGui::Text("   ");

				float innear_cutoff = glm::degrees(glm::acos(light.params1.w));
				if (ImGui::DragFloat("Innear Cutoff", &innear_cutoff, 0.2f))
					light.params1.w = glm::cos(glm::radians(innear_cutoff));

				float outer_cutoff = glm::degrees(glm::acos(light.params2.x));
				if (ImGui::DragFloat("Outer Cutoff", &outer_cutoff, 0.2f))
					light.params2.x = glm::cos(glm::radians(outer_cutoff));
			}

			float intensity = light.params2.y;
			if (ImGui::DragFloat("Intensity", &intensity, 0.01f))
			{
				light.params2.y = intensity;
				type = AnimationDataType::LIGHT_INTENSITY;
				memcpy(anim_data, &intensity, 4 );
				anim_dirty = true;
			}
			glm::vec4 color = light.color;
			if (ImGui::ColorEdit4("Color", glm::value_ptr(color)))
				light.color = color;


			});

		DrawComponent<MeshComponent>(entity, "Mesh", [](Entity obj, MeshComponent& comp) {

			Ref<Mesh> mesh = comp._mesh;
			std::string mesh_name = mesh->m_path.string();
			ImVec2 content_ava = ImGui::GetContentRegionAvail();
			float line_height = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
			ImVec2 button_size = { content_ava.x, line_height};
			ImGui::Button(mesh_name.c_str(), button_size);
			});

		DrawComponent<ModelComponent>(entity, "Model", [&](Entity obj, ModelComponent& comp) {

			Ref<Model> model = comp._model;
			std::string model_name = "None (Model)";
			if (model)
			{
				model_name = model->GetName();
			}
			
			ImVec2 content_ava = ImGui::GetContentRegionAvail();
			float line_height = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
			ImVec2 button_size = { content_ava.x, line_height };
			if (ImGui::Button(model_name.c_str(), button_size))
			{
				ImGui::OpenPopup("ALL_MODELS");
			}
			std::string model_res = m_editor->DrawModelsPopup();
			if (!model_res.empty())
			{
				std::filesystem::path p = model_res;
				Ref<Model> md_res = ModelManager::get_instance()->GetModel(p.filename().string());
				if (md_res) comp._model = md_res;
				else
				{
					Ref<Mesh> mesh = MeshManager::get_instance()->LoadMeshOnly_(p.parent_path().string());
					md_res = ModelManager::get_instance()->GetModel(p.filename().string());
					if (md_res) comp._model = md_res;
				}
			}


			});

		DrawComponent<ModelRendererComponent>(entity, "Model Renderer", [&](Entity obj, ModelRendererComponent& comp) {

			std::string mat_name = "None";
			if (comp._material)
			{
				mat_name = comp._material->GetName();
			}
			ImVec2 content_ava = ImGui::GetContentRegionAvail();
			float line_height = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
			ImVec2 button_size = { content_ava.x, line_height };


			ImGui::Text("Material :");
			ImGui::SameLine();
			ImGui::Button(mat_name.c_str(), button_size);

			if (ImGui::IsItemClicked())
			{
				ImGui::OpenPopup("ALL_MATERIALS");
			}

			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(".trmat"))
				{
					static char buf[1024] = { 0 };
					memcpy_s(buf, 1024, payload->Data, payload->DataSize);
					std::filesystem::path p = buf;
					Ref<MaterialInstance> mt_res = MaterialManager::get_instance()->GetMaterial(p.filename().string());
					if (mt_res) comp._material = mt_res;
					mt_res = MaterialSerializer::Deserialize(p.string());
					if (mt_res) comp._material = mt_res;
				}
				ImGui::EndDragDropTarget();
			}

			std::string mat_res = m_editor->DrawMaterialsPopup();
			if (!mat_res.empty())
			{
				std::filesystem::path p = mat_res;
				Ref<MaterialInstance> mt_res = MaterialManager::get_instance()->GetMaterial(p.filename().string());
				if (mt_res) comp._material = mt_res;
				else mt_res = MaterialSerializer::Deserialize(p.string());
				if (mt_res) comp._material = mt_res;
			}


			}
		);

		DrawComponent<TextComponent>(entity, "Text", [&](Entity obj, TextComponent& comp) {

			std::string font_name = "None (FONT)";
			if (comp.font)
			{
				font_name = comp.font->GetName();
			}

			ImVec2 content_ava = ImGui::GetContentRegionAvail();
			float line_height = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
			ImVec2 button_size = { content_ava.x, line_height };

			ImGui::Text("Font: ");
			ImGui::SameLine();
			ImGui::Button(font_name.c_str(), button_size);

			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(".ttf"))
				{
					static char buf[1024] = { 0 };
					memcpy_s(buf, 1024, payload->Data, payload->DataSize);
					std::filesystem::path p = buf;
					Ref<Font> ft = FontManager::get_instance()->GetFont(p.filename().string());
					if (ft) comp.font = ft;
					ft = FontManager::get_instance()->LoadFont_(p.string());
					if (ft) comp.font = ft;
				}
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(".TTF"))
				{
					static char buf[1024] = { 0 };
					memcpy_s(buf, 1024, payload->Data, payload->DataSize);
					std::filesystem::path p = buf;
					Ref<Font> ft = FontManager::get_instance()->GetFont(p.filename().string());
					if (ft) comp.font = ft;
					ft = FontManager::get_instance()->LoadFont_(p.string());
					if (ft) comp.font = ft;
				}
				ImGui::EndDragDropTarget();
			}

			ImGui::Text("Enter Text: ");
			ImGui::InputTextMultiline("##Text Data", &comp.text);

			ImGui::ColorEdit3("Color", glm::value_ptr(comp.color));
			if (ImGui::DragFloat("Intensity", &comp.intensity))
			{
				type = AnimationDataType::TEXT_INTENSITY;
				memcpy(anim_data, &comp.intensity, 4);
				anim_dirty = true;
			}
			
			});


		DrawComponent<RigidBodyComponent>(entity, "Rigid Body", [](Entity obj, RigidBodyComponent& comp) {

			RigidBody& body = comp.body;
			RigidBody::Type type = body.GetType();

			const char* type_string[] = { "Static", "Kinematic", "Dynamic" };
			const char* current_type = type_string[(int)type];
			if (ImGui::BeginCombo("Body Type", current_type))
			{
				for (int i = 0; i < 3; i++)
				{
					bool selected = (current_type == type_string[i]);
					if (ImGui::Selectable(type_string[i], selected))
					{
						body.SetType((RigidBody::Type)i);
					}

					if (selected)
						ImGui::SetItemDefaultFocus();
				}

				ImGui::EndCombo();
			}

			ImGui::DragFloat("Mass", &body.mass);
			ImGui::DragFloat("Density", &body.density);

			});

		DrawComponent<BoxColliderComponent>(entity, "Box Coillder", [](Entity obj, BoxColliderComponent& comp) {
			
			PhyShape& shp = comp.shape;

			ImGui::Checkbox("Is Trigger", &comp.is_trigger);
			ImGui::DragFloat3("Extent", glm::value_ptr(shp.box.half_extents));
			ImGui::DragFloat3("Offset", glm::value_ptr(shp.offset));
			
			});

		DrawComponent<SphereColliderComponent>(entity, "Sphere Coillder", [&](Entity obj, SphereColliderComponent& comp) {

			static bool show_collider = true;
			ImGui::Checkbox("Show Collider", &show_collider);

			PhyShape& shp = comp.shape;

			ImGui::Checkbox("Is Trigger", &comp.is_trigger);
			ImGui::DragFloat("Radius", &shp.sphere.radius);
			ImGui::DragFloat3("Offset", glm::value_ptr(shp.offset));

			if (show_collider)
			{
				Renderer* renderer = Renderer::get_instance();

				Transform pose = obj.GetComponent<TransformComponent>()._transform;
				pose.SetPosition(pose.GetPosition() + shp.offset);

				CommandList cmd_list = renderer->BeginCommandList();
				renderer->BeginScene(cmd_list, &m_editor->editor_cam);
				renderer->DrawDebugSphere(cmd_list, shp.sphere.radius + 0.001f, 25, pose.GetLocalMatrix());
				renderer->EndScene(cmd_list);
				renderer->SubmitCommandList(cmd_list);
			}

			});

		DrawComponent<AnimationComponent>(entity, "Animation", [&](Entity obj, AnimationComponent& comp) {

			Ref<AnimationGraph> graph = comp.anim_graph;
			std::string graph_name = "None (Animation Graph)";
			if (graph)
			{
				graph_name = graph->GetName();
				
			}

			ImVec2 content_ava = ImGui::GetContentRegionAvail();
			float line_height = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
			ImVec2 button_size = { content_ava.x, line_height };
			ImGui::Text("Anim Graph: ");
			ImGui::SameLine();
			ImGui::Button(graph_name.c_str(), button_size);

			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(".trcag"))
				{
					static char buf[1024] = { 0 };
					memcpy_s(buf, 1024, payload->Data, payload->DataSize);
					std::filesystem::path p = buf;
					Ref<AnimationGraph> ag = AnimationsManager::get_instance()->GetGraph(p.filename().string());
					if (ag) comp.anim_graph = ag;
					ag = AnimationsSerializer::DeserializeAnimationGraph(p.string());
					if (ag) comp.anim_graph = ag;
				}
				ImGui::EndDragDropTarget();
			}

			ImGui::Checkbox("Play On Start", &comp.play_on_start);

			});

		DrawComponent<ImageComponent>(entity, "Image Compoent", [&](Entity obj, ImageComponent& comp) {

			Ref<GTexture> img = comp.image;
			std::string image_name = "None (Texture)";
			if (img)
			{
				image_name = img->GetName();

			}

			ImVec2 content_ava = ImGui::GetContentRegionAvail();
			float line_height = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
			ImVec2 button_size = { content_ava.x, line_height };
			ImGui::Text("Image: ");
			ImGui::SameLine();
			ImGui::Button(image_name.c_str(), button_size);

			if (ImGui::BeginDragDropTarget())
			{
				static char _buf[1024] = { 0 };
				static auto load_texure = [&comp](char* buf)
				{
					std::filesystem::path p = buf;
					Ref<GTexture> tex = TextureManager::get_instance()->GetTexture(p.filename().string());
					if (tex) {}
					else tex = TextureManager::get_instance()->LoadTexture_(p.string());

					if (tex)
					{
						comp.image = tex;
					}
				};
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(".png"))
				{
					memcpy_s(_buf, 1024, payload->Data, payload->DataSize);
					load_texure(_buf);					
				}
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(".jpeg"))
				{
					memcpy_s(_buf, 1024, payload->Data, payload->DataSize);
					load_texure(_buf);
				}
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(".jpg"))
				{
					memcpy_s(_buf, 1024, payload->Data, payload->DataSize);
					load_texure(_buf);
				}
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(".tga"))
				{
					memcpy_s(_buf, 1024, payload->Data, payload->DataSize);
					load_texure(_buf);
				}
				ImGui::EndDragDropTarget();
			}

			if (comp.image)
			{
				void* a = nullptr;
				UIFunc::GetDrawTextureHandle(comp.image.get(), a);
				ImGui::Image(a, ImVec2(128.0f, 128.0f), { 0.0f, 1.0f }, { 1.0f, 0.0f });
			}


			});

		ScriptRegistry& script_registry = m_editor->m_currentScene->m_scriptRegistry;

		if (recording && anim_dirty)
		{
			m_editor->m_animPanel->SetFrameData(entity.GetID(), type, anim_data, 16);
		}

		m_editor->m_currentScene->m_scriptRegistry.Iterate(entity.GetID(), [&](UUID uuid, Script* script, ScriptInstance* instance)
			{

				float line_height = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
				ImVec2 content_region = ImGui::GetContentRegionAvail();
				//ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 4.0f, 4.0f });

				ImGuiTreeNodeFlags tree_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth;
				void* id = (void*)(uint64_t)(script->GetID() + (uint32_t)entity);
				ImGui::Separator();
				bool opened = ImGui::TreeNodeEx(id, tree_flags, script->script_name.c_str());
				//ImGui::PopStyleVar();
				ImGui::SameLine(content_region.x - line_height * 0.5f);
				ImVec2 button_size = { line_height + 3.0f , line_height };
				if (ImGui::Button("!", button_size))
				{
					ImGui::OpenPopup(script->script_name.c_str());
				}

				bool deleted = false;
				if (ImGui::BeginPopup(script->script_name.c_str()))
				{
					if (ImGui::MenuItem("Remove Script")) { deleted = true; }
					ImGui::EndPopup();
				}

				if (opened)
				{
					bool isPlaying = m_editor->current_state == EditorState::ScenePlay;
					if (isPlaying)
					{
						for (auto& [name, field] : script->m_fields)
						{
							switch (field.field_type)
							{
							case ScriptFieldType::String:
							{
								break;
							}
							case ScriptFieldType::Bool:
							{
								bool data;
								instance->GetFieldValue(name, data);
								if(ImGui::Checkbox(name.c_str(), &data))
									instance->SetFieldValue(name, data);
								break;
							}
							case ScriptFieldType::Byte:
							{
								std::byte data;
								instance->GetFieldValue(name, data);
								if (ImGui::DragScalar(name.c_str(), ImGuiDataType_U8, &data))
									instance->SetFieldValue(name, data);
								break;
							}
							case ScriptFieldType::Double:
							{
								double data;
								instance->GetFieldValue(name, data);
								if (ImGui::DragScalar(name.c_str(), ImGuiDataType_Double, &data))
									instance->SetFieldValue(name, data);
								break;
							}
							case ScriptFieldType::Char:
							{
								char data;
								instance->GetFieldValue(name, data);
								if (ImGui::DragScalar(name.c_str(), ImGuiDataType_U8, &data))
									instance->SetFieldValue(name, data);
								break;
							}
							case ScriptFieldType::Float:
							{
								float data;
								instance->GetFieldValue(name, data);
								if (ImGui::DragFloat(name.c_str(), &data))
									instance->SetFieldValue(name, data);
								break;
							}
							case ScriptFieldType::Int16:
							{
								int16_t data;
								instance->GetFieldValue(name, data);
								if (ImGui::DragScalar(name.c_str(), ImGuiDataType_S16, &data))
									instance->SetFieldValue(name, data);
								break;
							}
							case ScriptFieldType::Int32:
							{
								int32_t data;
								instance->GetFieldValue(name, data);
								if (ImGui::DragScalar(name.c_str(), ImGuiDataType_S32, &data))
									instance->SetFieldValue(name, data);
								break;
							}
							case ScriptFieldType::Int64:
							{
								int64_t data;
								instance->GetFieldValue(name, data);
								if (ImGui::DragScalar(name.c_str(), ImGuiDataType_S64, &data))
									instance->SetFieldValue(name, data);
								break;
							}
							case ScriptFieldType::UInt16:
							{
								uint16_t data;
								instance->GetFieldValue(name, data);
								if (ImGui::DragScalar(name.c_str(), ImGuiDataType_U16, &data))
									instance->SetFieldValue(name, data);
								break;
							}
							case ScriptFieldType::UInt32:
							{
								uint32_t data;
								instance->GetFieldValue(name, data);
								if (ImGui::DragScalar(name.c_str(), ImGuiDataType_U32, &data))
									instance->SetFieldValue(name, data);
								break;
							}
							case ScriptFieldType::UInt64:
							{
								uint64_t data;
								instance->GetFieldValue(name, data);
								if (ImGui::DragScalar(name.c_str(), ImGuiDataType_U64, &data))
									instance->SetFieldValue(name, data);
								break;
							}
							case ScriptFieldType::Vec2:
							{
								glm::vec2 data;
								instance->GetFieldValue(name, data);
								if (ImGui::DragFloat2(name.c_str(), glm::value_ptr(data)))
									instance->SetFieldValue(name, data);
								break;
							}
							case ScriptFieldType::Vec3:
							{
								glm::vec3 data;
								instance->GetFieldValue(name, data);
								if (ImGui::DragFloat3(name.c_str(), glm::value_ptr(data)))
									instance->SetFieldValue(name, data);
								break;
							}
							case ScriptFieldType::Vec4:
							{
								glm::vec3 data;
								instance->GetFieldValue(name, data);
								if (ImGui::DragFloat3(name.c_str(), glm::value_ptr(data)))
									instance->SetFieldValue(name, data);
								break;
							}

							}
						}
						

					}
					else
					{
						

						//auto& fields_instances = ScriptEngine::get_instance()->GetFieldInstances();
						auto& fields_instances = script_registry.GetFieldInstances();
						auto& field_manager = fields_instances[script];
						auto field_it = field_manager.find(uuid);
						if (field_it == field_manager.end())
						{
							ScriptFieldInstance& field_ins = field_manager[uuid];
							field_ins.Init(script);
						}
						ScriptFieldInstance& ins = field_manager[uuid];
						for (auto& [name, data] : ins.m_fields)
						{
							switch (data.type)
							{
							case ScriptFieldType::String:
							{
								break;
							}
							case ScriptFieldType::Bool:
							{
								ImGui::Checkbox(name.c_str(), (bool*)data.data);
								break;
							}
							case ScriptFieldType::Byte:
							{
								ImGui::DragScalar(name.c_str(), ImGuiDataType_U8, data.data);
								break;
							}
							case ScriptFieldType::Double:
							{
								ImGui::DragScalar(name.c_str(), ImGuiDataType_Double, data.data);
								break;
							}
							case ScriptFieldType::Char:
							{
								ImGui::DragScalar(name.c_str(), ImGuiDataType_U8, data.data);
								break;
							}
							case ScriptFieldType::Float:
							{
								ImGui::DragFloat(name.c_str(), (float*)data.data);
								break;
							}
							case ScriptFieldType::Int16:
							{
								ImGui::DragScalar(name.c_str(), ImGuiDataType_S16, data.data);
								break;
							}
							case ScriptFieldType::Int32:
							{
								ImGui::DragScalar(name.c_str(), ImGuiDataType_S32, data.data);
								break;
							}
							case ScriptFieldType::Int64:
							{
								ImGui::DragScalar(name.c_str(), ImGuiDataType_S64, data.data);
								break;
							}
							case ScriptFieldType::UInt16:
							{
								ImGui::DragScalar(name.c_str(), ImGuiDataType_U16, data.data);
								break;
							}
							case ScriptFieldType::UInt32:
							{
								ImGui::DragScalar(name.c_str(), ImGuiDataType_U32, data.data);
								break;
							}
							case ScriptFieldType::UInt64:
							{
								ImGui::DragScalar(name.c_str(), ImGuiDataType_U64, data.data);
								break;
							}
							case ScriptFieldType::Vec2:
							{
								ImGui::DragFloat2(name.c_str(), (float*)data.data);
								break;
							}
							case ScriptFieldType::Vec3:
							{
								ImGui::DragFloat3(name.c_str(), (float*)data.data);
								break;
							}
							case ScriptFieldType::Vec4:
							{
								ImGui::DragFloat4(name.c_str(), (float*)data.data);
								break;
							}
							
							}
						}
						
					}

					
					ImGui::TreePop();
				}
				if (deleted) entity.RemoveScript(script->script_name);


			});

			if (recording)
			{
				ImGui::PopStyleColor(2);
			}
		
	}

	void InspectorPanel::SetDrawCallbackFn(std::function<void()> cb, std::function<void()> on_enter, std::function<void()> on_exit)
	{
		if (m_onExit) m_onExit();
		m_onExit = on_exit;
		m_drawCallback = cb;
		on_enter();
	}

}