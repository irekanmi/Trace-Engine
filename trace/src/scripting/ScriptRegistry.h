#pragma once

#include "Script.h"
#include "scene/UUID.h"
#include <functional>

namespace trace {

	class ScriptRegistry
	{

	public:
		struct ScriptManager
		{
			std::vector<ScriptInstance> instances;
			std::vector<UUID> entities;
			std::unordered_map<UUID, size_t> handle_map;
			Script* script = nullptr;
		};


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
		void Iterate(std::function<void(ScriptManager&)> callback);

		void ReloadScripts();

		static void Copy(ScriptRegistry& from, ScriptRegistry& to);

		
	private:
		std::unordered_map<uintptr_t, ScriptManager> m_scripts;

	protected:
		friend class Scene;

	};

}
