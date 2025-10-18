#pragma once

#include "EditorWindow.h"
#include "render/Camera.h"
#include "scene/Scene.h"
#include "particle_effects/ParticleEffect.h"

namespace trace {

	class InspectorPanel;

	class ParticleEffectsWindow : public EditorWindow
	{

	public:

		bool OnCreate(TraceEditor* editor, const std::string& name, const std::string& file_path);
		virtual void OnDestroy(TraceEditor* editor);
		virtual void OnUpdate(float deltaTime);
		virtual void OnRender(float deltaTime);
		virtual void DockChildWindows();
		virtual void RenderViewport(std::vector<void*>& texture_handles);
		virtual void OnEvent(Event* p_event);

	private:
		Ref<ParticleEffect> m_particleEffect;
		Camera m_camera;
		Scene* m_scene;
		int32_t view_index;
		InspectorPanel* m_inspector;
		glm::vec2 m_viewportSize;
		std::string editor_name;
		std::string inspector_name;
		std::string viewport_name;
		std::string tool_bar_name;
		int gizmo_mode = 1;
		std::string particle_effect_path;
		UUID visual_id = 0;
		ParticleEffectInstance* particle_effect_instance;
		bool is_playing = false;



	protected:

	};

}
