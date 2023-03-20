#pragma once

#include "core/Core.h"
#include "core/Enums.h"
#include <stdint.h>
#include <vector>
#include <functional>

namespace trace {

	struct CommandParams
	{
		uint32_t val[4];
		void* ptrs[4];
	};

	//typedef void (*CommandFunc)(CommandParams params);
	using CommandFunc = std::function<void(CommandParams)>;

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