#include "pch.h"

#include "ScriptEngine.h"
#include "ScriptBackend.h"
#include "core/Coretypes.h"
#include "core/Utils.h"
#include "resource/PrefabManager.h"

#include <filesystem>

namespace trace {


	ScriptEngine::ScriptEngine()
	{

	}

	ScriptEngine::~ScriptEngine()
	{
	}

	bool ScriptEngine::Init()
	{
		bool result = true;
		result = result && FindDirectory(AppSettings::exe_path, "Data", bin_dir_path);
		result = result && InitializeInternal(bin_dir_path);

		bin_file_path = bin_dir_path + "/Assembly/TraceScriptLib.dll";
		result = result && LoadCoreAssembly(bin_file_path);

		//Temp -----------------------------
		LoadAllScripts(m_scripts);
		//----------------------------------

		CreateScript("Action", Action, "Trace", true);
		CreateScript("Network", Network, "Trace", true);
		LoadComponents();

		// BuiltIn Types ======================
		CreateScript("TriggerPair", m_builtTypes["TriggerPair"], "Trace", true);

		// ====================================

		return result;
	}

	void ScriptEngine::Shutdown()
	{
		m_fieldInstance.clear();
		for (auto& i : m_scripts)
		{
			DestroyScript(i.second);
		}
		ShutdownInternal();
	}

	bool ScriptEngine::ReloadAssembly(const std::string& assembly_path)
	{
		ReloadAssemblies(bin_file_path, assembly_path);

		//Temp -----------------------------
		LoadAllScripts(m_scripts);
		//----------------------------------

		CreateScript("Action", Action, "Trace", true);
		CreateScript("Network", Network, "Trace", true);
		LoadComponents();

		ReloadFieldInstances();

		// BuiltIn Types ======================
		CreateScript("TriggerPair", m_builtTypes["TriggerPair"], "Trace", true);

		// ====================================

		// Update prefab scene
		PrefabManager::get_instance()->GetScene()->GetScriptRegistry().ReloadScripts();

		return true;
	}


	void ScriptEngine::OnSceneStart(Scene* scene)
	{
		m_currentScene = scene;
		OnSceneStartInternal(scene);
	}

	void ScriptEngine::OnSceneStop(Scene* scene)
	{
		OnSceneStopInternal(scene);
		for (auto [id, ins] : m_entityActionInstance)
		{
			DestroyScriptInstance(ins);
		}
		m_entityActionInstance.clear();
		m_currentScene = nullptr;
	}

	ScriptMethod* ScriptEngine::GetConstructor()
	{
		GetScriptMethod(".ctor", Action_Construtor, Action, 1);

		return &Action_Construtor;
	}

	Script* ScriptEngine::GetBuiltInScript(const std::string script_name)
	{
		auto it = m_builtTypes.find(script_name);
		if (it != m_builtTypes.end()) return &it->second;

		return nullptr;
	}
	
	Script* ScriptEngine::GetScript(const std::string script_name)
	{
		auto it = m_scripts.find(script_name);
		if (it != m_scripts.end()) return &it->second;

		return nullptr;
	}

	ScriptField& ScriptEngine::GetIDField()
	{
		return Action.GetFields()["Id"];
	}

	ScriptInstance* ScriptEngine::GetEntityActionClass(UUID entity_id)
	{
		if (!m_currentScene)
		{
			return nullptr;
		}
		auto it = m_entityActionInstance.find(entity_id);

		if (it == m_entityActionInstance.end())
		{
			CreateScriptInstance(Action, m_entityActionInstance[entity_id]);

			m_entityActionInstance[entity_id].SetFieldValue("Id", entity_id);
		}

		return &m_entityActionInstance.at(entity_id);
	}

	void ScriptEngine::RemoveEnityActionClass(UUID entity_id)
	{
		if (!m_currentScene)
		{
			return;
		}
		auto it = m_entityActionInstance.find(entity_id);

		if (it != m_entityActionInstance.end())
		{
			
			m_entityActionInstance.erase(entity_id);
		}
	}

	ScriptEngine* ScriptEngine::get_instance()
	{
		static ScriptEngine* s_instance = new ScriptEngine;
		return s_instance;
	}

	void ScriptEngine::ReloadFieldInstances()
	{
		for (auto& i : m_fieldInstance)
		{
			for (auto& j : i.second)
			{
				j.second.Reload();
			}
		}
	}


}