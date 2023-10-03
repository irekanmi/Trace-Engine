#pragma once

#include <filesystem>
#include <vector>

namespace trace {

	class TraceEditor;

	class ContentBrowser
	{

	public:

		bool Init();
		void Shutdown();
		void Render(float deltaTime);
		void OnDirectoryChanged();
		void ProcessDirectory();

	private:
		std::filesystem::path m_currentDir;
		std::vector<std::filesystem::path> m_dirContents;
		std::vector<std::string> m_dirItems;
		

		TraceEditor* m_editor;
	protected:
		friend class TraceEditor;

	};

}
