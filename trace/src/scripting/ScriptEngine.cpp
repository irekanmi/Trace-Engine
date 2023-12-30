#include "pch.h"

#include "ScriptEngine.h"
#include "ScriptBackend.h"
#include "core/Coretypes.h"
#include "core/Utils.h"
#include <filesystem>

namespace trace {

	ScriptEngine* ScriptEngine::s_instance = nullptr;

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
		LoadComponents();

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
		LoadComponents();

		ReloadFieldInstances();

		return true;
	}


	void ScriptEngine::OnSceneStart(Scene* scene)
	{
		OnSceneStartInternal(scene);
	}

	void ScriptEngine::OnSceneStop(Scene* scene)
	{
		OnSceneStopInternal(scene);
	}

	ScriptMethod* ScriptEngine::GetConstructor()
	{
		GetScriptMethod(".ctor", Action_Construtor, Action, 1);

		return &Action_Construtor;
	}

	ScriptEngine* ScriptEngine::get_instance()
	{
		if (!s_instance)
		{
			s_instance = new ScriptEngine();
		}
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