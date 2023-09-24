#pragma once

#include <string>

namespace trace {

	struct TagComponent
	{
		std::string tag;

		TagComponent() = default;
		TagComponent(const std::string& name) { tag = name; }
		TagComponent(std::string& name) { tag = name; }

	};

}
