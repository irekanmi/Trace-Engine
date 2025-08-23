#include "pch.h"

#include "Entity.h"
#include "Components.h"


namespace trace {
	UUID Entity::GetID()
	{
		return GetComponent<IDComponent>()._id;
	}
	std::string& Entity::GetName()
	{
		return GetComponent<TagComponent>().GetTag();
	}
	bool Entity::IsOwner()
	{
		if (NetObject* net = TryGetComponent<NetObject>())
		{
			return net->is_owner;
		}

		return true;
	}
	uint32_t Entity::GetNetID()
	{
		if (NetObject* net = TryGetComponent<NetObject>())
		{
			return net->owner_id;
		}

		return 0;
	}
	bool Entity::HasScript(const std::string& script_name)
	{
		return m_scene->m_scriptRegistry.HasScript(GetID(), script_name);
	}
	bool Entity::HasScript(uintptr_t handle)
	{
		return m_scene->m_scriptRegistry.HasScript(GetID(), handle);
	}
	ScriptInstance* Entity::GetScript(const std::string& script_name)
	{
		return m_scene->m_scriptRegistry.GetScript(GetID(), script_name);
	}
	ScriptInstance* Entity::GetScript(uintptr_t handle)
	{
		return m_scene->m_scriptRegistry.GetScript(GetID(), handle);
	}
	ScriptInstance* Entity::AddScript(const std::string& script_name)
	{
		return m_scene->m_scriptRegistry.AddScript(GetID(), script_name);;
	}
	ScriptInstance* Entity::AddScript(uintptr_t handle)
	{
		return m_scene->m_scriptRegistry.AddScript(GetID(), handle);
	}
	bool Entity::RemoveScript(const std::string& script_name)
	{
		return m_scene->m_scriptRegistry.RemoveScript(GetID(), script_name);;
	}
	bool Entity::RemoveScript(uintptr_t handle)
	{
		return m_scene->m_scriptRegistry.RemoveScript(GetID(), handle);
	}
	bool Entity::HasParent()
	{
		return GetComponent<HierachyComponent>().HasParent();
	}
	Entity Entity::GetParent()
	{
		if (m_scene && HasParent())
		{
			return m_scene->GetEntity(GetComponent<HierachyComponent>().parent);
		}

		return Entity();
	}
	UUID Entity::GetParentID()
	{
		return GetComponent<HierachyComponent>().parent;
	}
}