#pragma once

#include "Script.h"

namespace trace {

	class ScriptEngine
	{

	public:
		ScriptEngine();
		~ScriptEngine();

		bool Init();
		void Shutdown();
		bool LoadAssembly(const std::string& assembly_path);
		void ReloadAssembly();

		std::unordered_map<std::string, Script>& GetScripts() { return m_scripts; }

		static ScriptEngine* get_instance();
	private:
		static ScriptEngine* s_instance;

		bool get_dir_path();

		std::string bin_dir_path;
		std::unordered_map<std::string, Script> m_scripts;


	protected:

	};

}
