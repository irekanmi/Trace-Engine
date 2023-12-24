#pragma once

#include "Script.h"
#include "scene/UUID.h"

namespace trace {

	using FieldManager = std::unordered_map<UUID, ScriptFieldInstance>;

	class Scene;

	class ScriptEngine
	{

	public:

		


		ScriptEngine();
		~ScriptEngine();

		bool Init();
		void Shutdown();
		bool LoadAssembly(const std::string& assembly_path);
		void ReloadAssembly();

		void OnSceneStart(Scene* scene);
		void OnSceneStop(Scene* scene);

		ScriptMethod* GetConstructor();

		std::unordered_map<std::string, Script>& GetScripts() { return m_scripts; }
		std::unordered_map<Script*, FieldManager>& GetFieldInstances() { return m_fieldInstance; }

		static ScriptEngine* get_instance();
	private:
		static ScriptEngine* s_instance;

		Script Action;
		ScriptMethod Action_Construtor;
		std::unordered_map<Script*, FieldManager> m_fieldInstance;


		bool get_dir_path();

		std::string bin_dir_path;
		std::unordered_map<std::string, Script> m_scripts;


	protected:

	};

}
