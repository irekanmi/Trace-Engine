#pragma once

#include "scene/Entity.h"
#include "render/Material.h"

namespace trace {

	class AnimationPanel;
	class InspectorPanel
	{

	public:
		InspectorPanel();
		~InspectorPanel() {}


		void DrawEntityComponent(Entity entity, AnimationPanel* animation_panel = nullptr);
		void SetDrawCallbackFn(std::function<void()> cb, std::function<void()> on_enter, std::function<void()> on_exit);

		bool DrawEditMaterial(Ref<MaterialInstance> asset, MaterialData& material_data);

		Entity GetSelectedEntity() { return m_selectedEntity; }
		std::function<void()>& GetDrawCallback() { return m_drawCallback; }

	private:
		std::function<void()> m_drawCallback;
		std::function<void()> m_onExit;
		Entity m_selectedEntity;

	protected:
	};

}
