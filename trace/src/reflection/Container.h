#pragma once

#include "TypeHash.h"

namespace trace::Reflection {

	struct Container
	{
		uint64_t key_type_id{};
		uint64_t value_type_id{};
	};

}