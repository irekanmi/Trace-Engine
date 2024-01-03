#pragma once

#include "scene/Entity.h"

namespace trace {

	class TraceEditor;

	class HierachyPanel
	{

	public:
		HierachyPanel();
		~HierachyPanel(){}

		void Render(float deltaTime);

		void DrawEntity(Entity entity);

	private:
		TraceEditor* m_editor;
		Entity m_selectedEntity;

		void DrawAllEntites();
		void DrawEntityHierachy(HierachyComponent& hierachy);

	protected:
		friend class TraceEditor;
	};

}
