#pragma once

#include "scene/Entity.h"

namespace trace {

	class TraceEditor;

	class InspectorPanel
	{

	public:
		InspectorPanel();
		~InspectorPanel() {}


		void DrawEntityComponent(Entity entity);

	private:
		TraceEditor* m_editor;
		Entity m_selectedEntity;

	protected:
		friend class TraceEditor;
	};

}
