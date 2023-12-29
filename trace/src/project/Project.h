#pragma once

#include "resource/Resource.h"

#include <string>

namespace trace {


	class Project : public Resource
	{

	public:

		std::string& GetName() { return m_name; }
		void SetName(const std::string& name) { m_name = name; }

		std::string assets_directory;
		std::string current_directory;
	private:
		std::string m_name;
	protected:

	};

}
