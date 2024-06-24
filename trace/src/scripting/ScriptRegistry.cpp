#include "pch.h"

#include "ScriptRegistry.h"
#include "ScriptEngine.h"
#include "core/io/Logging.h"
#include <iostream>

namespace trace {


	ScriptRegistry::ScriptRegistry()
	{
	}

	ScriptRegistry::~ScriptRegistry()
	{
	}

	bool ScriptRegistry::Init(uint32_t reserved_count)
	{

		for (auto& i : ScriptEngine::get_instance()->GetScripts())
		{
			m_scripts[i.second.GetID()].script = &i.second;
			m_scripts[i.second.GetID()].instances.reserve(reserved_count);
			m_scripts[i.second.GetID()].entities.reserve(reserved_count);
		}

		return true;
	}

	void ScriptRegistry::Clear()
	{
		m_fieldInstance.clear();
		m_scripts.clear();
	}

	bool ScriptRegistry::HasScript(UUID id, const std::string& script_name)
	{
		std::unordered_map<std::string, Script>& g_Scripts = ScriptEngine::get_instance()->GetScripts();
		auto s_it = g_Scripts.find(script_name);
		if (s_it == g_Scripts.end()) return false;
		uintptr_t handle = s_it->second.GetID();
		return HasScript(id, handle);
	}

	bool ScriptRegistry::HasScript(UUID id, uintptr_t handle)
	{
		auto it = m_scripts.find(handle);
		if (it != m_scripts.end())
		{
			ScriptManager& sm = it->second;
			auto in_it = sm.handle_map.find(id);
			if (in_it != sm.handle_map.end()) return true;
		}

		return false;
	}

	ScriptInstance* ScriptRegistry::GetScript(UUID id, const std::string& script_name)
	{
		std::unordered_map<std::string, Script>& g_Scripts = ScriptEngine::get_instance()->GetScripts();
		auto s_it = g_Scripts.find(script_name);
		if (s_it == g_Scripts.end()) return nullptr;
		uintptr_t handle = s_it->second.GetID();
		return GetScript(id, handle);
	}

	ScriptInstance* ScriptRegistry::GetScript(UUID id, uintptr_t handle)
	{
		auto it = m_scripts.find(handle);
		if (it != m_scripts.end())
		{
			ScriptManager& sm = it->second;
			auto in_it = sm.handle_map.find(id);
			if (in_it != sm.handle_map.end())
			{
				return &sm.instances[in_it->second];
			}
		}

		return nullptr;
	}

	ScriptInstance* ScriptRegistry::AddScript(UUID id, const std::string& script_name)
	{
		if (HasScript(id, script_name)) return GetScript(id, script_name);

		std::unordered_map<std::string, Script>& g_Scripts = ScriptEngine::get_instance()->GetScripts();

		auto s_it = g_Scripts.find(script_name);
		if (s_it == g_Scripts.end())
		{
			std::cout << "Script {" << script_name << "} does not exist" << std::endl;
			return nullptr;
		}
		uintptr_t handle = s_it->second.GetID();
		return AddScript(id, handle);
	}

	ScriptInstance* ScriptRegistry::AddScript(UUID id, uintptr_t handle)
	{
		if (HasScript(id, handle)) return GetScript(id, handle);

		auto it = m_scripts.find(handle);
		if (it != m_scripts.end())
		{
			ScriptManager& sm = it->second;
			sm.handle_map[id] = sm.instances.size();
			sm.entities.emplace_back(id);
			ScriptInstance& res = sm.instances.emplace_back();
			res.m_script = sm.script;
			return &sm.instances.back();
		}

		return nullptr;
	}

	bool ScriptRegistry::RemoveScript(UUID id, const std::string& script_name)
	{
		std::unordered_map<std::string, Script>& g_Scripts = ScriptEngine::get_instance()->GetScripts();
		auto s_it = g_Scripts.find(script_name);
		if (s_it == g_Scripts.end())
		{
			TRC_WARN("Script {} does not exist", script_name);
			return false;
		}
		uintptr_t handle = s_it->second.GetID();
		return RemoveScript(id, handle);
	}

	bool ScriptRegistry::RemoveScript(UUID id, uintptr_t handle)
	{
		auto it = m_scripts.find(handle);
		if (it != m_scripts.end())
		{
			ScriptManager& sm = it->second;

			sm.instances[sm.handle_map[id]] = std::move(sm.instances.back());
			sm.entities[sm.handle_map[id]] = std::move(sm.entities.back());

			sm.instances.pop_back();
			sm.entities.pop_back();
			sm.handle_map.erase(id);

			return true;
		}

		return false;
	}

	bool ScriptRegistry::Erase(UUID id)
	{
		for (auto& i : m_scripts)
		{
			if(HasScript(id, i.first))
				RemoveScript(id, i.first);
		}

		return true;
	}

	void ScriptRegistry::Iterate(UUID id, std::function<void(UUID, Script*, ScriptInstance*)> callback, bool has_script)
	{

		for (auto& i : m_scripts)
		{
			auto it = i.second.handle_map.find(id);
			if (has_script)
			{
				if (it != i.second.handle_map.end())
				{
					callback(id, i.second.script, &i.second.instances[it->second]);
				}
			}
			else callback(id, i.second.script, nullptr);
		}

	}

	void ScriptRegistry::Iterate(std::function<void(ScriptManager&)> callback)
	{

		for (auto& i : m_scripts)
		{
			callback(i.second);
		}

	}

	void ScriptRegistry::ReloadScripts()
	{
		std::unordered_map<uintptr_t, ScriptManager> new_scripts;
		for (auto& i : ScriptEngine::get_instance()->GetScripts())
		{
			bool found = false;
			for (auto& j : m_scripts)
			{
				if (j.second.script->GetScriptName() == i.second.GetScriptName())
				{
					new_scripts[i.second.GetID()] = j.second;
					found = true;
				}
			}
			if(!found) new_scripts[i.second.GetID()].script = &i.second;
		}

		m_scripts = std::move(new_scripts);


		for (auto& i : m_fieldInstance)
		{
			for (auto& j : i.second)
			{
				j.second.Reload();
			}
		}


	}

	void ScriptRegistry::Copy(ScriptRegistry& from, ScriptRegistry& to)
	{
		to.m_scripts.clear();

		for (auto& i : from.m_scripts)
		{
			to.m_scripts[i.first].handle_map = i.second.handle_map;
			to.m_scripts[i.first].instances = i.second.instances;
			to.m_scripts[i.first].entities = i.second.entities;
			to.m_scripts[i.first].script = i.second.script;
		}

		to.m_fieldInstance = from.m_fieldInstance;
	}

}
