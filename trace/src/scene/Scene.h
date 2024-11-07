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
	using StringID = size_t;

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
		Entity GetEntityByName(const std::string& name);
		Entity GetEntityByName(StringID name);
		Entity GetChildEntityByName(Entity parent, const std::string& name);
		Entity GetChildEntityByName(Entity parent, StringID name);
		Entity GetParentByName(Entity entity, std::string parent_name);
		Entity GetParentByName(Entity entity, StringID parent_name);
		Entity FindEnityInHierachy(Entity entity, StringID name);
		Entity GetParentWithAnimation(Entity entity);
		Entity DuplicateEntity(Entity entity);
		bool CopyEntity(Entity entity, Entity src);
		void DestroyEntity(Entity entity);
		Entity InstanciatePrefab(Ref<Prefab> prefab);
		Entity InstanciatePrefab(Ref<Prefab> prefab, Entity parent);
		bool ApplyPrefabChanges(Ref<Prefab> prefab);

		bool ApplyPrefabChangesOnSceneLoad();

		void SetParent(Entity child, Entity parent);
		bool IsParent(Entity parent, Entity child);
		void AddToRoot(Entity entity);
		Transform GetEntityWorldTransform(Entity entity);
		ScriptRegistry& GetScriptRegistry() { return m_scriptRegistry; }

		Transform GetEntityGlobalPose(Entity entity, bool recompute = false);


		void ProcessEntitiesByHierachy(std::function<void(Entity, UUID, Scene*)> callback);
		void ResolveHierachyTransforms();

		std::string& GetName() { return m_name; }
		void SetName(const std::string& name) { m_name = name; }
		bool IsRunning() { return m_running; }

		template<typename Component>
		Entity ParentHasComponent(Entity entity)
		{
			Entity result;

			HierachyComponent& hi = entity.GetComponent<HierachyComponent>();
			UUID parent = hi.parent;
			while (parent != 0)
			{
				Entity en = GetEntity(parent);
				if (en.HasComponent<Component>())
				{
					result = en;
					break;
				}
				parent = en.GetComponent<HierachyComponent>().parent;
			}

			return result;
		}

		static void Copy(Ref<Scene> from, Ref<Scene> to);

	private:
		void ProcessEntityHierachy(HierachyComponent& hierachy, std::function<void(Entity, UUID, Scene*)>& callback, bool child_first = false);
		void ApplyPrefabChanges(Entity prefab_handle, Entity entity);

		void OnConstructHierachyComponent(entt::registry& reg, entt::entity ent);
		void OnDestroyHierachyComponent(entt::registry& reg, entt::entity ent);
		void OnConstructRigidBodyComponent(entt::registry& reg, entt::entity ent);
		void OnDestroyRigidBodyComponent(entt::registry& reg, entt::entity ent);


		void OnConstructSkinnedModelRendererComponent(entt::registry& reg, entt::entity ent);
		void OnDestroySkinnedModelRendererComponent(entt::registry& reg, entt::entity ent);

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
