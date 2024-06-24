#pragma once

#include "scene/Entity.h"
#include "resource/Prefab.h"

namespace trace {


	class HierachyPanel
	{

	public:
		HierachyPanel();
		~HierachyPanel(){}

		void Render(float deltaTime);

		void DrawEntity(Entity entity);

		void SetPrefabEdit(Ref<Prefab> prefab) { m_editPrefab = prefab; }
		Ref<Prefab> GetPrefabEdit() { return m_editPrefab; }
		Entity GetSelectedEntity() { return m_selectedEntity; }


		void SetSelectedEntity(Entity entity) { m_selectedEntity = entity; }

	private:
		Entity m_selectedEntity;
		Entity m_selectedPrefabEntity;
		Ref<Prefab> m_editPrefab;

		void DrawAllEntites();
		void DrawEntityHierachy(HierachyComponent& hierachy);
		void DrawPrefabEntityHierachy(HierachyComponent& hierachy);

		void RenderPrefab(float deltaTime);

	protected:
	};

}
