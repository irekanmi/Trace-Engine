
#include "ContentBrowser.h"
#include "../TraceEditor.h"
#include "resource/TextureManager.h"
#include "backends/UIutils.h"
#include "imgui.h"

#include <unordered_map>
#include <functional>
#include <string>

namespace trace {

	static float thumbnail_size = 96.0f;
	static std::filesystem::path item_data[2048];
	static uint32_t item_index = 0;
	std::unordered_map<std::string, std::function<void(std::filesystem::path&)>> process_callbacks;
	std::unordered_map<std::string, std::function<void(std::filesystem::path&)>> item_callbacks;

	bool ContentBrowser::Init()
	{
		m_currentDir = m_editor->current_project_path;

		directory_icon = TextureManager::get_instance()->LoadTexture("directory.png");
		default_icon = TextureManager::get_instance()->LoadTexture("default_file_icon.png");

		{
			item_callbacks["directory"] = [&](std::filesystem::path& path)
			{
				std::string filename = path.filename().string();
				void* textureID = nullptr;
				UIFunc::GetDrawTextureHandle(directory_icon.get(), textureID);
				if (ImGui::ImageButton(filename.c_str(), textureID, { thumbnail_size, thumbnail_size }, { 0, 1 }, {1, 0}))
				{
					m_currentDir /= filename;
					OnDirectoryChanged();
				}
				ImGui::TextWrapped(filename.c_str());
			};

			item_callbacks["default"] = [&](std::filesystem::path& path)
			{
				std::string filename = path.filename().string();
				void* textureID = nullptr;
				UIFunc::GetDrawTextureHandle(default_icon.get(), textureID);
				if (ImGui::ImageButton(filename.c_str(), textureID, { thumbnail_size, thumbnail_size }, { 0, 1 }, { 1, 0 }))
				{

				}

				ImGui::TextWrapped(filename.c_str());
			};

			item_callbacks[".trscn"] = [&](std::filesystem::path& path)
			{
				std::string filename = path.filename().string();
				void* textureID = nullptr;
				UIFunc::GetDrawTextureHandle(default_icon.get(), textureID);
				if (ImGui::ImageButton(filename.c_str(), textureID, { thumbnail_size, thumbnail_size }, { 0, 1 }, { 1, 0 }))
				{

				}

				if (ImGui::BeginDragDropSource())
				{
					std::string res_path = path.string();
					res_path += "\0";
					ImGui::SetDragDropPayload(".trscn", res_path.c_str(), res_path.size());
					ImGui::EndDragDropSource();
				}
				ImGui::TextWrapped(filename.c_str());

			};
		};

		OnDirectoryChanged();
		return true;
	}
	void ContentBrowser::Shutdown()
	{
		// TODO: Add a data structure that holds and releases all textures
		directory_icon.release();
		default_icon.release();
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
		ImGui::SameLine(avail.x * 0.25f);
		ImGui::SliderFloat("##Thumbnail size", &thumbnail_size, 16.0f, 256.0f);

		float padding = 16.0f;
		float cell_size = thumbnail_size + padding;
		int column_count = (int)(avail.x / cell_size);
		if (column_count <= 0) column_count = 1;



		ImGui::Columns(column_count, nullptr, false);

		ImVec4* colors = ImGui::GetStyle().Colors;
		ImVec4 button_hov = colors[ImGuiCol_ButtonHovered];
		ImVec4 button_act = colors[ImGuiCol_ButtonActive];

		ImGui::PushStyleColor(ImGuiCol_Button, { 0.0f, 0.0f, 0.0f, 0.0f});
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { button_hov.x, button_hov.y, button_hov.z, 0.5f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, { button_act.x, button_act.y, button_act.z, 0.5f });

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

		ImGui::PopStyleColor(3);
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