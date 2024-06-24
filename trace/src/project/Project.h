#pragma once

#include "resource/Resource.h"
#include "scene/UUID.h"

#include <string>

namespace trace {

	class Project : public Resource
	{

	public:

		std::string& GetProjectName() { return m_name; }
		UUID GetStartScene() { return m_startScene; }
		void SetName(const std::string& name) { m_name = name; }
		void SetStartScene(UUID id) { m_startScene = id; }

		std::string GetAssetsDirectory() { return m_assetsDirectory; }
		std::string GetAssemblyPath() { return m_assemblyPath; }
		std::string GetProjectCurrentDirectory() { return m_currentDirectory; }
		
		void SetAssetsDirectory(const std::string& file_path) { m_assetsDirectory = file_path; }
		void SetAssemblyPath(const std::string& file_path) { m_assemblyPath = file_path; }
		void SetProjectCurrentDirectory(const std::string& file_path) { m_currentDirectory = file_path; }

	private:
		std::string m_assetsDirectory;
		std::string m_assemblyPath;
		std::string m_currentDirectory;
		std::string m_name;
		UUID m_startScene;

	protected:

	};

}
