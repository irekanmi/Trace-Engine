#pragma once

#include "Script.h"

namespace trace {

	class ScriptRegistry
	{

	public:
		ScriptRegistry();
		~ScriptRegistry();

		bool Init(uint32_t reserved_count = 64);

		bool HasScript(uint64_t id, const std::string& script_name);
		bool HasScript(uint64_t id, uintptr_t handle);

		//NOTE: The pointer should most probably be as a one time use
		ScriptInstance* GetScript(uint64_t id, const std::string& script_name);
		ScriptInstance* GetScript(uint64_t id, uintptr_t handle);

		//NOTE: The pointer should most probably be as a one time use
		ScriptInstance* AddScript(uint64_t id, const std::string& script_name);
		ScriptInstance* AddScript(uint64_t id, uintptr_t handle);

		bool RemoveScript(uint64_t id, const std::string& script_name);
		bool RemoveScript(uint64_t id, uintptr_t handle);

		bool Erase(uint64_t id);

		static void Copy(ScriptRegistry& from, ScriptRegistry& to);

		struct ScriptManager
		{
			std::vector<ScriptInstance> instances;
			std::unordered_map<uint64_t, size_t> handle_map;
			Script* script = nullptr;
		};
	private:
		std::unordered_map<uintptr_t, ScriptManager> m_scripts;

	protected:

	};

}
