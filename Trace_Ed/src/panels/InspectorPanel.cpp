
#include "InspectorPanel.h"
#include "scene/Componets.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_stdlib.h"
#include "glm/gtc/type_ptr.hpp"
#include "render/Renderutils.h"
#include "backends/UIutils.h"
#include "resource/MaterialManager.h"


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

			float line_height = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
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

	bool DrawVec3(const char* label, glm::vec3& data, float column_width = 100.0f)
	{
		bool modified = false;
		ImGui::PushID(label);

		ImGui::Columns(2);

		ImGui::SetColumnWidth(0, column_width);
		ImGui::Text(label);
		ImGui::NextColumn();

		ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0.0f, 0.0f });

		float line_height = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 button_size = { line_height + 3.0f, line_height };

		ImGui::PushStyleColor(ImGuiCol_Button, { 0.8f, 0.15f, 0.15f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0.9f, 0.2f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, { 0.8f, 0.15f, 0.15f, 1.0f });
		ImGui::Button("X", button_size);
		ImGui::SameLine();
		if (ImGui::DragFloat("##X", &data.x, 0.1f)) modified = true;
		ImGui::PopItemWidth();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, { 0.15f, 0.8f, 0.15f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0.2f, 0.9f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, { 0.15f, 0.8f, 0.15f, 1.0f });
		ImGui::Button("Y", button_size);
		ImGui::SameLine();
		if (ImGui::DragFloat("##Y", &data.y, 0.1f)) modified = true;
		ImGui::PopItemWidth();
		ImGui::PopStyleColor(3);


		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, { 0.15f, 0.15f, 0.8f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0.2f, 0.2f, 0.9f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, { 0.15f, 0.15f, 0.8f, 1.0f });
		ImGui::Button("Z", button_size);
		ImGui::SameLine();
		if (ImGui::DragFloat("##Z", &data.z, 0.1f)) modified = true;
		ImGui::PopItemWidth();
		ImGui::PopStyleColor(3);


		ImGui::PopStyleVar();
		ImGui::Columns(1);

		ImGui::PopID();

		return modified;
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

		
		ImGui::ColorEdit4("Diffuse color", glm::value_ptr(mat->m_material.m_diffuseColor));
		ImGui::DragFloat("Shineness", &mat->m_material.m_shininess);

		Ref<GTexture> tex;
		ImGui::Columns(2);
		ImGui::Text("Albedo Map");
		tex = mat->m_material.m_albedoMap;
		ImGui::Text(tex->GetName().c_str());
		ImGui::NextColumn();
		void* a = nullptr;
		UIFunc::GetDrawTextureHandle(tex.get(), a);
		ImGui::ImageButton(a, ImVec2(32.0f, 32.0f));
		ImGui::Columns(1);

		ImGui::Columns(2);
		ImGui::Text("Specular Map");
		tex = mat->m_material.m_specularMap;
		ImGui::Text(tex->GetName().c_str());
		ImGui::NextColumn();
		void* s = nullptr;
		UIFunc::GetDrawTextureHandle(tex.get(), s);
		ImGui::ImageButton(s, ImVec2(32.0f, 32.0f));
		ImGui::Columns(1);

		ImGui::Columns(2);
		ImGui::Text("Normal Map");
		tex = mat->m_material.m_normalMap;
		ImGui::Text(tex->GetName().c_str());
		ImGui::NextColumn();
		void* n = nullptr;
		UIFunc::GetDrawTextureHandle(tex.get(), n);
		ImGui::ImageButton(n, ImVec2(32.0f, 32.0f));
		ImGui::Columns(1);

	}

	void InspectorPanel::DrawEntityComponent(Entity entity)
	{
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
					comp._transform.SetPosition(pos);
				if (DrawVec3("Rotation", rotation))
					comp._transform.SetRotationEuler(rotation);
				if (DrawVec3("Scale", scale))
					comp._transform.SetScale(scale);


				ImGui::TreePop();
			}
		}

		DrawComponent<CameraComponent>(entity, "Camera", [](Entity obj, CameraComponent& comp) {

			Camera& camera = comp._camera;
			CameraType cam_type = camera.GetCameraType();
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

		DrawComponent<LightComponent>(entity, "Light", [](Entity obj, LightComponent& comp) {

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
				light.params2.y = intensity;
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

		DrawComponent<ModelComponent>(entity, "Model", [](Entity obj, ModelComponent& comp) {

			Ref<Model> model = comp._model;
			std::string model_name = model->GetName();
			ImVec2 content_ava = ImGui::GetContentRegionAvail();
			float line_height = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
			ImVec2 button_size = { content_ava.x, line_height };
			ImGui::Button(model_name.c_str(), button_size);

			DrawMaterial(model->m_matInstance);

			});

		
	}

}