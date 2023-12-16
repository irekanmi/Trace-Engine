#include "pch.h"

#include "ScriptEngine.h"

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
		return true;
	}

	void ScriptEngine::Shutdown()
	{
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