#pragma once

#include "resource/Resource.h"

#include <string>

namespace trace {


	class Project : public Resource
	{

	public:

		std::string& GetName() { return m_name; }
		std::string& GetStartScene() { return m_startScene; }
		void SetName(const std::string& name) { m_name = name; }
		void SetStartScene(const std::string& name) { m_startScene = name; }

		std::string assets_directory;
		std::string assembly_path;
		std::string current_directory;
	private:
		std::string m_name;
		std::string m_startScene;

	protected:

	};

}
