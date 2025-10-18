#pragma once

#include "EditorWindow.h"
#include "render/Camera.h"
#include "scene/Scene.h"
#include "particle_effects/ParticleGenerator.h"

namespace trace {

	class GenericGraphEditor;
	class InspectorPanel;

	class ParticleGeneratorWindow : public EditorWindow
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
		void render_class_elements(void* ptr);

	private:
		Ref<ParticleGenerator> m_particleGenerator;
		Camera m_camera;
		Scene* m_scene;
		int32_t view_index;
		GenericGraphEditor* m_editor;
		InspectorPanel* m_inspector;
		glm::vec2 m_viewportSize;
		std::string graph_editor_name;
		std::string inspector_name;
		std::string viewport_name;
		std::string tool_bar_name;
		int gizmo_mode = 1;
		std::string generator_path;
		ParticleEffectInstance* particle_effect_instance;
		bool is_playing = false;
		UUID visual_id = 0;



	protected:

	};

}
