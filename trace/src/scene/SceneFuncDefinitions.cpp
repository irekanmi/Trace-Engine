#include "pch.h"

#include "scene/Entity.h"
#include "scene/Scene.h"


namespace trace {

	template<typename... Component>
	void CopyComponent(entt::registry& from, entt::registry& to, std::unordered_map<UUID, entt::entity>& entity_map)
	{
		([&]()
			{
				auto view = from.view<Component>();
				for (auto e : view)
				{
					UUID uuid = from.get<IDComponent>(e)._id;
					auto [comp] = view.get(e);
					entt::entity en = entity_map[uuid];
					to.emplace_or_replace<Component>(en, comp);
				}

			}(), ...);

	}

	template<typename... Component>
	void CopyComponent(ComponentGroup<Component...>, entt::registry& from, entt::registry& to, std::unordered_map<UUID, entt::entity>& entity_map)
	{
		CopyComponent<Component...>(from, to, entity_map);
	}

	void Scene::Copy(Ref<Scene> from, Ref<Scene> to)
	{

		entt::registry& f_reg = from->m_registry;

		to->Destroy();
		to->Create();

		entt::registry& t_reg = to->m_registry;
		to->m_rootNode->children.clear();
		to->m_entityMap.clear();

		std::unordered_map<UUID, entt::entity> entity_map;

		auto id_view = f_reg.view<IDComponent>();
		for (auto e : id_view)
		{
			UUID uuid = id_view.get<IDComponent>(e)._id;
			Entity en = to->CreateEntity_UUID(uuid, "");
			entity_map[uuid] = en;
		}

		CopyComponent(AllComponents{}, f_reg, t_reg, entity_map);
		CopyComponent(ComponentGroup<HierachyComponent>{}, f_reg, t_reg, entity_map);

		auto skin_view = f_reg.view<SkinnedModelRenderer>();
		for (auto e : skin_view)
		{
			Entity from_entity = Entity(e, from.get());
			SkinnedModelRenderer& from_model = skin_view.get<SkinnedModelRenderer>(e);
			Entity to_entity = to->GetEntity(from_entity.GetID());
			SkinnedModelRenderer& to_model = to_entity.GetComponent<SkinnedModelRenderer>();

			to_model.SetSkeleton(from_model.GetSkeleton(), to.get(), to_entity.GetID());

		}

		/*auto graph_view = f_reg.view<AnimationGraphController>();
		for (auto e : graph_view)
		{
			Entity from_entity = Entity(e, from.get());
			AnimationGraphController& from_model = graph_view.get<AnimationGraphController>(e);
			Entity to_entity = to->GetEntity(from_entity.GetID());
			to_entity.RemoveComponent<AnimationGraphController>();
			to_entity.AddOrReplaceComponent<AnimationGraphController>(from_model);

		}*/

		auto sequence_view = f_reg.view<SequencePlayer>();
		for (auto e : sequence_view)
		{
			Entity from_entity = Entity(e, from.get());
			SequencePlayer& from_model = sequence_view.get<SequencePlayer>(e);
			Entity to_entity = to->GetEntity(from_entity.GetID());
			to_entity.RemoveComponent<SequencePlayer>();
			to_entity.AddOrReplaceComponent<SequencePlayer>(from_model);

		}


		ScriptRegistry::Copy(from->m_scriptRegistry, to->m_scriptRegistry);

		to->m_rootNode->children = from->m_rootNode->children;

		to->InitializeSceneComponents();

	}

}