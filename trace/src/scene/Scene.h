#pragma once

#include "resource/Resource.h"
#include "resource/Ref.h"
#include "entt.hpp"
#include "render/Commands.h"
#include "UUID.h"

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
		void OnUpdate(float deltaTime);
		void OnRender();
		void OnRender(CommandList& cmd_list);
		void OnViewportChange(float width, float height);

		Entity CreateEntity();
		Entity CreateEntity(const std::string& _tag);
		Entity CreateEntity_UUID(UUID id,const std::string& _tag);
		void DestroyEntity(Entity entity);

		std::string& GetName() { return m_name; }
		void SetName(const std::string& name) { m_name = name; }


		static void Copy(Ref<Scene> from, Ref<Scene> to);

	private:
		entt::registry m_registry;
		std::string m_name;

	protected:
		friend class Entity;
		friend class TraceEditor;
		friend class HierachyPanel;
		friend class SceneSerializer;
	};

}
