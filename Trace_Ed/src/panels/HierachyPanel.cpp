
#include "HierachyPanel.h"
#include "../TraceEditor.h"
#include "scene/Components.h"
#include "InspectorPanel.h"
#include "../utils/ImGui_utils.h"
#include "serialize/SceneSerializer.h"
#include "resource/PrefabManager.h"


#include "imgui.h"
#include "imgui_internal.h"

namespace trace {
	HierachyPanel::HierachyPanel()
	{
		
	}
	void HierachyPanel::Render(float deltaTime)
	{
		TraceEditor* editor = TraceEditor::get_instance();
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(2.0f, 4.0f));
		ImGui::Begin("Scene Hierachy", 0, ImGuiWindowFlags_NoCollapse);



		if (editor->GetCurrentScene())
		{
			
			std::string& scene_name = editor->GetEditScene()->GetName();
			ImGuiTreeNodeFlags tree_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_DefaultOpen;
			bool clicked = ImGui::TreeNodeEx(scene_name.c_str(), tree_flags);

			if (clicked)
			{
				DrawAllEntites();
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
					Entity new_entity = editor->GetCurrentScene()->CreateEntity();
					editor->GetCurrentScene()->DisableEntity(new_entity);
					editor->GetCurrentScene()->EnableEntity(new_entity);
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
				Entity child = editor->GetCurrentScene()->GetEntity(uuid);
				editor->GetCurrentScene()->AddToRoot(child);
			}
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Prefab Entity"))
			{
				UUID uuid = *(UUID*)payload->Data;
				Entity child = editor->GetCurrentScene()->GetEntity(uuid);
				editor->GetCurrentScene()->AddToRoot(child);
			}
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(".trprf"))
			{
				static char buf[1024] = { 0 };
				memcpy_s(buf, 1024, payload->Data, payload->DataSize);
				std::string path = buf;
				Ref<Prefab> prefab = SceneSerializer::DeserializePrefab(path);
				editor->GetCurrentScene()->InstanciatePrefab(prefab);
			}
			ImGui::EndDragDropTarget();
		}
		ImGui::End();
		ImGui::PopStyleVar();
		
		RenderPrefab(deltaTime);


	}
	void HierachyPanel::DrawEntity(Entity entity)
	{
		TraceEditor* editor = TraceEditor::get_instance();
		bool selected = (m_selectedEntity == entity);
		ImGuiTreeNodeFlags tree_flags = ( selected ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanFullWidth;
		TagComponent& tag = editor->GetCurrentScene()->m_registry.get<TagComponent>(entity);
		void* id = (void*)(uint64_t)(uint32_t)entity;
		bool clicked = ImGui::TreeNodeEx(id, tree_flags, tag.GetTag().c_str());

		if (ImGui::IsItemClicked())
		{
			if (selected)
				m_selectedEntity = Entity();
			else
				m_selectedEntity = entity;

			editor->GetInspectorPanel()->SetDrawCallbackFn([&]()
				{
					if (m_selectedEntity)
						editor->GetInspectorPanel()->DrawEntityComponent(m_selectedEntity);
				}, []() {}, []() {});
		}

		//FIX: Rendering twice if tag is the same
		if (ImGui::BeginPopupContextItem())
		{
			if (ImGui::MenuItem("Delete Entity")) 
			{ 
				editor->GetCurrentScene()->DestroyEntity(entity);
				if (selected) { m_selectedEntity = Entity(); }
			}
			ImGui::EndPopup();
		}


		if (clicked)
		{
			ImGui::TreePop();
		}

		
		
	}
	void HierachyPanel::SetPrefabEdit(Ref<Prefab> prefab)
	{
		m_editPrefab = prefab;
		m_selectedEntity = Entity();
	}
	void HierachyPanel::DrawAllEntites()
	{
		TraceEditor* editor = TraceEditor::get_instance();
		for (UUID& uuid : editor->GetCurrentScene()->m_rootNode->children)
		{
			Entity entity = editor->GetCurrentScene()->GetEntity(uuid);
			HierachyComponent& hi = entity.GetComponent<HierachyComponent>();
			bool is_active = entity.HasComponent<ActiveComponent>();

			bool selected = (m_selectedEntity == entity);
			ImGuiTreeNodeFlags tree_flags = (selected ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanFullWidth;
			TagComponent& tag = editor->GetCurrentScene()->m_registry.get<TagComponent>(entity);
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
					Entity new_child = editor->GetCurrentScene()->GetEntity(uuid);
					if (new_child && new_child != entity && !editor->GetCurrentScene()->IsParent(new_child, entity))
					{
						editor->GetCurrentScene()->SetParent(new_child, entity);
					}
					
				}
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Prefab Entity"))
				{
					UUID uuid = *(UUID*)payload->Data;
					Entity new_child = editor->GetCurrentScene()->GetEntity(uuid);
					if (new_child && new_child != entity && !editor->GetCurrentScene()->IsParent(new_child, entity))
					{
						editor->GetCurrentScene()->SetParent(new_child, entity);
					}

				}
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(".trprf"))
				{
					static char buf[1024] = { 0 };
					memcpy_s(buf, 1024, payload->Data, payload->DataSize);
					std::string path = buf;
					Ref<Prefab> prefab = SceneSerializer::DeserializePrefab(path);
					editor->GetCurrentScene()->InstanciatePrefab(prefab, entity);
				}
				ImGui::EndDragDropTarget();
			}

			if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
			{
				if (selected)
					m_selectedEntity = Entity();
				else
					m_selectedEntity = entity;

				editor->GetInspectorPanel()->SetDrawCallbackFn([editor]()
					{
						Entity selected_entity = editor->GetHierachyPanel()->GetSelectedEntity();
						if (selected_entity)
						{
							editor->GetInspectorPanel()->DrawEntityComponent(selected_entity);
						}
					}, []() {}, []() {});
			}

			//FIX: Rendering twice if tag is the same
			if (ImGui::BeginPopupContextItem())
			{
				if (ImGui::MenuItem("Create Entity"))
				{
					Entity new_entity = editor->GetCurrentScene()->CreateEntity(entity.GetID());
					editor->GetCurrentScene()->DisableEntity(new_entity);
					editor->GetCurrentScene()->EnableEntity(new_entity);
				}
				if (ImGui::MenuItem("Delete Entity"))
				{
					m_selectedEntity = Entity();
					editor->GetCurrentScene()->DestroyEntity(entity);
				}
				ImGui::EndPopup();
			}


			if (clicked)
			{
				DrawEntityHierachy(hi);
				ImGui::TreePop();
			}



		}
	}
	void HierachyPanel::DrawEntityHierachy(HierachyComponent& hierachy)
	{
		TraceEditor* editor = TraceEditor::get_instance();
		for (UUID& uuid : hierachy.children)
		{
			
			Entity entity = editor->GetCurrentScene()->GetEntity(uuid);
			HierachyComponent& hi = entity.GetComponent<HierachyComponent>();
			bool is_active = entity.HasComponent<ActiveComponent>();


			bool selected = (m_selectedEntity == entity);
			ImGuiTreeNodeFlags tree_flags = (selected ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanFullWidth;
			TagComponent& tag = editor->GetCurrentScene()->m_registry.get<TagComponent>(entity);
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
					Entity new_child = editor->GetCurrentScene()->GetEntity(uuid);
					if (new_child && new_child != entity && !editor->GetCurrentScene()->IsParent(new_child, entity))
					{
						editor->GetCurrentScene()->SetParent(new_child, entity);
					}

				}
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Prefab Entity"))
				{
					UUID uuid = *(UUID*)payload->Data;
					Entity new_child = editor->GetCurrentScene()->GetEntity(uuid);
					if (new_child && new_child != entity && !editor->GetCurrentScene()->IsParent(new_child, entity))
					{
						editor->GetCurrentScene()->SetParent(new_child, entity);
					}

				}
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(".trprf"))
				{
					static char buf[1024] = { 0 };
					memcpy_s(buf, 1024, payload->Data, payload->DataSize);
					std::string path = buf;
					Ref<Prefab> prefab = SceneSerializer::DeserializePrefab(path);
					editor->GetCurrentScene()->InstanciatePrefab(prefab, entity);
				}
				ImGui::EndDragDropTarget();
			}

			if (ImGui::IsItemClicked())
			{
				if (selected)
					m_selectedEntity = Entity();
				else
					m_selectedEntity = entity;

				editor->GetInspectorPanel()->SetDrawCallbackFn([editor]()
					{
						Entity selected_entity = editor->GetHierachyPanel()->GetSelectedEntity();
						if (selected_entity)
						{
							editor->GetInspectorPanel()->DrawEntityComponent(selected_entity);
						}
					}, []() {}, []() {});
			}

			//FIX: Rendering twice if tag is the same
			if (ImGui::BeginPopupContextItem())
			{
				if (ImGui::MenuItem("Create Entity"))
				{
					Entity new_entity = editor->GetCurrentScene()->CreateEntity(entity.GetID());
					editor->GetCurrentScene()->DisableEntity(new_entity);
					editor->GetCurrentScene()->EnableEntity(new_entity);
				}
				if (ImGui::MenuItem("Delete Entity"))
				{
					editor->GetCurrentScene()->DestroyEntity(entity);
					m_selectedEntity = Entity();
				}
				ImGui::EndPopup();
			}


			if (clicked)
			{
				DrawEntityHierachy(hi);
				ImGui::TreePop();
			}

		}
	}
	void HierachyPanel::DrawPrefabEntityHierachy(HierachyComponent& hierachy)
	{
		TraceEditor* editor = TraceEditor::get_instance();
		Scene* prefab_scene = PrefabManager::get_instance()->GetScene();
		for (UUID& uuid : hierachy.children)
		{

			Entity entity = prefab_scene->GetEntity(uuid);
			HierachyComponent& hi = entity.GetComponent<HierachyComponent>();


			bool selected = (m_selectedEntity == entity);
			ImGuiTreeNodeFlags tree_flags = (selected ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanFullWidth;
			TagComponent& tag = entity.GetComponent<TagComponent>();
			void* id = (void*)(uint64_t)(uint32_t)entity;


			if (entity.HasComponent<PrefabComponent>()) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.1f, 0.3f, 0.65f, 0.85f));
			bool clicked = ImGui::TreeNodeEx(id, tree_flags, tag.GetTag().c_str());
			if (entity.HasComponent<PrefabComponent>()) ImGui::PopStyleColor(1);


			if (ImGui::BeginDragDropSource())
			{
				UUID uuid = entity.GetID();
				if (entity.HasComponent<PrefabComponent>()) ImGui::SetDragDropPayload("Prefab Entity Edit", &uuid, sizeof(UUID));
				else ImGui::SetDragDropPayload("Entity Edit", &uuid, sizeof(UUID));
				ImGui::EndDragDropSource();
			}

			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Entity"))
				{
					UUID uuid = *(UUID*)payload->Data;
					Entity new_child = prefab_scene->GetEntity(uuid);
					if (new_child && new_child != entity && !prefab_scene->IsParent(new_child, entity))
					{
						prefab_scene->SetParent(new_child, entity);
					}

				}
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Prefab Entity"))
				{
					UUID uuid = *(UUID*)payload->Data;
					Entity new_child = prefab_scene->GetEntity(uuid);
					if (new_child && new_child != entity && !prefab_scene->IsParent(new_child, entity))
					{
						prefab_scene->SetParent(new_child, entity);
					}

				}
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(".trprf"))
				{
					static char buf[1024] = { 0 };
					memcpy_s(buf, 1024, payload->Data, payload->DataSize);
					std::string path = buf;
					Ref<Prefab> prefab = SceneSerializer::DeserializePrefab(path);
					if(prefab->GetHandle() != m_editPrefab->GetHandle())
						prefab_scene->InstanciatePrefab(prefab, entity);
				}
				ImGui::EndDragDropTarget();
			}

			if (ImGui::IsItemClicked())
			{
				if (selected)
					m_selectedEntity = Entity();
				else
					m_selectedEntity = entity;

				editor->GetInspectorPanel()->SetDrawCallbackFn([editor]()
					{
						Entity selected_entity = editor->GetHierachyPanel()->GetSelectedEntity();
						if (selected_entity)
							editor->GetInspectorPanel()->DrawEntityComponent(selected_entity);
					}, []() {}, []() {});
			}

			//FIX: Rendering twice if tag is the same
			if (ImGui::BeginPopupContextItem())
			{
				if (ImGui::MenuItem("Create Entity"))
				{
					prefab_scene->CreateEntity(entity.GetID());
				}
				if (ImGui::MenuItem("Delete Entity"))
				{
					prefab_scene->DestroyEntity(entity);
					m_selectedEntity = Entity();
				}
				ImGui::EndPopup();
			}


			if (clicked)
			{
				DrawPrefabEntityHierachy(hi);
				ImGui::TreePop();
			}

		}
	}
	void HierachyPanel::RenderPrefab(float deltaTime)
	{
		TraceEditor* editor = TraceEditor::get_instance();
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(2.0f, 4.0f));
		ImGui::Begin("Prefab Edit", 0, ImGuiWindowFlags_NoCollapse);



		if (m_editPrefab)
		{
			Scene* prefab_scene = PrefabManager::get_instance()->GetScene();

			std::string& scene_name = prefab_scene->GetName();
			ImGuiTreeNodeFlags tree_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_DefaultOpen;
			bool clicked = ImGui::TreeNodeEx(scene_name.c_str(), tree_flags);

			if (clicked)
			{
				Entity entity = prefab_scene->GetEntity(m_editPrefab->GetHandle());
				HierachyComponent& hi = entity.GetComponent<HierachyComponent>();


				bool selected = (m_selectedEntity == entity);
				ImGuiTreeNodeFlags tree_flags = (selected ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanFullWidth;
				TagComponent& tag = entity.GetComponent<TagComponent>();
				void* id = (void*)(uint64_t)(uint32_t)entity;


				if (entity.HasComponent<PrefabComponent>()) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.1f, 0.3f, 0.65f, 0.85f));
				bool clicked = ImGui::TreeNodeEx(id, tree_flags, tag.GetTag().c_str());
				if (entity.HasComponent<PrefabComponent>()) ImGui::PopStyleColor(1);


				if (ImGui::BeginDragDropSource())
				{
					UUID uuid = entity.GetID();
					if (entity.HasComponent<PrefabComponent>()) ImGui::SetDragDropPayload("Prefab Entity Edit", &uuid, sizeof(UUID));
					else ImGui::SetDragDropPayload("Entity Edit", &uuid, sizeof(UUID));
					ImGui::EndDragDropSource();
				}

				if (ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Entity"))
					{
						UUID uuid = *(UUID*)payload->Data;
						Entity new_child = prefab_scene->GetEntity(uuid);
						if (new_child && new_child != entity && !prefab_scene->IsParent(new_child, entity))
						{
							prefab_scene->SetParent(new_child, entity);
						}

					}
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Prefab Entity"))
					{
						UUID uuid = *(UUID*)payload->Data;
						Entity new_child = prefab_scene->GetEntity(uuid);
						if (new_child && new_child != entity && !prefab_scene->IsParent(new_child, entity))
						{
							prefab_scene->SetParent(new_child, entity);
						}

					}
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(".trprf"))
					{
						static char buf[1024] = { 0 };
						memcpy_s(buf, 1024, payload->Data, payload->DataSize);
						std::string path = buf;
						Ref<Prefab> prefab = SceneSerializer::DeserializePrefab(path);
						if (prefab->GetHandle() != m_editPrefab->GetHandle())
							prefab_scene->InstanciatePrefab(prefab, entity);
					}
					ImGui::EndDragDropTarget();
				}

				if (ImGui::IsItemClicked())
				{
					if (selected)
						m_selectedEntity = Entity();
					else
						m_selectedEntity = entity;

					editor->GetInspectorPanel()->SetDrawCallbackFn([editor]()
						{
							Entity selected_entity = editor->GetHierachyPanel()->GetSelectedEntity();
							if (selected_entity)
								editor->GetInspectorPanel()->DrawEntityComponent(selected_entity);
						}, []() {}, []() {});
				}

				//FIX: Rendering twice if tag is the same
				if (ImGui::BeginPopupContextItem())
				{
					if (ImGui::MenuItem("Create Entity"))
					{
						prefab_scene->CreateEntity(entity.GetID());
					}
					ImGui::EndPopup();
				}


				if (clicked)
				{
					DrawPrefabEntityHierachy(hi);
					ImGui::TreePop();
				}
				ImGui::TreePop();
			}



			if (ImGui::IsMouseDown(ImGuiMouseButton_Left) && ImGui::IsWindowHovered())
				m_selectedEntity = Entity();

			if (ImGui::BeginPopupContextWindow(0, ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
			{
				if (ImGui::MenuItem("Create Empty Entity"))
				{
					prefab_scene->CreateEntity(m_editPrefab->GetHandle());
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
		if (ImGui::BeginDragDropTargetCustom(d_r, ImGui::GetID("Prefab Edit Drag_Drop")))
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Entity"))
			{
				UUID uuid = *(UUID*)payload->Data;

			}
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Prefab Entity"))
			{
				UUID uuid = *(UUID*)payload->Data;

			}
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(".trprf"))
			{
				static char buf[1024] = { 0 };
				memcpy_s(buf, 1024, payload->Data, payload->DataSize);
			}
			ImGui::EndDragDropTarget();
		}
		ImGui::End();
		ImGui::PopStyleVar();
	}
}