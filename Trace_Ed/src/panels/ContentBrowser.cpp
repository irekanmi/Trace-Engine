
#include "ContentBrowser.h"
#include "../TraceEditor.h"
#include "imgui.h"

#include <unordered_map>
#include <functional>
#include <string>

namespace trace {

	static float thumbnail_size = 128.0f;
	static std::filesystem::path item_data[2048];
	static uint32_t item_index = 0;
	std::unordered_map<std::string, std::function<void(std::filesystem::path&)>> process_callbacks;
	std::unordered_map<std::string, std::function<void(std::filesystem::path&)>> item_callbacks;

	bool ContentBrowser::Init()
	{
		m_currentDir = m_editor->current_project_path;



		{
			item_callbacks["directory"] = [&](std::filesystem::path& path)
			{
				std::string filename = path.filename().string();
				if (ImGui::Button(filename.c_str()))
				{
					m_currentDir /= filename;
					OnDirectoryChanged();
				}
			};

			item_callbacks["default"] = [&](std::filesystem::path& path)
			{
				std::string filename = path.filename().string();
				ImGui::Text(filename.c_str());
			};

			item_callbacks[".trscn"] = [&](std::filesystem::path& path)
			{
				std::string filename = path.filename().string();
				ImGui::Button(filename.c_str());

				if (ImGui::BeginDragDropSource())
				{
					std::string res_path = path.string();
					ImGui::SetDragDropPayload(".trscn", res_path.c_str(), res_path.size());
					ImGui::EndDragDropSource();
				}


			};
		};

		OnDirectoryChanged();
		return true;
	}
	void ContentBrowser::Shutdown()
	{
	}
	void ContentBrowser::Render(float deltaTime)
	{
		ImGui::Begin("Content Browser");

		if (ImGui::ArrowButton("<<", ImGuiDir_Left))
		{
			if (m_currentDir != m_editor->current_project_path)
			{
				m_currentDir = m_currentDir.parent_path();
				OnDirectoryChanged();
			}
		}
		ImVec2 avail = ImGui::GetContentRegionAvail();
		ImGui::SameLine(avail.x * 0.25);
		ImGui::SliderFloat("##Thumbnail size", &thumbnail_size, 16.0f, 256.0f);

		float padding = 16.0f;
		float cell_size = thumbnail_size + padding;
		int column_count = (int)(avail.x / cell_size);
		if (column_count <= 0) column_count = 1;

		ImGui::Columns(column_count, nullptr, false);

		for (uint32_t i = 0; i < m_dirItems.size(); i++)
		{
			auto& path = item_data[i];
			std::string& item_id = m_dirItems[i];
			if (item_callbacks[item_id])
				item_callbacks[item_id](path);
			else
				item_callbacks["default"](path);

			ImGui::NextColumn();
		}

		ImGui::Columns(1);

		ImGui::End();

	}
	void ContentBrowser::OnDirectoryChanged()
	{
		m_dirContents.clear();
		for (auto p : std::filesystem::directory_iterator(m_currentDir))
		{
			m_dirContents.emplace_back(p.path());
		}
		ProcessDirectory();

	}
	void ContentBrowser::ProcessDirectory()
	{
		m_dirItems.clear();
		item_index = 0;
		for (uint32_t i = 0; i < m_dirContents.size(); i++)
		{
			std::filesystem::path& path = m_dirContents[i];
			
			if (std::filesystem::is_directory(path))
			{
				m_dirItems.emplace_back("directory");
				item_data[item_index] = path;
				item_index++;
			}
			else if (std::filesystem::is_regular_file(path))
			{
				std::string ext = path.extension().string();
				if (process_callbacks[ext])
				{
					process_callbacks[ext](path);
				}
				else
				{
					m_dirItems.emplace_back(ext);
					item_data[item_index] = path;
					item_index++;
				}
			}

		}

	}
}