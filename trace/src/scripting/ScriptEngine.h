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
		bool ReloadAssembly(const std::string& assembly_path);

		void OnSceneStart(Scene* scene);
		void OnSceneStop(Scene* scene);

		ScriptMethod* GetConstructor();

		std::unordered_map<std::string, Script>& GetScripts() { return m_scripts; }
		std::unordered_map<Script*, FieldManager>& GetFieldInstances() { return m_fieldInstance; }
		Scene* GetCurrentScene() { return m_currentScene; }
		Script* GetBuiltInScript(const std::string script_name);

		ScriptInstance* GetEntityActionClass(UUID entity_id);

		static ScriptEngine* get_instance();
	private:
		void ReloadFieldInstances();

		static ScriptEngine* s_instance;

		Script Action;
		ScriptMethod Action_Construtor;
		std::unordered_map<Script*, FieldManager> m_fieldInstance;
		Scene* m_currentScene = nullptr;



		std::string bin_dir_path;
		std::string bin_file_path;
		std::unordered_map<std::string, Script> m_scripts;
		std::unordered_map<std::string, Script> m_builtTypes;


		std::unordered_map<UUID, ScriptInstance> m_entityActionInstance;


	protected:

	};

}
