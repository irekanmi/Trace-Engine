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

		static TraceGame* get_instance();
	private:

		void load_appinfo();
		void load_assetdb();

		std::string m_name;
		Ref<Scene> m_currentScene;
		UUID m_startScene;

	protected:

	};

}
