#pragma once

#include "Scene.h"
#include "core/io/Logging.h"

namespace trace {

	class Entity
	{
		
	public:
		Entity()
			:m_handle(entt::null), m_scene(nullptr) {}

		Entity(entt::entity handle, Scene* scene)
			:m_handle(handle), m_scene(scene) {}

		~Entity() { m_handle = entt::null; m_scene = nullptr; }

		template<typename T>
		T& GetComponent()
		{
			TRC_ASSERT(HasComponent<T>(), __FUNCTION__);
			return m_scene->m_registry.get<T>(m_handle);
		}

		template<typename T,typename ...Args>
		T& AddComponent(Args ...args)
		{
			TRC_ASSERT(!HasComponent<T>(), __FUNCTION__);
			return m_scene->m_registry.emplace<T>(m_handle, std::forward<Args>(args)...);
		}

		template<typename T>
		void RemoveComponent()
		{
			TRC_ASSERT(HasComponent<T>(), __FUNCTION__);
			m_scene->m_registry.erase<T>(m_handle);
		}

		template<typename T>
		bool HasComponent()
		{
			//TODO: Check if there is another way to do this
			return m_scene->m_registry.try_get<T>(m_handle) != nullptr;
		}

		operator entt::entity() { return m_handle; }
		operator bool() { return m_handle == entt::null; }
		bool operator ==(Entity other) { return (m_handle == other.m_handle) && (m_scene == other.m_scene); }
		bool operator !=(Entity other) { return !(*this == other); }

	
	private:
		entt::entity m_handle;
		Scene* m_scene;

	protected:
		friend Scene;
	};

}