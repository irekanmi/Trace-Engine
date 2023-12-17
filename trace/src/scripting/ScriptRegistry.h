#pragma once

#include "Script.h"
#include "scene/UUID.h"
#include <functional>

namespace trace {

	class ScriptRegistry
	{

	public:
		ScriptRegistry();
		~ScriptRegistry();

		bool Init(uint32_t reserved_count = 64);
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

		void ReloadScripts();

		static void Copy(ScriptRegistry& from, ScriptRegistry& to);

		struct ScriptManager
		{
			std::vector<ScriptInstance> instances;
			std::unordered_map<UUID, size_t> handle_map;
			Script* script = nullptr;
		};
	private:
		std::unordered_map<uintptr_t, ScriptManager> m_scripts;

	protected:

	};

}
