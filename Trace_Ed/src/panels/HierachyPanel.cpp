
#include "HierachyPanel.h"
#include "../TraceEditor.h"
#include "scene/Components.h"
#include "InspectorPanel.h"
#include "../utils/ImGui_utils.h"
#include "serialize/SceneSerializer.h"
#include "resource/PrefabManager.h"
#include "scene/Entity.h"


#include "imgui.h"
#include "imgui_internal.h"

namespace trace {
	HierachyPanel::HierachyPanel()
	{
		
	}
	void HierachyPanel::Render(Scene* scene, const std::string& tree_name, const std::string& window_name, float deltaTime)
	{
		//TraceEditor* editor = TraceEditor::get_instance();
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(2.0f, 4.0f));
		ImGui::Begin(window_name.c_str(), 0, ImGuiWindowFlags_NoCollapse);

		Scene* current_active_scene = scene;

		if (current_active_scene)
		{
			
			ImGuiTreeNodeFlags tree_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_DefaultOpen;
			bool clicked = ImGui::TreeNodeEx(tree_name.c_str(), tree_flags);

			if (clicked)
			{
				DrawAllEntites(current_active_scene);
				ImGui::TreePop();
			}



			if (ImGui::IsMouseDown(ImGuiMouseButton_Left) && ImGui::IsWindowHovered())
			{
				m_selectedEntity = Entity();
			}

			if (ImGui::BeginPopupContextWindow(0, ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
			{
				if (ImGui::MenuItem("Create Empty Entity"))
				{
					Entity new_entity = current_active_scene->CreateEntity();
					current_active_scene->DisableEntity(new_entity);
					current_active_scene->EnableEntity(new_entity);
				}
				ImGui::EndPopup();
			}

			

			
		}

		ImRect d_r = {};
		d_r.Min = ImGui::GetWindowPos();
		d_r.Max.x = d_r.Min.x + ImGui::GetWindowWidth();
		d_r.Max.y = d_r.Min.y + ImGui::GetWindowHeight();

		d_r.Min.x += 10.0f;
		d_r.Min.y += GetLineHeight() + 8.0f;
		d_r.Max.x -= 10.0f;
		d_r.Max.y -= 10.0f;
		if (ImGui::BeginDragDropTargetCustom(d_r, ImGui::GetID("Scene Hierachy Drag_Drop")))
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Entity"))
			{
				UUID uuid = *(UUID*)payload->Data;
				Entity child = current_active_scene->GetEntity(uuid);
				current_active_scene->AddToRoot(child);
			}
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Prefab Entity"))
			{
				UUID uuid = *(UUID*)payload->Data;
				Entity child = current_active_scene->GetEntity(uuid);
				current_active_scene->AddToRoot(child);
			}
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(".trprf"))
			{
				static char buf[1024] = { 0 };
				memcpy_s(buf, 1024, payload->Data, payload->DataSize);
				std::string path = buf;
				Ref<Prefab> prefab = SceneSerializer::DeserializePrefab(path);
				current_active_scene->InstanciatePrefab(prefab);
			}
			ImGui::EndDragDropTarget();
		}
		ImGui::End();
		ImGui::PopStyleVar();		


	}
	void HierachyPanel::RenderEntity(Entity entity, const std::string& tree_name, float deltaTime)
	{
		if (!entity)
		{
			return;
		}

		ImGuiTreeNodeFlags tree_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_DefaultOpen;
		bool clicked = ImGui::TreeNodeEx(tree_name.c_str(), tree_flags);

		if (clicked)
		{
			HierachyComponent& hi = entity.GetComponent<HierachyComponent>();
			bool is_active = entity.HasComponent<ActiveComponent>();


			bool selected = (m_selectedEntity == entity);
			ImGuiTreeNodeFlags tree_flags = (selected ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanFullWidth;
			TagComponent& tag = entity.GetScene()->m_registry.get<TagComponent>(entity);
			void* id = (void*)(uint64_t)(uint32_t)entity;


			if (entity.HasComponent<PrefabComponent>())
			{
				ImVec4 text_color = ImVec4(0.1f, 0.3f, 0.65f, 0.85f);
				if (!is_active)
				{
					text_color.w = 0.35f;
				}
				ImGui::PushStyleColor(ImGuiCol_Text, text_color);
			}
			else if (!is_active)
			{
				ImVec4* colors = ImGui::GetStyle().Colors;
				ImVec4 text_color = colors[ImGuiCol_Text];
				text_color.w = 0.35f;
				ImGui::PushStyleColor(ImGuiCol_Text, text_color);
			}
			bool clicked_entity = ImGui::TreeNodeEx(id, tree_flags, tag.GetTag().c_str());
			if (entity.HasComponent<PrefabComponent>() || !is_active)
			{
				ImGui::PopStyleColor(1);
			}


			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Entity"))
				{
					UUID uuid = *(UUID*)payload->Data;
					Entity new_child = entity.GetScene()->GetEntity(uuid);
					if (new_child && new_child != entity && !entity.GetScene()->IsParent(new_child, entity))
					{
						entity.GetScene()->SetParent(new_child, entity);
					}

				}
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Prefab Entity"))
				{
					UUID uuid = *(UUID*)payload->Data;
					Entity new_child = entity.GetScene()->GetEntity(uuid);
					if (new_child && new_child != entity && !entity.GetScene()->IsParent(new_child, entity))
					{
						entity.GetScene()->SetParent(new_child, entity);
					}

				}
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(".trprf"))
				{
					static char buf[1024] = { 0 };
					memcpy_s(buf, 1024, payload->Data, payload->DataSize);
					std::string path = buf;
					Ref<Prefab> prefab = SceneSerializer::DeserializePrefab(path);
					entity.GetScene()->InstanciatePrefab(prefab, entity);
				}
				ImGui::EndDragDropTarget();
			}

			if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
			{
				if (selected)
					m_selectedEntity = Entity();
				else
					m_selectedEntity = entity;
			}

			//FIX: Rendering twice if tag is the same
			if (ImGui::BeginPopupContextItem())
			{
				if (ImGui::MenuItem("Create Entity"))
				{
					Entity new_entity = entity.GetScene()->CreateEntity(entity.GetID());
					entity.GetScene()->DisableEntity(new_entity);
					entity.GetScene()->EnableEntity(new_entity);
				}
				if (ImGui::MenuItem("Delete Entity"))
				{
					entity.GetScene()->DestroyEntity(entity);
					m_selectedEntity = Entity();
				}
				ImGui::EndPopup();
			}


			if (clicked_entity)
			{
				DrawEntityHierachy(hi, entity.GetScene());
				ImGui::TreePop();
			}
			ImGui::TreePop();
		}

		if (ImGui::IsMouseDown(ImGuiMouseButton_Left) && ImGui::IsWindowHovered())
		{
			m_selectedEntity = Entity();
		}

		if (ImGui::BeginPopupContextWindow(0, ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
		{
			if (ImGui::MenuItem("Create Entity"))
			{
				Entity new_entity = entity.GetScene()->CreateEntity(entity.GetID());
			}
			ImGui::EndPopup();
		}

	}
	void HierachyPanel::DrawEntity(Entity entity, Scene* current_active_scene)
	{
		//TraceEditor* editor = TraceEditor::get_instance();
		bool selected = (m_selectedEntity == entity);
		ImGuiTreeNodeFlags tree_flags = ( selected ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanFullWidth;
		TagComponent& tag = current_active_scene->m_registry.get<TagComponent>(entity);
		void* id = (void*)(uint64_t)(uint32_t)entity;
		bool clicked = ImGui::TreeNodeEx(id, tree_flags, tag.GetTag().c_str());

		if (ImGui::IsItemClicked())
		{
			if (selected)
				m_selectedEntity = Entity();
			else
				m_selectedEntity = entity;
			
		}

		//FIX: Rendering twice if tag is the same
		if (ImGui::BeginPopupContextItem())
		{
			if (ImGui::MenuItem("Delete Entity")) 
			{ 
				current_active_scene->DestroyEntity(entity);
				if (selected) { m_selectedEntity = Entity(); }
			}
			ImGui::EndPopup();
		}


		if (clicked)
		{
			ImGui::TreePop();
		}

		
		
	}

	void HierachyPanel::DrawAllEntites(Scene* current_active_scene)
	{
		//TraceEditor* editor = TraceEditor::get_instance();
		for (UUID& uuid : current_active_scene->m_rootNode->children)
		{
			Entity entity = current_active_scene->GetEntity(uuid);
			HierachyComponent& hi = entity.GetComponent<HierachyComponent>();
			bool is_active = entity.HasComponent<ActiveComponent>();

			bool selected = (m_selectedEntity == entity);
			ImGuiTreeNodeFlags tree_flags = (selected ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanFullWidth;
			TagComponent& tag = current_active_scene->m_registry.get<TagComponent>(entity);
			void* id = (void*)(uint64_t)(uint32_t)entity;

			if (entity.HasComponent<PrefabComponent>())
			{
				ImVec4 text_color = ImVec4(0.1f, 0.3f, 0.65f, 0.85f);
				if (!is_active)
				{
					text_color.w = 0.35f;
				}
				ImGui::PushStyleColor(ImGuiCol_Text, text_color);
			}
			else if (!is_active)
			{
				ImVec4* colors = ImGui::GetStyle().Colors;
				ImVec4 text_color = colors[ImGuiCol_Text];
				text_color.w = 0.35f;
				ImGui::PushStyleColor(ImGuiCol_Text, text_color);
			}
			bool clicked = ImGui::TreeNodeEx(id, tree_flags, tag.GetTag().c_str());
			if (entity.HasComponent<PrefabComponent>() || !is_active)
			{
				ImGui::PopStyleColor(1);
			}

			if (ImGui::BeginDragDropSource())
			{
				UUID uuid = entity.GetID();
				if(entity.HasComponent<PrefabComponent>()) ImGui::SetDragDropPayload("Prefab Entity", &uuid, sizeof(UUID));
				else ImGui::SetDragDropPayload("Entity", &uuid, sizeof(UUID));
				ImGui::EndDragDropSource();
			}

			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Entity"))
				{
					UUID uuid = *(UUID*)payload->Data;
					Entity new_child = current_active_scene->GetEntity(uuid);
					if (new_child && new_child != entity && !current_active_scene->IsParent(new_child, entity))
					{
						current_active_scene->SetParent(new_child, entity);
					}
					
				}
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Prefab Entity"))
				{
					UUID uuid = *(UUID*)payload->Data;
					Entity new_child = current_active_scene->GetEntity(uuid);
					if (new_child && new_child != entity && !current_active_scene->IsParent(new_child, entity))
					{
						current_active_scene->SetParent(new_child, entity);
					}

				}
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(".trprf"))
				{
					static char buf[1024] = { 0 };
					memcpy_s(buf, 1024, payload->Data, payload->DataSize);
					std::string path = buf;
					Ref<Prefab> prefab = SceneSerializer::DeserializePrefab(path);
					current_active_scene->InstanciatePrefab(prefab, entity);
				}
				ImGui::EndDragDropTarget();
			}

			if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
			{
				if (selected)
					m_selectedEntity = Entity();
				else
					m_selectedEntity = entity;
			}

			//FIX: Rendering twice if tag is the same
			if (ImGui::BeginPopupContextItem())
			{
				if (ImGui::MenuItem("Create Entity"))
				{
					Entity new_entity = current_active_scene->CreateEntity(entity.GetID());
					current_active_scene->DisableEntity(new_entity);
					current_active_scene->EnableEntity(new_entity);
				}
				if (ImGui::MenuItem("Delete Entity"))
				{
					m_selectedEntity = Entity();
					current_active_scene->DestroyEntity(entity);
				}
				ImGui::EndPopup();
			}


			if (clicked)
			{
				DrawEntityHierachy(hi, current_active_scene);
				ImGui::TreePop();
			}



		}
	}
	void HierachyPanel::DrawEntityHierachy(HierachyComponent& hierachy, Scene* current_active_scene)
	{
		//TraceEditor* editor = TraceEditor::get_instance();
		for (UUID& uuid : hierachy.children)
		{
			
			Entity entity = current_active_scene->GetEntity(uuid);
			HierachyComponent& hi = entity.GetComponent<HierachyComponent>();
			bool is_active = entity.HasComponent<ActiveComponent>();


			bool selected = (m_selectedEntity == entity);
			ImGuiTreeNodeFlags tree_flags = (selected ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanFullWidth;
			TagComponent& tag = current_active_scene->m_registry.get<TagComponent>(entity);
			void* id = (void*)(uint64_t)(uint32_t)entity;


			if (entity.HasComponent<PrefabComponent>())
			{
				ImVec4 text_color = ImVec4(0.1f, 0.3f, 0.65f, 0.85f);
				if (!is_active)
				{
					text_color.w = 0.35f;
				}
				ImGui::PushStyleColor(ImGuiCol_Text, text_color);
			}
			else if (!is_active)
			{
				ImVec4* colors = ImGui::GetStyle().Colors;
				ImVec4 text_color = colors[ImGuiCol_Text];
				text_color.w = 0.35f;
				ImGui::PushStyleColor(ImGuiCol_Text, text_color);
			}
			bool clicked = ImGui::TreeNodeEx(id, tree_flags, tag.GetTag().c_str());
			if (entity.HasComponent<PrefabComponent>() || !is_active)
			{
				ImGui::PopStyleColor(1);
			}


			if (ImGui::BeginDragDropSource())
			{
				UUID uuid = entity.GetID();
				if (entity.HasComponent<PrefabComponent>()) ImGui::SetDragDropPayload("Prefab Entity", &uuid, sizeof(UUID));
				else ImGui::SetDragDropPayload("Entity", &uuid, sizeof(UUID));
				ImGui::EndDragDropSource();
			}

			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Entity"))
				{
					UUID uuid = *(UUID*)payload->Data;
					Entity new_child = current_active_scene->GetEntity(uuid);
					if (new_child && new_child != entity && !current_active_scene->IsParent(new_child, entity))
					{
						current_active_scene->SetParent(new_child, entity);
					}

				}
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Prefab Entity"))
				{
					UUID uuid = *(UUID*)payload->Data;
					Entity new_child = current_active_scene->GetEntity(uuid);
					if (new_child && new_child != entity && !current_active_scene->IsParent(new_child, entity))
					{
						current_active_scene->SetParent(new_child, entity);
					}

				}
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(".trprf"))
				{
					static char buf[1024] = { 0 };
					memcpy_s(buf, 1024, payload->Data, payload->DataSize);
					std::string path = buf;
					Ref<Prefab> prefab = SceneSerializer::DeserializePrefab(path);
					current_active_scene->InstanciatePrefab(prefab, entity);
				}
				ImGui::EndDragDropTarget();
			}

			if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
			{
				if (selected)
					m_selectedEntity = Entity();
				else
					m_selectedEntity = entity;
			}

			//FIX: Rendering twice if tag is the same
			if (ImGui::BeginPopupContextItem())
			{
				if (ImGui::MenuItem("Create Entity"))
				{
					Entity new_entity = current_active_scene->CreateEntity(entity.GetID());
					current_active_scene->DisableEntity(new_entity);
					current_active_scene->EnableEntity(new_entity);
				}
				if (ImGui::MenuItem("Delete Entity"))
				{
					current_active_scene->DestroyEntity(entity);
					m_selectedEntity = Entity();
				}
				ImGui::EndPopup();
			}


			if (clicked)
			{
				DrawEntityHierachy(hi, current_active_scene);
				ImGui::TreePop();
			}

		}
	}
	void HierachyPanel::DrawPrefabEntityHierachy(HierachyComponent& hierachy)
	{
		//TraceEditor* editor = TraceEditor::get_instance();
		//Scene* prefab_scene = PrefabManager::get_instance()->GetScene();
		//for (UUID& uuid : hierachy.children)
		//{

		//	Entity entity = prefab_scene->GetEntity(uuid);
		//	HierachyComponent& hi = entity.GetComponent<HierachyComponent>();


		//	bool selected = (m_selectedEntity == entity);
		//	ImGuiTreeNodeFlags tree_flags = (selected ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanFullWidth;
		//	TagComponent& tag = entity.GetComponent<TagComponent>();
		//	void* id = (void*)(uint64_t)(uint32_t)entity;


		//	if (entity.HasComponent<PrefabComponent>()) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.1f, 0.3f, 0.65f, 0.85f));
		//	bool clicked = ImGui::TreeNodeEx(id, tree_flags, tag.GetTag().c_str());
		//	if (entity.HasComponent<PrefabComponent>()) ImGui::PopStyleColor(1);


		//	if (ImGui::BeginDragDropSource())
		//	{
		//		UUID uuid = entity.GetID();
		//		if (entity.HasComponent<PrefabComponent>()) ImGui::SetDragDropPayload("Prefab Entity Edit", &uuid, sizeof(UUID));
		//		else ImGui::SetDragDropPayload("Entity Edit", &uuid, sizeof(UUID));
		//		ImGui::EndDragDropSource();
		//	}

		//	if (ImGui::BeginDragDropTarget())
		//	{
		//		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Entity"))
		//		{
		//			UUID uuid = *(UUID*)payload->Data;
		//			Entity new_child = prefab_scene->GetEntity(uuid);
		//			if (new_child && new_child != entity && !prefab_scene->IsParent(new_child, entity))
		//			{
		//				prefab_scene->SetParent(new_child, entity);
		//			}

		//		}
		//		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Prefab Entity"))
		//		{
		//			UUID uuid = *(UUID*)payload->Data;
		//			Entity new_child = prefab_scene->GetEntity(uuid);
		//			if (new_child && new_child != entity && !prefab_scene->IsParent(new_child, entity))
		//			{
		//				prefab_scene->SetParent(new_child, entity);
		//			}

		//		}
		//		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(".trprf"))
		//		{
		//			static char buf[1024] = { 0 };
		//			memcpy_s(buf, 1024, payload->Data, payload->DataSize);
		//			std::string path = buf;
		//			Ref<Prefab> prefab = SceneSerializer::DeserializePrefab(path);
		//			if(prefab->GetHandle() != m_editPrefab->GetHandle())
		//				prefab_scene->InstanciatePrefab(prefab, entity);
		//		}
		//		ImGui::EndDragDropTarget();
		//	}

		//	if (ImGui::IsItemClicked())
		//	{
		//		if (selected)
		//			m_selectedEntity = Entity();
		//		else
		//			m_selectedEntity = entity;

		//		editor->GetInspectorPanel()->SetDrawCallbackFn([editor]()
		//			{
		//				Entity selected_entity = editor->GetHierachyPanel()->GetSelectedEntity();
		//				if (selected_entity)
		//					editor->GetInspectorPanel()->DrawEntityComponent(selected_entity);
		//			}, []() {}, []() {});
		//	}

		//	//FIX: Rendering twice if tag is the same
		//	if (ImGui::BeginPopupContextItem())
		//	{
		//		if (ImGui::MenuItem("Create Entity"))
		//		{
		//			prefab_scene->CreateEntity(entity.GetID());
		//		}
		//		if (ImGui::MenuItem("Delete Entity"))
		//		{
		//			prefab_scene->DestroyEntity(entity);
		//			m_selectedEntity = Entity();
		//		}
		//		ImGui::EndPopup();
		//	}


		//	if (clicked)
		//	{
		//		DrawPrefabEntityHierachy(hi);
		//		ImGui::TreePop();
		//	}

		//}
	}
}