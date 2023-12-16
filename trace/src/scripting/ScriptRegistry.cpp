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
		}

		return true;
	}

	bool ScriptRegistry::HasScript(uint64_t id, const std::string& script_name)
	{
		std::unordered_map<std::string, Script>& g_Scripts = ScriptEngine::get_instance()->GetScripts();
		auto s_it = g_Scripts.find(script_name);
		if (s_it == g_Scripts.end()) return false;
		uintptr_t handle = s_it->second.GetID();
		auto it = m_scripts.find(handle);
		if (it != m_scripts.end())
		{
			ScriptManager& sm = it->second;
			auto in_it = sm.handle_map.find(id);
			if (in_it != sm.handle_map.end()) return true;
		}

		return false;
	}

	bool ScriptRegistry::HasScript(uint64_t id, uintptr_t handle)
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

	ScriptInstance* ScriptRegistry::GetScript(uint64_t id, const std::string& script_name)
	{
		std::unordered_map<std::string, Script>& g_Scripts = ScriptEngine::get_instance()->GetScripts();
		auto s_it = g_Scripts.find(script_name);
		if (s_it == g_Scripts.end()) return nullptr;
		uintptr_t handle = s_it->second.GetID();
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

	ScriptInstance* ScriptRegistry::GetScript(uint64_t id, uintptr_t handle)
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

	ScriptInstance* ScriptRegistry::AddScript(uint64_t id, const std::string& script_name)
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
		auto it = m_scripts.find(handle);
		if (it != m_scripts.end())
		{
			ScriptManager& sm = it->second;
			sm.handle_map[id] = sm.instances.size();
			ScriptInstance& res = sm.instances.emplace_back();
			res.m_script = sm.script;
			return &sm.instances.back();
		}

		return nullptr;
	}

	ScriptInstance* ScriptRegistry::AddScript(uint64_t id, uintptr_t handle)
	{
		if (HasScript(id, handle)) return GetScript(id, handle);

		auto it = m_scripts.find(handle);
		if (it != m_scripts.end())
		{
			ScriptManager& sm = it->second;
			sm.handle_map[id] = sm.instances.size();
			ScriptInstance& res = sm.instances.emplace_back();
			res.m_script = sm.script;
			return &sm.instances.back();
		}

		return nullptr;
	}

	bool ScriptRegistry::RemoveScript(uint64_t id, const std::string& script_name)
	{
		std::unordered_map<std::string, Script>& g_Scripts = ScriptEngine::get_instance()->GetScripts();
		auto s_it = g_Scripts.find(script_name);
		if (s_it == g_Scripts.end())
		{
			TRC_WARN("Script {} does not exist", script_name);
			return false;
		}
		uintptr_t handle = s_it->second.GetID();
		auto it = m_scripts.find(handle);
		if (it != m_scripts.end())
		{
			ScriptManager& sm = it->second;

			sm.instances[sm.handle_map[id]] = std::move(sm.instances.back());

			sm.instances.pop_back();
			sm.handle_map.erase(id);

			return true;
		}

		return false;
	}

	bool ScriptRegistry::RemoveScript(uint64_t id, uintptr_t handle)
	{
		auto it = m_scripts.find(handle);
		if (it != m_scripts.end())
		{
			ScriptManager& sm = it->second;

			sm.instances[sm.handle_map[id]] = std::move(sm.instances.back());

			sm.instances.pop_back();
			sm.handle_map.erase(id);

			return true;
		}

		return false;
	}

	bool ScriptRegistry::Erase(uint64_t id)
	{
		for (auto& i : m_scripts)
		{
			RemoveScript(id, i.first);
		}

		return true;
	}

	void ScriptRegistry::Copy(ScriptRegistry& from, ScriptRegistry& to)
	{
		to.m_scripts.clear();

		for (auto& i : from.m_scripts)
		{
			to.m_scripts[i.first].handle_map = i.second.handle_map;
			to.m_scripts[i.first].instances = i.second.instances;
			to.m_scripts[i.first].script = i.second.script;
		}
	}

}
