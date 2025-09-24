#pragma once

#include "scene/Scene.h"
#include "core/events/Events.h"

namespace trace {

	class TraceGame
	{

	public:

		bool Init();
		void Start();
		void Update(float deltaTime);
		void Render(float deltaTime);
		void End();
		std::string& GetName() { return m_name; }
		void OnEvent(Event* p_event);
		bool SetNextScene(Ref<Scene> scene);

		static TraceGame* get_instance();
	private:

		void load_appinfo();
		void load_assetdb();
		void start_current_scene();
		void stop_current_scene();

		std::string m_name;
		Ref<Scene> m_currentScene;
		Ref<Scene> m_nextScene;
		UUID m_startScene;
		int32_t swapchain_graph = -1;

	protected:

	};

}
