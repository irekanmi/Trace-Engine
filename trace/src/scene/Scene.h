#pragma once

#include "resource/Resource.h"
#include "resource/Ref.h"
#include "entt.hpp"
#include "render/Commands.h"
#include "UUID.h"
#include "scripting/ScriptRegistry.h"
#include "Components.h"

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
		void OnScriptStart();
		void OnStop();
		void OnScriptStop();
		void OnUpdate(float deltaTime);
		void OnScriptUpdate(float deltaTime);
		void OnPhysicsUpdate(float deltaTime);
		void OnAnimationUpdate(float deltaTime);
		void OnRender();
		void OnRender(CommandList& cmd_list);
		void OnViewportChange(float width, float height);

		Entity CreateEntity(UUID parent = 0);
		Entity CreateEntity(const std::string& _tag, UUID parent = 0);
		Entity CreateEntity_UUID(UUID id,const std::string& _tag, UUID parent = 0);
		Entity CreateEntity_UUIDWithParent(UUID id,const std::string& _tag, UUID parent = 0);
		Entity GetEntity(UUID uuid);
		void DuplicateEntity(Entity entity);
		void DestroyEntity(Entity entity);
		void SetParent(Entity child, Entity parent);
		void AddToRoot(Entity entity);
		Transform GetEntityWorldTransform(Entity entity);
		ScriptRegistry& GetScriptRegistry() { return m_scriptRegistry; }

		Transform GetEntityGlobalPose(Entity entity);


		void ProcessEntitiesByHierachy(std::function<void(Entity, UUID, Scene*)> callback);
		void ResolveHierachyTransforms();

		std::string& GetName() { return m_name; }
		void SetName(const std::string& name) { m_name = name; }
		bool IsRunning() { return m_running; }


		static void Copy(Ref<Scene> from, Ref<Scene> to);

	private:
		void ProcessEntityHierachy(HierachyComponent& hierachy, std::function<void(Entity, UUID, Scene*)>& callback, bool child_first = false);

		void OnConstructHierachyComponent(entt::registry& reg, entt::entity ent);
		void OnDestroyHierachyComponent(entt::registry& reg, entt::entity ent);
		void OnConstructRigidBodyComponent(entt::registry& reg, entt::entity ent);
		void OnDestroyRigidBodyComponent(entt::registry& reg, entt::entity ent);

		entt::registry m_registry;
		ScriptRegistry m_scriptRegistry;
		std::string m_name;
		void* m_physics3D = nullptr;
		std::unordered_map<UUID, Entity> m_entityMap;
		HierachyComponent* m_rootNode;
		bool m_running = false;

	protected:
		friend class Entity;
		friend class TraceEditor;
		friend class HierachyPanel;
		friend class SceneSerializer;
		friend class InspectorPanel;
	};

}
