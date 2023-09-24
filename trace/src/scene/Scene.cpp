#include "pch.h"

#include "Scene.h"
#include "Entity.h"

#include "Componets.h"

namespace trace {
	void Scene::OnCreate()
	{
	}
	void Scene::OnDestroy()
	{
		m_registry.clear();
	}
	Entity Scene::CreateEntity()
	{
		entt::entity handle = m_registry.create();
		Entity entity(handle, this);
		TagComponent& Tag = entity.AddComponent<TagComponent>();
		Tag.tag = "New Entity";

		return entity;
	}

	Entity Scene::CreateEntity(const std::string& tag)
	{
		entt::entity handle = m_registry.create();

		Entity entity(handle, this);
		entity.AddComponent<TagComponent>(tag);

		return entity;
	}

	void Scene::DestroyEntity(Entity entity)
	{
		m_registry.destroy(entity);
	}

}