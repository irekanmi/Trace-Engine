
#include "InspectorPanel.h"
#include "scene/Components.h"
#include "backends/Renderutils.h"
#include "backends/UIutils.h"
#include "resource/PrefabManager.h"
#include "serialize/MaterialSerializer.h"
#include "serialize/AnimationsSerializer.h"
#include "scripting/ScriptEngine.h"
#include "../TraceEditor.h"
#include "resource/GenericAssetManager.h"
#include "AnimationPanel.h"
#include "../utils/ImGui_utils.h"
#include "HierachyPanel.h"
#include "ContentBrowser.h"
#include "external_utils.h"
#include "resource/DefaultAssetsManager.h"
#include "AnimationPanel.h"

#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_stdlib.h"
#include "glm/gtc/type_ptr.hpp"

namespace trace {

	InspectorPanel::InspectorPanel()
	{
	}

	template<typename T, typename DrawFunc>
	static bool DrawComponent(Entity entity, const char* placeholder, DrawFunc func)
	{
		bool result = false;
		if (entity.HasComponent<T>())
		{
			T& component = entity.GetComponent<T>();

			float line_height = GetLineHeight();
			ImVec2 content_region = ImGui::GetContentRegionAvail();
			//ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 4.0f, 4.0f });

			ImGuiTreeNodeFlags tree_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth;
			void* id = (void*)((uint64_t)(typeid(T).hash_code() + entity.GetID()));
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
				result = func(entity, component);
				ImGui::TreePop();
			}
			if (deleted) entity.RemoveComponent<T>();

			return result;
		}
		return false;
	}

	

	void DrawMaterial(Ref<MaterialInstance> mat)
	{
		TraceEditor* editor = TraceEditor::get_instance();
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

		for (auto& m_data : mat->GetMaterialData())
		{
			//trace::UniformMetaData& meta_data = mat->GetRenderPipline()->GetSceneUniforms()[m_data.second.hash];
			lambda(m_data.second.type, m_data.second.internal_data, m_data.first);
		}

	}

	void InspectorPanel::DrawEntityComponent(Entity entity, AnimationPanel* animation_panel)
	{
		if (!entity)
		{
			return;
		}

		TraceEditor* editor = TraceEditor::get_instance();
		bool is_prefab = (entity.GetScene() == PrefabManager::get_instance()->GetScene());
		bool recording = animation_panel ? animation_panel->Recording() : false;
		if (recording)
		{
			ImGui::PushStyleColor(ImGuiCol_Border, { 0.79f, 0.12f, 0.15f, 0.35f });
			ImGui::PushStyleColor(ImGuiCol_FrameBg, { 0.79f, 0.12f, 0.15f, 0.25f });
		}
		char anim_data[16] = { 0 };
		AnimationDataType type = AnimationDataType::NONE;
		bool anim_dirty = false;
		bool is_active = entity.HasComponent<ActiveComponent>();
		bool comp_dirty = false;

		if (!is_active)
		{
			ImVec4* colors = ImGui::GetStyle().Colors;
			ImVec4 text_color = colors[ImGuiCol_Text];
			text_color.w = 0.35f;
			ImGui::PushStyleColor(ImGuiCol_Text, text_color);
		}

		if (entity.HasComponent<HierachyComponent>())
		{
			HierachyComponent& val = entity.GetComponent<HierachyComponent>();
			bool enabled = val.is_enabled;
			if (ImGui::Checkbox("##Is Enabled", &enabled))
			{
				if (enabled)
				{
					entity.GetScene()->EnableEntity(entity);
				}
				else
				{
					entity.GetScene()->DisableEntity(entity);
				}
			}
			ImGui::SameLine();
		}
		if (entity.HasComponent<TagComponent>())
		{
			TagComponent& val = entity.GetComponent<TagComponent>();
			std::string temp = val.GetTag();
			if (ImGui::InputText("Tag", &temp))
			{
				if (!temp.empty())
				{
					val.SetTag(std::move(temp));
					comp_dirty = true;
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
				comp_dirty = true;
			}
			if (ImGui::MenuItem("Light"))
			{
				entity.AddComponent<LightComponent>();
				comp_dirty = true;
			}
			if (ImGui::MenuItem("Model"))
			{
				entity.AddComponent<ModelComponent>();
				comp_dirty = true;
			}
			if (ImGui::MenuItem("Model Renderer"))
			{
				ModelRendererComponent& i = entity.AddComponent<ModelRendererComponent>();
				i._material = DefaultAssetsManager::default_material;
				comp_dirty = true;
			}
			if (ImGui::MenuItem("Skinned Model Renderer"))
			{
				SkinnedModelRenderer& i = entity.AddComponent<SkinnedModelRenderer>();
				comp_dirty = true;
			}
			if (ImGui::MenuItem("Text"))
			{
				entity.AddComponent<TextComponent>();
				comp_dirty = true;
			}
			if (ImGui::MenuItem("Rigid Body"))
			{
				entity.AddComponent<RigidBodyComponent>();
				comp_dirty = true;
			}
			if (ImGui::MenuItem("Box Coillder"))
			{
				entity.AddComponent<BoxColliderComponent>();
				comp_dirty = true;
			}
			if (ImGui::MenuItem("Sphere Coillder"))
			{
				entity.AddComponent<SphereColliderComponent>();
				comp_dirty = true;
			}
			if (ImGui::MenuItem("Animation"))
			{
				entity.AddComponent<AnimationComponent>();
				comp_dirty = true;
			}
			if (ImGui::MenuItem("Image"))
			{
				entity.AddComponent<ImageComponent>();
				comp_dirty = true;
			}
			if (ImGui::MenuItem("Sun Light"))
			{
				entity.AddComponent<SunLight>();
				comp_dirty = true;
			}
			if (ImGui::MenuItem("Point Light"))
			{
				entity.AddComponent<PointLight>();
				comp_dirty = true;
			}
			if (ImGui::MenuItem("Spot Light"))
			{
				entity.AddComponent<SpotLight>();
				comp_dirty = true;
			}
			if (ImGui::MenuItem("Animation Graph Controller"))
			{
				entity.AddComponent<AnimationGraphController>();
				comp_dirty = true;
			}
			if (ImGui::MenuItem("Sequence Player"))
			{
				entity.AddComponent<SequencePlayer>();
				comp_dirty = true;
			}
			if (ImGui::MenuItem("Character Controller"))
			{
				entity.AddComponent<CharacterControllerComponent>();
				comp_dirty = true;
			}
			if (ImGui::MenuItem("Motion Matching Component"))
			{
				entity.AddComponent<MotionMatchingComponent>();
				comp_dirty = true;
			}
			if (ImGui::MenuItem("Spring Motion Matching Controller"))
			{
				entity.AddComponent<SpringMotionMatchingController>();
				comp_dirty = true;
			}
			if (ImGui::MenuItem("Net Object"))
			{
				entity.AddComponent<NetObject>();
				comp_dirty = true;
			}
			if (ImGui::MenuItem("Particle Effect Controller"))
			{
				entity.AddComponent<ParticleEffectController>();
				comp_dirty = true;
			}

			for (auto& i : ScriptEngine::get_instance()->GetScripts())
			{
				if (ImGui::MenuItem(i.second.GetScriptName().c_str()))
				{
					entity.AddScript(i.second.GetScriptName());
					comp_dirty = true;
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
					comp_dirty = true;
				}
				if (DrawVec3("Rotation", rotation))
				{
					comp._transform.SetRotationEuler(rotation);
					type = AnimationDataType::ROTATION;
					glm::quat rot = comp._transform.GetRotation();
					memcpy(anim_data, glm::value_ptr(rot), 4 * 4);
					anim_dirty = true;
					comp_dirty = true;
				}
				if (DrawVec3("Scale", scale))
				{
					comp._transform.SetScale(scale);
					type = AnimationDataType::SCALE;
					memcpy(anim_data, glm::value_ptr(scale), 4 * 3);
					anim_dirty = true;
					comp_dirty = true;
				}


				ImGui::TreePop();
			}
		}

		comp_dirty = comp_dirty || DrawComponent<CameraComponent>(entity, "Camera", [](Entity obj, CameraComponent& comp) -> bool {
			bool dirty = false;

			Camera& camera = comp._camera;
			CameraType cam_type = camera.GetCameraType();

			if (ImGui::Checkbox("Is Main", &comp.is_main)) dirty = true;

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
						dirty = true;
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
				{
					camera.SetFov(fov);
					dirty = true;
				}
				
			}
			else if (cam_type == CameraType::ORTHOGRAPHIC)
			{
				float ortho_size = camera.GetOrthographicSize();
				if (ImGui::DragFloat("Orthographic size", &ortho_size, 0.1f))
				{
					camera.SetOrthographicSize(ortho_size);
					dirty = true;
				}
			}

			float _near = camera.GetNear(), _far = camera.GetFar();
			if (ImGui::DragFloat("Near", &_near, 0.1f))
			{
				camera.SetNear(_near);
				dirty = true;
			}
			if (ImGui::DragFloat("Far", &_far, 0.1f))
			{
				camera.SetFar(_far);
				dirty = true;
			}

			return true;
			});

		comp_dirty = comp_dirty || DrawComponent<LightComponent>(entity, "Light", [&](Entity obj, LightComponent& comp) -> bool {

			bool dirty = false;

			Light& light = comp._light;
			LightType light_type = comp.light_type;
			const char* type_string[] = { "Directional", "Point", "Spot"};
			const char* current_type = type_string[(int)light_type];

			IMGUI_WIDGET_MODIFIED_IF(dirty, ImGui::BeginCombo("Light Type", current_type), Light_Type)
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
			IMGUI_WIDGET_MODIFIED_IF(dirty,ImGui::DragFloat("Intensity", &intensity, 0.01f), Intensity)
			{
				light.params2.y = intensity;
				type = AnimationDataType::LIGHT_INTENSITY;
				memcpy(anim_data, &intensity, 4 );
				anim_dirty = true;
			}
			glm::vec4 color = light.color;
			IMGUI_WIDGET_MODIFIED_IF(dirty, ImGui::ColorEdit4("Color", glm::value_ptr(color)), Color)
				light.color = color;

			return dirty;
			});

		/*DrawComponent<MeshComponent>(entity, "Mesh", [](Entity obj, MeshComponent& comp) -> bool {

			Ref<Mesh> mesh = comp._mesh;
			std::string mesh_name = mesh->m_path.string();
			ImVec2 content_ava = ImGui::GetContentRegionAvail();
			float line_height = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
			ImVec2 button_size = { content_ava.x, line_height};
			ImGui::Button(mesh_name.c_str(), button_size);
			});*/

		comp_dirty = comp_dirty || DrawComponent<ModelComponent>(entity, "Model", [&](Entity obj, ModelComponent& comp) -> bool {
			bool dirty = false;

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
			std::string model_res = editor->DrawModelsPopup();
			if (!model_res.empty())
			{
				std::filesystem::path p = model_res;
				Ref<Model> md_res = GenericAssetManager::get_instance()->Get<Model>(p.filename().string());
				// TODO: Implement Model loading
				/*if (!md_res)
				{
					Ref<Mesh> mesh = MeshManager::get_instance()->LoadMeshOnly_(p.parent_path().string());
					md_res = ModelManager::get_instance()->GetModel(p.filename().string());
				}*/
				if (md_res)
				{
					comp._model = md_res;
					dirty = true;
				}
			}
			return dirty;

			});

		comp_dirty = comp_dirty || DrawComponent<ModelRendererComponent>(entity, "Model Renderer", [editor](Entity obj, ModelRendererComponent& comp) -> bool {

			bool dirty = false;

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
					Ref<MaterialInstance> mt_res = GenericAssetManager::get_instance()->Get<MaterialInstance>(p.filename().string());
					if (mt_res);
					else mt_res = MaterialSerializer::Deserialize(p.string());
					if (mt_res)
					{
						comp._material = mt_res;
						dirty = true;
					}
				}
				ImGui::EndDragDropTarget();
			}

			std::string mat_res = editor->DrawMaterialsPopup();
			if (!mat_res.empty())
			{
				std::filesystem::path p = mat_res;
				Ref<MaterialInstance> mt_res = MaterialSerializer::Deserialize(p.string());;
				if (mt_res)
				{
					comp._material = mt_res;
					dirty = true;
				}
			}


			IMGUI_WIDGET_MODIFIED_IF(dirty, ImGui::Checkbox("Cast Shadows", &comp.cast_shadow), Cast_Shadows) {}

			return dirty;
			}
		);
		comp_dirty = comp_dirty || DrawComponent<SkinnedModelRenderer>(entity, "Skinned Model Renderer", [editor](Entity obj, SkinnedModelRenderer& comp) -> bool {

			bool dirty = false;


			Ref<SkinnedModel> model = comp._model;
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
				
			}
			
			std::string mat_name = "None";
			if (comp._material)
			{
				mat_name = comp._material->GetName();
			}

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
					Ref<MaterialInstance> mt_res = MaterialSerializer::Deserialize(p.string());;
					if (mt_res)
					{
						comp._material = mt_res;
						dirty = true;
					}
				}
				ImGui::EndDragDropTarget();
			}

			std::string mat_res = editor->DrawMaterialsPopup();
			if (!mat_res.empty())
			{
				std::filesystem::path p = mat_res;
				Ref<MaterialInstance> mt_res = GenericAssetManager::get_instance()->Get<MaterialInstance>(p.filename().string());
				if (mt_res);
				else mt_res = MaterialSerializer::Deserialize(p.string());
				if (mt_res)
				{
					comp._material = mt_res;
					dirty = true;
				}
			}


			IMGUI_WIDGET_MODIFIED_IF(dirty, ImGui::Checkbox("Cast Shadows", &comp.cast_shadow), Cast_Shadows) {}

			std::string skeleton_name = "None";
			if (comp.GetSkeleton())
			{
				skeleton_name = comp.GetSkeleton()->GetName();
			}

			ImGui::Text("Skeletion :");
			ImGui::SameLine();
			ImGui::Button(skeleton_name.c_str(), button_size);

			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(".trcsk"))
				{
					static char buf[1024] = { 0 };
					memcpy_s(buf, 1024, payload->Data, payload->DataSize);
					std::filesystem::path p = buf;
					Ref<Animation::Skeleton> _res = AnimationsSerializer::DeserializeSkeleton(p.filename().string());
					if (_res)
					{
						comp.SetSkeleton(_res, obj.GetScene(), obj.GetID());
						dirty = true;
					}
				}
				ImGui::EndDragDropTarget();
			}

			return dirty;
			}
		);

		comp_dirty = comp_dirty || DrawComponent<TextComponent>(entity, "Text", [&](Entity obj, TextComponent& comp) -> bool {
			bool dirty = false;

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
					Ref<Font> ft = GenericAssetManager::get_instance()->Get<Font>(p.filename().string());
					if (ft);
					else ft = GenericAssetManager::get_instance()->CreateAssetHandle_<Font>(p.string(), p.string());
					if (ft)
					{
						comp.font = ft;
						dirty = false;
					}
				}
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(".TTF"))
				{
					static char buf[1024] = { 0 };
					memcpy_s(buf, 1024, payload->Data, payload->DataSize);
					std::filesystem::path p = buf;
					Ref<Font> ft = GenericAssetManager::get_instance()->Get<Font>(p.filename().string());
					if (ft);
					else ft = GenericAssetManager::get_instance()->CreateAssetHandle_<Font>(p.string(), p.string());
					if (ft)
					{
						comp.font = ft;
						dirty = false;
					}
				}
				ImGui::EndDragDropTarget();
			}

			ImGui::Text("Enter Text: ");
			IMGUI_WIDGET_MODIFIED_IF(dirty, ImGui::InputTextMultiline("##Text Data", &comp.text), Text_Data) {}

			IMGUI_WIDGET_MODIFIED_IF(dirty, ImGui::ColorEdit3("Color", glm::value_ptr(comp.color)), Color) 
			{}
			IMGUI_WIDGET_MODIFIED_IF(dirty, ImGui::DragFloat("Intensity", &comp.intensity), Intensity)
			{
				type = AnimationDataType::TEXT_INTENSITY;
				memcpy(anim_data, &comp.intensity, sizeof(float));
				anim_dirty = true;
			}
			
			return dirty;
			});


		comp_dirty = comp_dirty || DrawComponent<RigidBodyComponent>(entity, "Rigid Body", [](Entity obj, RigidBodyComponent& comp) -> bool {

			bool dirty = false;
			RigidBody& body = comp.body;
			RigidBodyType type = body.GetType();

			const char* type_string[] = { "Static", "Kinematic", "Dynamic" };
			const char* current_type = type_string[(int)type];
			if (ImGui::BeginCombo("Body Type", current_type))
			{
				for (int i = 0; i < 3; i++)
				{
					bool selected = (current_type == type_string[i]);
					if (ImGui::Selectable(type_string[i], selected))
					{
						body.SetType((RigidBodyType)i);
						dirty = true;
					}

					if (selected)
					{
						ImGui::SetItemDefaultFocus();
					}
				}

				ImGui::EndCombo();
			}

			IMGUI_WIDGET_MODIFIED_IF(dirty, ImGui::DragFloat("Mass", &body.mass), Mass)
			{}
			IMGUI_WIDGET_MODIFIED_IF(dirty, ImGui::DragFloat("Density", &body.density), Density)
			{}

			return dirty;
			});

		comp_dirty = comp_dirty || DrawComponent<BoxColliderComponent>(entity, "Box Coillder", [](Entity obj, BoxColliderComponent& comp) -> bool {
			bool dirty = false;

			PhyShape& shp = comp.shape;

			IMGUI_WIDGET_MODIFIED_IF(dirty, ImGui::Checkbox("Is Trigger", &comp.is_trigger), Is_Trigger)
			{}
			IMGUI_WIDGET_MODIFIED_IF(dirty, ImGui::DragFloat3("Extent", glm::value_ptr(shp.box.half_extents)), Extent)
			{}
			IMGUI_WIDGET_MODIFIED_IF(dirty, ImGui::DragFloat3("Offset", glm::value_ptr(shp.offset)), Offset)
			{}
			
			return dirty;
			});

		comp_dirty = comp_dirty || DrawComponent<SphereColliderComponent>(entity, "Sphere Coillder", [&](Entity obj, SphereColliderComponent& comp) -> bool {

			static bool show_collider = true;

			bool dirty = false;

			ImGui::Checkbox("Show Collider", &show_collider);

			PhyShape& shp = comp.shape;

			IMGUI_WIDGET_MODIFIED_IF(dirty, ImGui::Checkbox("Is Trigger", &comp.is_trigger), Is_Trigger)
			{}
			IMGUI_WIDGET_MODIFIED_IF(dirty, ImGui::DragFloat("Radius", &shp.sphere.radius), Radius)
			{}
			IMGUI_WIDGET_MODIFIED_IF(dirty, ImGui::DragFloat3("Offset", glm::value_ptr(shp.offset)), Offset)
			{}

			

			return dirty;
			});

		comp_dirty = comp_dirty || DrawComponent<AnimationComponent>(entity, "Animation", [&](Entity obj, AnimationComponent& comp) -> bool {

			bool dirty = false;
			Ref<AnimationClip> clip = comp.animation;
			std::string clip_name = "None (Animation Clip)";
			if (clip)
			{
				clip_name = clip->GetName();
				
			}

			ImVec2 content_ava = ImGui::GetContentRegionAvail();
			float line_height = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
			ImVec2 button_size = { content_ava.x, line_height };
			ImGui::Text("Animation Clip: ");
			ImGui::SameLine();
			ImGui::Button(clip_name.c_str(), button_size);			

			clip = ImGuiDragDropResource<AnimationClip>(".trcac");
			if (clip)
			{
				comp.entities.clear();
				comp.animation = clip;
				comp.InitializeEntities(entity.GetScene());
			}

			IMGUI_WIDGET_MODIFIED_IF(dirty, ImGui::Checkbox("Play On Start", &comp.play_on_start), Play_On_Start)
			{}

			IMGUI_WIDGET_MODIFIED_IF(dirty, ImGui::Checkbox("Loop", &comp.loop), Loop)
			{}

			return dirty;
			});

		comp_dirty = comp_dirty || DrawComponent<ImageComponent>(entity, "Image Compoent", [&](Entity obj, ImageComponent& comp) -> bool {
			bool dirty = false;

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

			ImGui::PushID((uint32_t)entity);
			if (ImGui::BeginDragDropTarget())
			{
				static char _buf[1024] = { 0 };
				static auto load_texure = [&dirty, &entity](char* buf)
				{
					ImageComponent& comp = entity.GetComponent<ImageComponent>();
					std::filesystem::path p = buf;
					Ref<GTexture> tex = GenericAssetManager::get_instance()->TryGet<GTexture>(p.filename().string());
					if (tex) {}
					else tex = GenericAssetManager::get_instance()->CreateAssetHandle_<GTexture>(p.string(), p.string());

					if (tex)
					{
						comp.image = tex;
						dirty = true;
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
			ImGui::PopID();
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
							comp.image = tex_r;
							dirty = true;
						}
						image_tex_modified = false;
					}
				}
				else image_tex_modified = false;
			}

			if (comp.image)
			{
				void* a = nullptr;
				UIFunc::GetDrawTextureHandle(comp.image.get(), a);
				ImGui::Image(a, ImVec2(128.0f, 128.0f), { 0.0f, 1.0f }, { 1.0f, 0.0f });
			}

			ImVec4 color = ImGui::ColorConvertU32ToFloat4(comp.color);
			IMGUI_WIDGET_MODIFIED_IF(dirty, ImGui::ColorEdit4("Base Color", &color.x, ImGuiColorEditFlags_Uint8), BaseColor)
			{
				comp.color = (uint32_t)ImGui::ColorConvertFloat4ToU32(color);

				type = AnimationDataType::IMAGE_COLOR;
				memcpy(anim_data, &comp.color, sizeof(uint32_t));
				anim_dirty = true;
			}

			return dirty;
			});

		comp_dirty = comp_dirty || DrawComponent<SunLight>(entity, "Sun Light", [&](Entity obj, SunLight& comp) -> bool {
			bool dirty = false;

			ImVec2 content_ava = ImGui::GetContentRegionAvail();
			float line_height = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
			ImVec2 button_size = { content_ava.x, line_height };



			IMGUI_WIDGET_MODIFIED_IF(dirty, ImGui::ColorEdit3("Color", glm::value_ptr(comp.color)), Light_Color) {}

			IMGUI_WIDGET_MODIFIED_IF(dirty, ImGui::DragFloat("Intensity", &comp.intensity), Light_Intensity) {}

			IMGUI_WIDGET_MODIFIED_IF(dirty, ImGui::Checkbox("Cast Shadows", &comp.cast_shadows), Cast_Shadows) {}

			return dirty;
			});

		comp_dirty = comp_dirty || DrawComponent<PointLight>(entity, "Point Light", [&](Entity obj, PointLight& comp) -> bool {
			bool dirty = false;

			ImVec2 content_ava = ImGui::GetContentRegionAvail();
			float line_height = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
			ImVec2 button_size = { content_ava.x, line_height };



			IMGUI_WIDGET_MODIFIED_IF(dirty, ImGui::ColorEdit3("Color", glm::value_ptr(comp.color)), Light_Color) {}

			IMGUI_WIDGET_MODIFIED_IF(dirty, ImGui::DragFloat("Intensity", &comp.intensity), Light_Intensity) {}

			IMGUI_WIDGET_MODIFIED_IF(dirty, ImGui::DragFloat("Radius", &comp.radius), Light_Radius) {}

			IMGUI_WIDGET_MODIFIED_IF(dirty, ImGui::DragFloat("Constant", &comp.constant, 0.005f), Light_Constant) {}
			IMGUI_WIDGET_MODIFIED_IF(dirty, ImGui::DragFloat("Linear", &comp.linear, 0.002f), Light_Linear) {}
			IMGUI_WIDGET_MODIFIED_IF(dirty, ImGui::DragFloat("Quadratic", &comp.quadratic, 0.002f), Light_Quadratic) {}

			IMGUI_WIDGET_MODIFIED_IF(dirty, ImGui::Checkbox("Cast Shadows", &comp.cast_shadows), Cast_Shadows) {}


			return dirty;
			});

		comp_dirty = comp_dirty || DrawComponent<SpotLight>(entity, "Spot Light", [&](Entity obj, SpotLight& comp) -> bool {
			bool dirty = false;

			ImVec2 content_ava = ImGui::GetContentRegionAvail();
			float line_height = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
			ImVec2 button_size = { content_ava.x, line_height };



			IMGUI_WIDGET_MODIFIED_IF(dirty, ImGui::ColorEdit3("Color", glm::value_ptr(comp.color)), Light_Color) {}

			IMGUI_WIDGET_MODIFIED_IF(dirty, ImGui::DragFloat("Intensity", &comp.intensity), Light_Intensity) {}

			float inner_cutoff = glm::degrees(glm::acos(comp.innerCutOff));
			float outer_cutoff = glm::degrees(glm::acos(comp.outerCutOff));
			
			IMGUI_WIDGET_MODIFIED_IF(dirty, ImGui::DragFloat("Inner CutOff", &inner_cutoff), Light_Inner_CutOff) 
			{
				comp.innerCutOff = glm::cos(glm::radians(inner_cutoff));
			}
			IMGUI_WIDGET_MODIFIED_IF(dirty, ImGui::DragFloat("Outer CutOff", &outer_cutoff), Light_Outer_CutOff)
			{
				comp.outerCutOff = glm::cos(glm::radians(outer_cutoff));
			}

			IMGUI_WIDGET_MODIFIED_IF(dirty, ImGui::Checkbox("Cast Shadows", &comp.cast_shadows), Cast_Shadows) {}


			return dirty;
			});

		comp_dirty = comp_dirty || DrawComponent<AnimationGraphController>(entity, "Animation Graph Controller", [&](Entity obj, AnimationGraphController& comp) -> bool {

			static bool show_collider = true;

			bool dirty = false;

			IMGUI_WIDGET_MODIFIED_IF(dirty, ImGui::Checkbox("Play On Start", &comp.play_on_start), Play_On_Start)
			{}

			Ref<Animation::Graph> graph = comp.graph.GetGraph();
			std::string name = "None (Animation Graph)";
			if (graph)
			{
				name = graph->GetName();
			}

			ImGui::Text("Anim Graph: ");
			ImGui::SameLine();
			IMGUI_WIDGET_MODIFIED_IF(dirty, ImGui::Button(name.c_str()), Resource)
			{}
			graph = ImGuiDragDropResource<Animation::Graph>(ANIMATION_GRAPH_FILE_EXTENSION);
			if (graph)
			{
				comp.graph.DestroyInstance();
				comp.graph.CreateInstance(graph, entity.GetScene(), entity.GetID());
			}

			return dirty;
			});

		comp_dirty = comp_dirty || DrawComponent<SequencePlayer>(entity, "Sequence Player", [&](Entity obj, SequencePlayer& comp) -> bool {

			static bool show_collider = true;

			bool dirty = false;

			IMGUI_WIDGET_MODIFIED_IF(dirty, ImGui::Checkbox("Play On Start", &comp.play_on_start), Play_On_Start)
			{}

			Ref<Animation::Sequence> sequence = comp.sequence.GetSequence();
			std::string name = "None (Animation Sequence)";
			if (sequence)
			{
				name = sequence->GetName();
			}
			ImGui::Text("Animation Sequence: ");
			ImGui::SameLine();
			IMGUI_WIDGET_MODIFIED_IF(dirty, ImGui::Button(name.c_str()), Resource)
			{}
			sequence = ImGuiDragDropResource<Animation::Sequence>(".trcsq");
			if (sequence)
			{
				comp.sequence.DestroyInstance();
				comp.sequence.CreateInstance(sequence, entity.GetScene());
			}

			return dirty;
			});

		comp_dirty = comp_dirty || DrawComponent<CharacterControllerComponent>(entity, "Character Controller", [](Entity obj, CharacterControllerComponent& comp) -> bool {

			bool dirty = false;


			IMGUI_WIDGET_MODIFIED_IF(dirty, ImGui::DragFloat("Contact Offset", &comp.character.contact_offset, 0.005f), ContactOffset)
			{}

			IMGUI_WIDGET_MODIFIED_IF(dirty, ImGui::DragFloat("Height", &comp.character.height, 0.005f), Height)
			{}

			IMGUI_WIDGET_MODIFIED_IF(dirty, ImGui::DragFloat("Radius", &comp.character.radius, 0.005f), Radius)
			{}

			IMGUI_WIDGET_MODIFIED_IF(dirty, ImGui::DragFloat("Min Move Distance", &comp.character.min_move_distance, 0.005f), MinMoveDistance)
			{}

			IMGUI_WIDGET_MODIFIED_IF(dirty, ImGui::DragFloat("Slope Limit", &comp.character.slope_limit, 0.005f), SlopeLimit)
			{}

			IMGUI_WIDGET_MODIFIED_IF(dirty, ImGui::DragFloat("Step Offset", &comp.character.step_offset, 0.005f), StepOffset)
			{}

			IMGUI_WIDGET_MODIFIED_IF(dirty, DrawVec3("Offset", comp.character.offset), Offset)
			{}


			return dirty;
			});

		comp_dirty = comp_dirty || DrawComponent<MotionMatchingComponent>(entity, "Motion Matching", [&](Entity obj, MotionMatchingComponent& comp) -> bool {

			static bool show_collider = true;

			bool dirty = false;

			IMGUI_WIDGET_MODIFIED_IF(dirty, ImGui::DragFloat("Update Frequency", &comp.update_frequency, 0.0003f, 0.0001f, 0.75f, "%.5f"), UpdateFrequency)
			{}

			IMGUI_WIDGET_MODIFIED_IF(dirty, ImGui::DragFloat("Trajectory Weight", &comp.trajectory_weight, 0.005f, 0.0f, 1.0f, "%.5f"), TrajectoryWeight)
			{}

			IMGUI_WIDGET_MODIFIED_IF(dirty, ImGui::DragFloat("Pose Weight", &comp.pose_weight, 0.005f, 0.0f, 1.0f, "%.5f"), PoseWeight)
			{}

			Ref<MotionMatching::MotionMatchingInfo> mmt_info = comp.motion_matching_info;
			std::string name = "None (Motion Matching Info)";
			if (mmt_info)
			{
				name = mmt_info->GetName();
			}

			ImGui::Text("MMT Info: ");
			ImGui::SameLine();
			IMGUI_WIDGET_MODIFIED_IF(dirty, ImGui::Button(name.c_str()), Resource)
			{}
			mmt_info = ImGuiDragDropResource<MotionMatching::MotionMatchingInfo>(MMT_INFO_FILE_EXTENSION);
			if (mmt_info)
			{
				comp.motion_matching_info = mmt_info;
			}

			return dirty;
			});
		
		comp_dirty = comp_dirty || DrawComponent<SpringMotionMatchingController>(entity, "Spring Motion Matching Controller", [&](Entity obj, SpringMotionMatchingController& comp) -> bool {


			bool dirty = false;


			IMGUI_WIDGET_MODIFIED_IF(dirty, ImGui::DragFloat("Position Halflife", &comp.position_halflife, 0.005f, 0.0f, 1.0f, "%.5f"), PositionHalflife)
			{}

			IMGUI_WIDGET_MODIFIED_IF(dirty, ImGui::DragFloat("Rotation Halflife", &comp.rotation_halflife, 0.005f, 0.0f, 1.0f, "%.5f"), RotationHalflife)
			{}
			return dirty;
			});

		comp_dirty = comp_dirty || DrawComponent<NetObject>(entity, "Net Object", [](Entity obj, NetObject& comp) -> bool {
			bool dirty = false;

			
			const char* type_string[] = { "Unknown", "Server", "Client", "Both"};
			const char* current_type = type_string[(int)comp.type];
			if (ImGui::BeginCombo("Object Type", current_type))
			{
				for (int i = 0; i < 4; i++)
				{
					bool selected = (current_type == type_string[i]);
					if (ImGui::Selectable(type_string[i], selected))
					{
						comp.type = (Network::NetObjectType)i;
						dirty = true;
					}

					if (selected)
					{
						ImGui::SetItemDefaultFocus();
					}
				}

				ImGui::EndCombo();
			}

			return true;
			});

		comp_dirty = comp_dirty || DrawComponent<ParticleEffectController>(entity, "Particle Effect Controller", [&](Entity obj, ParticleEffectController& comp) -> bool {


			bool dirty = false;

			IMGUI_WIDGET_MODIFIED_IF(dirty, ImGui::Checkbox("Start On Create", &comp.start_on_create), Start_On_Create)
			{}

			Ref<ParticleEffect> particle_effect = comp.particle_effect.GetParticleEffect();
			std::string name = "None (Particle Effect)";
			if (particle_effect)
			{
				name = particle_effect->GetName();
			}

			ImGui::Text("Particle Effect: ");
			ImGui::SameLine();
			IMGUI_WIDGET_MODIFIED_IF(dirty, ImGui::Button(name.c_str()), Resource)
			{}
			particle_effect = ImGuiDragDropResource<ParticleEffect>(PARTICLE_EFFECT_FILE_EXTENSION);
			if (particle_effect)
			{
				comp.particle_effect.CreateInstance(particle_effect, entity.GetID(), entity.GetScene());
			}

			return dirty;
			});

		ScriptRegistry& script_registry = entity.GetScene()->m_scriptRegistry;

		if (recording && anim_dirty)
		{
			animation_panel->SetFrameData(entity.GetScene(), entity.GetID(), type, anim_data, 16);
		}

		entity.GetScene()->GetScriptRegistry().Iterate(entity.GetID(), [&](UUID uuid, Script* script, ScriptInstance* instance)
			{

				float line_height = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
				ImVec2 content_region = ImGui::GetContentRegionAvail();
				//ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 4.0f, 4.0f });

				ImGuiTreeNodeFlags tree_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth;
				void* id = (void*)(uint64_t)(script->GetID() + (uint32_t)entity);
				ImGui::Separator();
				bool opened = ImGui::TreeNodeEx(id, tree_flags, script->GetScriptName().c_str());
				//ImGui::PopStyleVar();
				ImGui::SameLine(content_region.x - line_height * 0.5f);
				ImVec2 button_size = { line_height + 3.0f , line_height };
				if (ImGui::Button("!", button_size))
				{
					ImGui::OpenPopup(script->GetScriptName().c_str());
				}

				bool deleted = false;
				if (ImGui::BeginPopup(script->GetScriptName().c_str()))
				{
					if (ImGui::MenuItem("Remove Script")) { deleted = true; }
					ImGui::EndPopup();
				}

				if (opened)
				{
					bool isPlaying = editor->GetEditorState() == EditorState::ScenePlay;
					if (isPlaying && !is_prefab)
					{
						for (auto& [name, field] : script->GetFields())
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
							case ScriptFieldType::Action:
							{
								UUID data;
								instance->GetFieldValue(name, data);
								std::string entity_name = "None(Entity)";
								if (data != 0)
								{
									Entity data_result = entity.GetScene()->GetEntity(data);
									if (data_result)
									{
										entity_name = data_result.GetComponent<TagComponent>().GetTag();
									}
								}

								ImGui::Text("%s: ", name.c_str());
								ImGui::SameLine();
								ImGui::Button(entity_name.c_str());
								break;
							}
							case ScriptFieldType::Prefab:
							{
								UUID data;
								instance->GetFieldValue(name, data);
								std::string asset_name = "None(Prefab)";
								if (data != 0)
								{
									std::string filename = GetNameFromUUID(data);
									asset_name = filename.empty() ? asset_name : filename;
								}

								ImGui::Text("%s: ", name.c_str());
								ImGui::SameLine();
								ImGui::Button(asset_name.c_str());
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
						for (auto& i : ins.GetFields())
						{
							const std::string& name = i.first;
							ScriptData& data = i.second;
							switch (data.type)
							{
							case ScriptFieldType::String:
							{
								break;
							}
							case ScriptFieldType::Bool:
							{
								IMGUI_WIDGET_MODIFIED_IF(comp_dirty, ImGui::Checkbox(name.c_str(), (bool*)data.data), Bool)
								{}
								break;
							}
							case ScriptFieldType::Byte:
							{
								IMGUI_WIDGET_MODIFIED_IF(comp_dirty, ImGui::DragScalar(name.c_str(), ImGuiDataType_U8, data.data), Byte)
								{}
								break;
							}
							case ScriptFieldType::Double:
							{
								IMGUI_WIDGET_MODIFIED_IF(comp_dirty, ImGui::DragScalar(name.c_str(), ImGuiDataType_Double, data.data), Double)
								{}
								break;
							}
							case ScriptFieldType::Char:
							{
								IMGUI_WIDGET_MODIFIED_IF(comp_dirty, ImGui::DragScalar(name.c_str(), ImGuiDataType_U8, data.data), Char)
								{}
								break;
							}
							case ScriptFieldType::Float:
							{
								float _data = 0.0f;
								memcpy(&_data, data.data, sizeof(float));
								IMGUI_WIDGET_MODIFIED_IF(comp_dirty, ImGui::DragFloat(name.c_str(), &_data), Float)
									memcpy(data.data, &_data, sizeof(float));
								break;
							}
							case ScriptFieldType::Int16:
							{
								IMGUI_WIDGET_MODIFIED_IF(comp_dirty, ImGui::DragScalar(name.c_str(), ImGuiDataType_S16, data.data), Int16)
								{}
								break;
							}
							case ScriptFieldType::Int32:
							{
								IMGUI_WIDGET_MODIFIED_IF(comp_dirty, ImGui::DragScalar(name.c_str(), ImGuiDataType_S32, data.data), Int32)
								{}
								break;
							}
							case ScriptFieldType::Int64:
							{
								IMGUI_WIDGET_MODIFIED_IF(comp_dirty, ImGui::DragScalar(name.c_str(), ImGuiDataType_S64, data.data), Int64)
								{}
								break;
							}
							case ScriptFieldType::UInt16:
							{
								IMGUI_WIDGET_MODIFIED_IF(comp_dirty, ImGui::DragScalar(name.c_str(), ImGuiDataType_U16, data.data), UInt16)
								{}
								break;
							}
							case ScriptFieldType::UInt32:
							{
								IMGUI_WIDGET_MODIFIED_IF(comp_dirty, ImGui::DragScalar(name.c_str(), ImGuiDataType_U32, data.data), UInt32)
								{}
								break;
							}
							case ScriptFieldType::UInt64:
							{
								IMGUI_WIDGET_MODIFIED_IF(comp_dirty, ImGui::DragScalar(name.c_str(), ImGuiDataType_U64, data.data), UInt64)
								{}
								break;
							}
							case ScriptFieldType::Action:
							{
								UUID data = 0;
								memcpy(&data, i.second.data, sizeof(UUID));
								std::string entity_name = "None(Entity)";
								if (data != 0)
								{
									Entity data_result = entity.GetScene()->GetEntity(data);
									if (data_result)
									{
										entity_name = data_result.GetComponent<TagComponent>().GetTag();
									}
								}

								ImGui::Text("%s: ",name.c_str());
								ImGui::SameLine();
								ImGui::Button(entity_name.c_str());
								if (ImGui::BeginDragDropTarget())
								{
									if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Entity"))
									{
										UUID entity_uuid = *(UUID*)payload->Data;
										if (entity.GetScene()->GetEntity(entity_uuid))
										{
											memcpy(i.second.data, &entity_uuid, sizeof(UUID));
										}
									}
									ImGui::EndDragDropTarget();
								}
								break;
							}
							case ScriptFieldType::Prefab:
							{
								UUID data = 0;
								memcpy(&data, i.second.data, sizeof(UUID));
								std::string asset_name = "None(Prefab)";
								if (data != 0)
								{
									std::string filename = GetNameFromUUID(data);
									asset_name = filename.empty() ? asset_name : filename;
								}

								ImGui::Text("%s: ", name.c_str());
								ImGui::SameLine();
								ImGui::Button(asset_name.c_str());
								Ref<Prefab> asset = ImGuiDragDropResource<Prefab>(PREFAB_FILE_EXTENSION);
								if (asset)
								{
									if (data != 0)
									{
										Ref<Resource> prev_asset = GenericAssetManager::get_instance()->Get<Resource>(data);
										if (prev_asset)
										{
											prev_asset->Decrement();
										}
									}
									asset->Increment();
									UUID asset_id = asset->GetUUID();
									memcpy(i.second.data, &asset_id, sizeof(UUID));
								}
								break;
							}
							case ScriptFieldType::Vec2:
							{
								IMGUI_WIDGET_MODIFIED_IF(comp_dirty, ImGui::DragFloat2(name.c_str(), (float*)data.data), Vec2)
								{}
								break;
							}
							case ScriptFieldType::Vec3:
							{
								IMGUI_WIDGET_MODIFIED_IF(comp_dirty, ImGui::DragFloat3(name.c_str(), (float*)data.data), Vec3)
								{}
								break;
							}
							case ScriptFieldType::Vec4:
							{
								IMGUI_WIDGET_MODIFIED_IF(comp_dirty, ImGui::DragFloat4(name.c_str(), (float*)data.data), Vec4)
								{}
								break;
							}
							
							}
						}
						
					}

					
					ImGui::TreePop();
				}
				if (deleted) entity.RemoveScript(script->GetScriptName());


			});

		if (recording)
		{
			ImGui::PopStyleColor(2);
		}

		/*if(comp_dirty && is_prefab)
			{
				Ref<Prefab> prefab = editor->GetHierachyPanel()->GetPrefabEdit();
				entity.GetScene()->ApplyPrefabChanges(prefab);
			}*/

		if (!is_active)
		{
			ImGui::PopStyleColor();
		}
		
	}

	void InspectorPanel::SetDrawCallbackFn(std::function<void()> cb, std::function<void()> on_enter, std::function<void()> on_exit)
	{
		if (m_onExit) m_onExit();
		m_onExit = on_exit;
		m_drawCallback = cb;
		on_enter();
	}

	bool InspectorPanel::DrawEditMaterial(Ref<MaterialInstance> asset, MaterialData& material_data)
	{
		TraceEditor* editor = TraceEditor::get_instance();
		Ref<MaterialInstance> mat = asset;
		static bool tex_modified = false;
		static std::string tex_name;
		bool dirty = false;
		static bool popup = false;


		ImGui::Text("Material : %s", mat->m_path.string().c_str());
		ImGui::Columns(2);
		ImGui::Text("Render Pipeline");
		ImGui::NextColumn();
		Ref<GPipeline> sp = mat->GetRenderPipline();
		std::string name = sp->m_path.string().empty() ? "None(GPipeline)" : sp->m_path.string();
		if (ImGui::Button(name.c_str()))
		{
			popup = true;
		}

		/*if (popup)
		{
			std::string pipe_res;
			if (popup = editor->DrawPipelinesPopup(pipe_res))
			{
				if (!pipe_res.empty())
				{
					std::filesystem::path p = pipe_res;
					Ref<GPipeline> p_res = GenericAssetManager::get_instance()->Get<GPipeline>(p.filename().string());
					if (!p_res) p_res = PipelineSerializer::Deserialize(p.string());

					if (p_res)
					{
						if (mat->RecreateMaterial(p_res))
						{
							m_assetsEdit.editMaterialPipeChanged = true;
						}
						else
						{
							m_assetsEdit.editMaterial = MaterialSerializer::Deserialize(m_assetsEdit.editMaterialPath.string());
							mat = m_assetsEdit.editMaterial;
						}

					}

					popup = false;
				}
			}
		}*/

		ImGui::Columns(1);


		auto lambda = [&](trace::ShaderData type, std::any& dst, const std::string& name)
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
				ImGui::DragFloat(name.c_str(), data, 0.001f);
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
				ImGui::Image(a, ImVec2(128.0f, 128.0f), { 0.0f, 1.0f }, { 1.0f, 0.0f });
				if (ImGui::IsItemClicked())
				{
					tex_modified = true;
					tex_name = name;
				}
				if (ImGui::BeginDragDropTarget())
				{
					static char buf[1024] = { 0 };
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(".png"))
					{
						memcpy_s(buf, 1024, payload->Data, payload->DataSize);
						std::filesystem::path p = buf;
						Ref<GTexture> tex = GenericAssetManager::get_instance()->TryGet<GTexture>(p.filename().string());
						if (tex) {}
						else tex = GenericAssetManager::get_instance()->CreateAssetHandle_<GTexture>(p.string(), p.string());

						if (tex)
						{
							dst = tex;
							dirty = true;
						}
					}
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(".tga"))
					{
						memcpy_s(buf, 1024, payload->Data, payload->DataSize);
						std::filesystem::path p = buf;
						Ref<GTexture> tex = GenericAssetManager::get_instance()->TryGet<GTexture>(p.filename().string());
						if (tex) {}
						else tex = GenericAssetManager::get_instance()->CreateAssetHandle_<GTexture>(p.string(), p.string());

						if (tex)
						{
							dst = tex;
							dirty = true;
						}
					}
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(".jpg"))
					{
						memcpy_s(buf, 1024, payload->Data, payload->DataSize);
						std::filesystem::path p = buf;
						Ref<GTexture> tex = GenericAssetManager::get_instance()->TryGet<GTexture>(p.filename().string());
						if (tex) {}
						else tex = GenericAssetManager::get_instance()->CreateAssetHandle_<GTexture>(p.string(), p.string());

						if (tex)
						{
							dst = tex;
							dirty = true;
						}
					}
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(".jpeg"))
					{
						memcpy_s(buf, 1024, payload->Data, payload->DataSize);
						std::filesystem::path p = buf;
						Ref<GTexture> tex = GenericAssetManager::get_instance()->TryGet<GTexture>(p.filename().string());
						if (tex) {}
						else tex = GenericAssetManager::get_instance()->CreateAssetHandle_<GTexture>(p.string(), p.string());

						if (tex)
						{
							dst = tex;
							dirty = true;
						}
					}
					ImGui::EndDragDropTarget();
				}

				ImGui::Columns(1);
				break;
			}
			case trace::ShaderData::CUSTOM_DATA_VEC2:
			{
				glm::vec2& data = std::any_cast<glm::vec2&>(dst);
				ImGui::DragFloat2(name.c_str(), glm::value_ptr(data), 0.001f);
				dst = data;
				break;
			}
			case trace::ShaderData::CUSTOM_DATA_VEC3:
			{
				glm::vec3& data = std::any_cast<glm::vec3&>(dst);
				ImGui::DragFloat3(name.c_str(), glm::value_ptr(data), 0.001f);
				dst = data;
				break;
			}
			case trace::ShaderData::CUSTOM_DATA_VEC4:
			{
				glm::vec4& data = std::any_cast<glm::vec4&>(dst);
				ImGui::DragFloat4(name.c_str(), glm::value_ptr(data), 0.001f);
				dst = data;
				break;
			}
			}
		};

		for (auto& m_data : mat->GetMaterialData())
		{
			//trace::UniformMetaData& meta_data = mat->GetRenderPipline()->GetSceneUniforms()[m_data.second.hash];
			lambda(m_data.second.type, m_data.second.internal_data, m_data.first);
		}

		// Select Texture
		if (tex_modified)
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
						mat->GetMaterialData()[tex_name].internal_data = tex_r;
						dirty = true;
					}
					tex_modified = false;
				}
			}
			else tex_modified = false;
		}


		return dirty;
	}

}