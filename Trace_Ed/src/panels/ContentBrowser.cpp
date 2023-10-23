
#include "ContentBrowser.h"
#include "../TraceEditor.h"
#include "resource/TextureManager.h"
#include "resource/MeshManager.h"
#include "backends/UIutils.h"
#include "scene/UUID.h"
#include "imgui.h"

#include "serialize/yaml_util.h"
#include <unordered_map>
#include <functional>
#include <string>

namespace trace {

	static float thumbnail_size = 96.0f;
	static std::filesystem::path item_data[2048];
	static uint32_t item_index = 0;
	std::unordered_map<std::string, std::function<void(std::filesystem::path&)>> process_callbacks;
	std::unordered_map<std::string, std::function<void(std::filesystem::path&)>> item_callbacks;
	std::unordered_map<std::string, std::function<void(YAML::Emitter& ,std::filesystem::path&)>> extensions_callbacks;

	bool ContentBrowser::Init()
	{
		m_currentDir = m_editor->current_project_path;

		directory_icon = TextureManager::get_instance()->LoadTexture("directory.png");
		default_icon = TextureManager::get_instance()->LoadTexture("default_file_icon.png");

		//Items callbacks
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
					ImGui::SetDragDropPayload(".trscn", res_path.c_str(), res_path.size() + 1);
					ImGui::EndDragDropSource();
				}
				ImGui::TextWrapped(filename.c_str());

			};
		};

		//Process callbacks
		{
			process_callbacks[".obj"] = [&](std::filesystem::path& path) 
			{
				Ref<Mesh> mesh = MeshManager::get_instance()->LoadMesh_(path.string());
				for (auto i : mesh->GetModels())
				{
					m_editor->all_assets.models.emplace(i->m_path);
				}
			};

			auto tex_lambda = [&](std::filesystem::path& path)
			{
				m_editor->all_assets.textures.emplace(path);
			};

			process_callbacks[".png"] = tex_lambda;
			process_callbacks[".jpg"] = tex_lambda;
			process_callbacks[".tga"] = tex_lambda;
		};

		//extensions callbacks
		{

			extensions_callbacks["default"] = [](YAML::Emitter& emit, std::filesystem::path& path)
			{
				emit << YAML::Key << "Meta Type" << YAML::Value << path.extension().string();

			};

		};

		OnDirectoryChanged();
		ProcessAllDirectory();
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
	void ContentBrowser::ProcessAllDirectory()
	{
		all_files_id.clear();
		all_id_path.clear();
		std::filesystem::path db_path = m_editor->current_project_path / "InternalAssetsDB";
		std::filesystem::path assetsDB_path = db_path / "assets.trdb";
		if (!std::filesystem::exists(assetsDB_path))
		{
			if(!std::filesystem::exists(db_path)) std::filesystem::create_directory(db_path);

			YAML::Emitter emit;

			emit << YAML::BeginMap;
			emit << YAML::Key << "Trace Version" << YAML::Value << "0.0.0.0";
			emit << YAML::Key << "DataBase Version" << YAML::Value << "0.0.0.0";
			emit << YAML::Key << "DataBase Type" << YAML::Value << "Assets";
			emit << YAML::Key << "DATA" << YAML::Value << YAML::BeginSeq;
			for (auto p : std::filesystem::directory_iterator(m_editor->current_project_path))
			{
				std::filesystem::path path = p.path();
				if (path == db_path) continue;
				if (std::filesystem::is_directory(path))
				{
					for (auto r : std::filesystem::recursive_directory_iterator(path))
					{
						if (std::filesystem::is_regular_file(r.path()))
						{
							std::string filename = r.path().filename().string();
							emit << YAML::BeginMap;
							emit << YAML::Key << "Name" << YAML::Value << filename;
							emit << YAML::Key << "UUID" << YAML::Value << UUID::GenUUID();

							emit << YAML::EndMap;
						}
					}
				}
				else if (std::filesystem::is_regular_file(path))
				{
					std::string filename = path.filename().string();
					emit << YAML::BeginMap;
					emit << YAML::Key << "Name" << YAML::Value << filename;
					emit << YAML::Key << "UUID" << YAML::Value << UUID::GenUUID();

					emit << YAML::EndMap;
				}
			}

			emit << YAML::EndSeq;
			emit << YAML::EndMap;

			FileHandle out_handle;
			if (FileSystem::open_file(assetsDB_path.string(), FileMode::WRITE, out_handle))
			{
				FileSystem::writestring(out_handle, emit.c_str());
				FileSystem::close_file(out_handle);
			}
			

		}
		if (std::filesystem::exists(assetsDB_path))
		{
			FileHandle in_handle;
			if (!FileSystem::open_file(assetsDB_path.string(), FileMode::READ, in_handle))
			{
				TRC_ERROR("Unable to open file {}", assetsDB_path.string());
				return;
			}
			std::string file_data;
			FileSystem::read_all_lines(in_handle, file_data);
			FileSystem::close_file(in_handle);

			YAML::Node data = YAML::Load(file_data);
			std::string trace_version = data["Trace Version"].as<std::string>(); // TODO: To be used later
			std::string db_version = data["DataBase Version"].as<std::string>(); // TODO: To be used later
			std::string db_type = data["DataBase Type"].as<std::string>(); // TODO: To be used later
			YAML::Node DATA = data["DATA"];
			for (auto i : DATA)
			{
				std::string filename = i["Name"].as<std::string>();
				UUID id = i["UUID"].as<uint64_t>();
				all_files_id[filename] = id;
			}
			
		}
		bool dirty_ids = false;
		for (auto p : std::filesystem::directory_iterator(m_editor->current_project_path))
		{
			std::filesystem::path path = p.path();
			if (path == db_path) continue;
			if (std::filesystem::is_directory(path))
			{
				for (auto r : std::filesystem::recursive_directory_iterator(path))
				{
					if (std::filesystem::is_regular_file(r.path()))
					{
						std::string filename = r.path().filename().string();
						auto it = all_files_id.find(filename);
						if (it == all_files_id.end())
						{
							dirty_ids = true;
							all_files_id[filename] = UUID::GenUUID();
						}
						UUID id = all_files_id[filename];
						all_id_path[id] = r.path();
					}
				}
			}
			else if (std::filesystem::is_regular_file(path))
			{
				std::string filename = path.filename().string();
				auto it = all_files_id.find(filename);
				if (it == all_files_id.end())
				{
					dirty_ids = true;
					all_files_id[filename] = UUID::GenUUID();
				}
				UUID id = all_files_id[filename];
				all_id_path[id] = path;
			}
		}

		if (dirty_ids)
		{
			YAML::Emitter emit;

			emit << YAML::BeginMap;
			emit << YAML::Key << "Trace Version" << YAML::Value << "0.0.0.0";
			emit << YAML::Key << "DataBase Version" << YAML::Value << "0.0.0.0";
			emit << YAML::Key << "DataBase Type" << YAML::Value << "Assets";
			emit << YAML::Key << "DATA" << YAML::Value << YAML::BeginSeq;

			for (auto i : all_files_id)
			{
				emit << YAML::BeginMap;
				emit << YAML::Key << "Name" << YAML::Value << i.first;
				emit << YAML::Key << "UUID" << YAML::Value << i.second;
				emit << YAML::EndMap;
			}

			emit << YAML::EndSeq;
			emit << YAML::EndMap;

			FileHandle out_handle;
			if (FileSystem::open_file(assetsDB_path.string(), FileMode::WRITE, out_handle))
			{
				FileSystem::writestring(out_handle, emit.c_str());
				FileSystem::close_file(out_handle);
			}
		}
	}
}