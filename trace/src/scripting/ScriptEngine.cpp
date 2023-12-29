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

		result = result && LoadCoreAssembly(bin_dir_path + "/Assembly/TraceScriptLib.dll");

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

	bool ScriptEngine::LoadAssembly(const std::string& assembly_path)
	{


		return true;
	}

	void ScriptEngine::ReloadAssembly()
	{
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


}