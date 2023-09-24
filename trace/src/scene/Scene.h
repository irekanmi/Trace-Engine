#pragma once

#include "resource/Resource.h"
#include "entt.hpp"

namespace trace {

	class Entity;

	class Scene : public Resource
	{

	public:
		Scene(){}
		Scene(const Scene& other) {/*TODO: Implement*/ };
		~Scene(){}

		void OnCreate();
		void OnDestroy();

		Entity CreateEntity();
		Entity CreateEntity(const std::string& tag);

		void DestroyEntity(Entity entity);

	private:
		entt::registry m_registry;

	protected:
		friend class Entity;
		friend class TraceEditor;
		friend class HierachyPanel;
	};

}
