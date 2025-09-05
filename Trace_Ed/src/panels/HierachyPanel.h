#pragma once

#include "scene/Entity.h"
#include "resource/Prefab.h"

namespace trace {


	class HierachyPanel
	{

	public:
		HierachyPanel();
		~HierachyPanel(){}

		void Render(Scene* scene, const std::string& tree_name, float deltaTime);
		void RenderEntity(Entity entity, const std::string& tree_name, float deltaTime);

		void DrawEntity(Entity entity, Scene* scene);

		Entity GetSelectedEntity() { return m_selectedEntity; }


		void SetSelectedEntity(Entity entity) { m_selectedEntity = entity; }

	private:
		Entity m_selectedEntity;
		Entity m_selectedPrefabEntity;

		void DrawAllEntites(Scene* scene);
		void DrawEntityHierachy(HierachyComponent& hierachy, Scene* scene);
		void DrawPrefabEntityHierachy(HierachyComponent& hierachy);


	protected:
	};

}
