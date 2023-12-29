#include "pch.h"

#include "Utils.h"
#include "core/Coretypes.h"

#include <filesystem>

namespace trace {



	bool FindDirectory(const std::string& from, const std::string& dir, std::string& result)
	{
		std::filesystem::path current_dir = std::filesystem::path(from);


		while (current_dir != "")
		{
			if (std::filesystem::exists(current_dir / dir))
			{
				result = (current_dir / dir).generic_string();
				return true;
			}

			current_dir = current_dir.parent_path();
		}

		return false;
	}

}