#pragma once

#include "resource/Resource.h"
#include "resource/Ref.h"
#include "entt.hpp"
#include "render/Commands.h"
#include "UUID.h"
#include "scripting/ScriptRegistry.h"
#include "Components.h"
#include "core/Coretypes.h"

namespace trace::Network {
	class NetworkStream;
}

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
		void BeginFrame();
		void EndFrame();
		void OnStart();
		void OnScriptStart();
		void OnPhysicsStart();
		void OnNetworkStart();
		void OnStop();
		void OnScriptStop();
		void OnPhysicsStop();
		void OnNetworkStop();
		void OnUpdate(float deltaTime);
		void OnScriptUpdate(float deltaTime);
		void OnPhysicsUpdate(float deltaTime);
		void OnNetworkUpdate(float deltaTime);
		void OnAnimationUpdate(float deltaTime);
		void OnRender();
		void OnRender(CommandList& cmd_list);
		void OnViewportChange(float width, float height);
		bool InitializeSceneComponents();
		void OnPacketReceive_Client(Network::NetworkStream* data, uint32_t source_handle);
		void OnPacketReceive_Server(Network::NetworkStream* data, uint32_t source_handle);

		void EnableEntity(Entity entity);
		void DisableEntity(Entity entity);
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
		Entity InstanciateEntity(Entity source, glm::vec3 position);
		bool ApplyPrefabChanges(Ref<Prefab> prefab);

		bool ApplyPrefabChangesOnSceneLoad();

		void SetParent(Entity child, Entity parent);
		bool IsParent(Entity parent, Entity child);
		void AddToRoot(Entity entity);
		Transform GetEntityWorldTransform(Entity entity);
		ScriptRegistry& GetScriptRegistry() { return m_scriptRegistry; }

		Transform GetEntityGlobalPose(Entity entity, bool recompute = false);
		glm::vec3 GetEntityWorldPosition(Entity entity);
		glm::quat GetEntityWorldRotation(Entity entity);
		glm::vec3 GetEntityWorldScale(Entity entity);

		void SetEntityWorldPosition(Entity entity, glm::vec3 position);
		void SetEntityWorldRotation(Entity entity, glm::quat rotation);
		void SetEntityWorldScale(Entity entity, glm::vec3 scale);


		void ProcessEntitiesByHierachy(std::function<void(Entity, UUID, Scene*)> callback, bool skip_inactive = true);
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

		template<typename Component>
		void IterateComponent(std::function<void(Entity)> callback)
		{
			auto comp_group = m_registry.view<Component>();
			for (auto i : comp_group)
			{
				auto [comp] = comp_group.get(i);
				Entity entity(i, this);
				callback(entity);

			}
		}

		static void Copy(Ref<Scene> from, Ref<Scene> to);

	private:
		void ProcessEntityHierachy(HierachyComponent& hierachy, std::function<void(Entity, UUID, Scene*)>& callback, bool skip_inactive = true, bool child_first = false);
		void ApplyPrefabChanges(Entity prefab_handle, Entity entity);

		void OnConstructHierachyComponent(entt::registry& reg, entt::entity ent);
		void OnDestroyHierachyComponent(entt::registry& reg, entt::entity ent);
		void OnConstructRigidBodyComponent(entt::registry& reg, entt::entity ent);
		void OnDestroyRigidBodyComponent(entt::registry& reg, entt::entity ent);
		void OnConstructBoxColliderComponent(entt::registry& reg, entt::entity ent);
		void OnDestroyBoxColliderComponent(entt::registry& reg, entt::entity ent);
		void OnConstructSphereColliderComponent(entt::registry& reg, entt::entity ent);
		void OnDestroySphereColliderComponent(entt::registry& reg, entt::entity ent);
		void OnConstructCharacterControllerComponent(entt::registry& reg, entt::entity ent);
		void OnDestroyCharacterControllerComponent(entt::registry& reg, entt::entity ent);
		void OnConstructAnimationGraphController(entt::registry& reg, entt::entity ent);
		void OnDestroyAnimationGraphController(entt::registry& reg, entt::entity ent);
		void OnConstructSequencePlayer(entt::registry& reg, entt::entity ent);
		void OnDestroySequencePlayer(entt::registry& reg, entt::entity ent);

		void enable_child_entity(Entity entity);
		void disable_child_entity(Entity entity);
		//NOTE: force_destroy to be set to true if client has recieved a destroy packet from the packet
		void destroy_entity(Entity entity, bool force_destroy = false);
		void duplicate_entity(Entity entity, Entity res);
		//INFO: called when an entity is to be create at runtime
		Entity instanciate_entity_net(Entity entity);
		bool can_destroy_entity(Entity entity);


		void OnConstructSkinnedModelRendererComponent(entt::registry& reg, entt::entity ent);
		void OnDestroySkinnedModelRendererComponent(entt::registry& reg, entt::entity ent);

		entt::registry m_registry;
		ScriptRegistry m_scriptRegistry;
		std::string m_name;
		void* m_physics3D = nullptr;
		std::unordered_map<UUID, Entity> m_entityMap;
		HierachyComponent* m_rootNode;
		bool m_running = false;
		std::vector<Entity> m_entityToDestroy;

	protected:
		friend class Entity;
		friend class TraceEditor;
		friend class HierachyPanel;
		friend class SceneSerializer;
		friend class InspectorPanel;
	};

}
