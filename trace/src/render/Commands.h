#pragma once

#include "core/Core.h"
#include "core/Enums.h"
#include <stdint.h>
#include <vector>
#include <functional>

namespace trace {

	struct CommandParams
	{
		void* ptrs[4];
		uint32_t val[4];
		char* data = nullptr;// NOTE: It uses frame allocation
	};

	using CommandFunc = std::function<void(CommandParams&)>;

	struct Command
	{
		CommandParams params;
		CommandFunc func;
	};

	struct CommandList
	{
		std::vector<Command> _commands;
	};

}