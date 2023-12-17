#pragma once

#include "resource/Resource.h"
#include "resource/Ref.h"
#include "entt.hpp"
#include "render/Commands.h"
#include "UUID.h"
#include "scripting/ScriptRegistry.h"

namespace trace {

	class Entity;

	class Scene : public Resource
	{

	public:
		Scene(){}
		Scene(const Scene& other) {/*TODO: Implement*/ };
		~Scene(){}

		void Create();
		void Destroy();
		void OnStart();
		void OnStop();
		void OnUpdate(float deltaTime);
		void OnPhysicsUpdate(float deltaTime);
		void OnRender();
		void OnRender(CommandList& cmd_list);
		void OnViewportChange(float width, float height);

		Entity CreateEntity();
		Entity CreateEntity(const std::string& _tag);
		Entity CreateEntity_UUID(UUID id,const std::string& _tag);
		void DuplicateEntity(Entity entity);
		void DestroyEntity(Entity entity);

		std::string& GetName() { return m_name; }
		void SetName(const std::string& name) { m_name = name; }


		static void Copy(Ref<Scene> from, Ref<Scene> to);

	private:
		entt::registry m_registry;
		ScriptRegistry m_scriptRegistry;
		std::string m_name;
		void* m_physics3D = nullptr;
		std::unordered_map<UUID, Entity> m_entityMap;

	protected:
		friend class Entity;
		friend class TraceEditor;
		friend class HierachyPanel;
		friend class SceneSerializer;
		friend class InspectorPanel;
	};

}
