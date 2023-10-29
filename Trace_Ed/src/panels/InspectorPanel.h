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

		TraceEditor* m_editor;
	private:
		Entity m_selectedEntity;

	protected:
		friend class TraceEditor;
	};

}
