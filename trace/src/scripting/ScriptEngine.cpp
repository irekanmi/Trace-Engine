#include "pch.h"

#include "ScriptEngine.h"
#include "ScriptBackend.h"
#include "core/Coretypes.h"
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
		result = result && get_dir_path();
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

	bool ScriptEngine::get_dir_path()
	{
		std::filesystem::path current_dir = std::filesystem::path(AppSettings::exe_path).parent_path();


		while (current_dir != "")
		{
			if (std::filesystem::exists(current_dir / "Data"))
			{
				bin_dir_path = (current_dir / "Data").string();
				return true;
			}

			current_dir = current_dir.parent_path();
		}

		return false;
	}

}