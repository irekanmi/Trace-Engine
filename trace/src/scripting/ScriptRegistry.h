#pragma once

#include "Script.h"
#include "scene/UUID.h"
#include "multithreading/SpinLock.h"

#include <functional>

namespace trace {
	using FieldManager = std::unordered_map<UUID, ScriptFieldInstance>;

	class ScriptRegistry
	{

	public:
		struct ScriptManager
		{
			std::vector<ScriptInstance> instances;
			std::vector<UUID> entities;
			std::unordered_map<UUID, size_t> handle_map;
			Script* script = nullptr;
			SpinLock lock;
		};


		ScriptRegistry();
		~ScriptRegistry();

		bool Init(Scene* scene, uint32_t reserved_count = 64);
		void Clear();

		bool HasScript(UUID id, const std::string& script_name);
		bool HasScript(UUID id, uintptr_t handle);

		//NOTE: The pointer should most probably be as a one time use
		ScriptInstance* GetScript(UUID id, const std::string& script_name);
		ScriptInstance* GetScript(UUID id, uintptr_t handle);

		//NOTE: The pointer should most probably be as a one time use
		ScriptInstance* AddScript(UUID id, const std::string& script_name);
		ScriptInstance* AddScript(UUID id, uintptr_t handle);

		bool RemoveScript(UUID id, const std::string& script_name);
		bool RemoveScript(UUID id, uintptr_t handle);

		bool Erase(UUID id);
		void Iterate(UUID id, std::function<void(UUID, Script*, ScriptInstance*)> callback, bool has_script = true);
		void Iterate(uintptr_t script_handle, std::function<bool(UUID, Script*, ScriptInstance*)> callback);
		void Iterate(std::function<void(ScriptManager&)> callback);

		void ReloadScripts();
		std::unordered_map<Script*, FieldManager>& GetFieldInstances() { return m_fieldInstance; }
		ScriptInstance* CopyScriptInstance(UUID id, ScriptInstance* source);

		static void Copy(ScriptRegistry& from, ScriptRegistry& to);

	private:
		ScriptInstance* add_new_instance(UUID id, uintptr_t script_id);
		
	private:
		std::unordered_map<uintptr_t, ScriptManager> m_scripts;
		std::unordered_map<Script*, FieldManager> m_fieldInstance;
		Scene* m_scene = nullptr;
		

	protected:
		friend class Scene;
		friend class SceneSerializer;

	};

}
