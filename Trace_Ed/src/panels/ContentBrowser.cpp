
#include "ContentBrowser.h"
#include "../TraceEditor.h"
#include "resource/TextureManager.h"
#include "resource/MeshManager.h"
#include "resource/MaterialManager.h"
#include "resource/PipelineManager.h"
#include "resource/ShaderManager.h"
#include "resource/TextureManager.h"
#include "resource/FontManager.h"
#include "backends/UIutils.h"
#include "scene/UUID.h"
#include "serialize/MaterialSerializer.h"
#include "serialize/PipelineSerializer.h"

#include "imgui.h"
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "serialize/yaml_util.h"
#include <unordered_map>
#include <functional>
#include <string>

namespace trace {
	extern void ImportOBJ(const std::string& path, std::vector<std::string>& out_models, bool create_materials);
	extern std::filesystem::path GetPathFromUUID(UUID uuid);
	extern UUID GetUUIDFromName(const std::string& name);

	static float thumbnail_size = 96.0f;
	std::unordered_map<std::string, std::function<void(std::filesystem::path&)>> process_callbacks;
	std::unordered_map<std::string, std::function<void(std::filesystem::path&)>> extensions_callbacks;

	bool ContentBrowser::Init()
	{
		m_currentDir = m_editor->current_project_path;

		directory_icon = TextureManager::get_instance()->LoadTexture("directory.png");
		default_icon = TextureManager::get_instance()->LoadTexture("default_file_icon.png");


		//Process callbacks
		{
			process_callbacks[".obj"] = [&](std::filesystem::path& path) 
			{
				auto it = m_editor->all_assets.meshes.find(path);
				if (it == m_editor->all_assets.meshes.end())
				{
					m_editor->all_assets.meshes.emplace(path);
					std::vector<std::string> models;
					ImportOBJ(path.string(), models, true);
					for (auto& i : models)
					{
						m_editor->all_assets.models.emplace(path / i);
					}
				}
			};
			process_callbacks[".trmat"] = [&](std::filesystem::path& path)
			{
				m_editor->all_assets.materials.emplace(path);

			};

			process_callbacks[".trpip"] = [&](std::filesystem::path& path)
			{
				m_editor->all_assets.pipelines.emplace(path);

			};

			process_callbacks[".glsl"] = [&](std::filesystem::path& path)
			{
				m_editor->all_assets.shaders.emplace(path);

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

			extensions_callbacks[".trmat"] = [&](std::filesystem::path& path)
			{
				m_editMaterialPath = path;
				m_editor->m_inspectorPanel.SetDrawCallbackFn([&]() { m_editor->m_contentBrowser.DrawEditMaterial(); }, 
					[&]()
					{
						m_editMaterialPath = path;
						std::string filename = m_editMaterialPath.filename().string();
						m_editMaterial = MaterialManager::get_instance()->GetMaterial(filename);
						if (!m_editMaterial) m_editMaterial = MaterialSerializer::Deserialize(m_editMaterialPath.string());
						m_editMaterialPipe = m_editMaterial->m_renderPipeline;

						if (m_editMaterial)
						{
							m_materialDataCache.clear();
							m_materialDataCache = m_editMaterial->m_data;
						}
						
					},
					[&]() 
					{
						if (m_editMaterialPipeChanged)
						{
							MaterialManager::get_instance()->RecreateMaterial(m_editMaterial, m_editMaterialPipe);
							m_editMaterialPipeChanged = false;
						}
						m_editMaterial->m_data = m_materialDataCache;
						RenderFunc::PostInitializeMaterial(m_editMaterial.get(), m_editMaterialPipe);

						m_editMaterialPath = "";
						m_editMaterial.free();
						m_materialDataCache.clear();
						m_editMaterialPipe.free();

					});
			};

			extensions_callbacks[".trscn"] = [&](std::filesystem::path& path)
			{
				m_editor->OpenScene(path.string());
			};

			extensions_callbacks[".ttf"] = [&](std::filesystem::path& path)
			{
				m_editor->m_inspectorPanel.SetDrawCallbackFn([&]() { m_editor->m_contentBrowser.DrawEditFont(); },
					[&]()
					{
						m_editFont = FontManager::get_instance()->LoadFont_(path.string());
					},
					[&]()
					{
						m_editFont.free();
					});
			};

			extensions_callbacks[".TTF"] = [&](std::filesystem::path& path)
			{
				m_editor->m_inspectorPanel.SetDrawCallbackFn([&]() { m_editor->m_contentBrowser.DrawEditFont(); },
					[&]()
					{
						m_editFont = FontManager::get_instance()->LoadFont_(path.string());
					},
					[&]()
					{
						m_editFont.free();
					});
			};

			extensions_callbacks[".trpip"] = [&](std::filesystem::path& path)
			{
				m_editor->m_inspectorPanel.SetDrawCallbackFn([&]() { m_editor->m_contentBrowser.DrawEditPipeline(); },
					[&]()
					{
						m_editPipeline = PipelineSerializer::Deserialize(path.string());
						m_editPipePath = path;
						if (m_editPipeline)
						{
							m_editPipeDesc = m_editPipeline->GetDesc();
							m_editPipeType = (PipelineType)m_editPipeline->pipeline_type;
						}
					},
					[&]()
					{
						m_editPipeline.free();
						m_editPipePath = "";
						m_editPipeDesc = {};
						m_editPipeType = PipelineType::Unknown;
					});
			};



		};

		OnDirectoryChanged();
		ProcessAllDirectory();
		return true;
	}
	void ContentBrowser::Shutdown()
	{
		// TODO: Add a data structure that holds and releases all textures
		directory_icon.free();
		default_icon.free();
		m_editMaterial.free();
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


		for (uint32_t i = 0; i < m_dirContents.size(); i++)
		{
			std::filesystem::path& path = m_dirContents[i];

			std::string filename = path.filename().string();
			std::string extension = path.extension().string();
			void* textureID = nullptr;
			if (std::filesystem::is_directory(path))
			{
				UIFunc::GetDrawTextureHandle(directory_icon.get(), textureID);
				ImGui::ImageButton(filename.c_str(), textureID, { thumbnail_size, thumbnail_size }, { 0, 1 }, { 1, 0 });
				if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && ImGui::IsItemFocused())
				{
					m_currentDir /= filename;
					OnDirectoryChanged();
				}
			}
			else if (std::filesystem::is_regular_file(path))
			{
				UIFunc::GetDrawTextureHandle(default_icon.get(), textureID);

				ImGui::ImageButton(filename.c_str(), textureID, { thumbnail_size, thumbnail_size }, { 0, 1 }, { 1, 0 });
				if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && ImGui::IsItemFocused())
				{
					if (extensions_callbacks[extension]) extensions_callbacks[extension](path);
				}
			}

			OnItemPopup(path);

			if (ImGui::BeginDragDropSource())
			{
				std::string res_path = path.string();
				res_path += "\0";
				std::string ext = path.extension().string();
				ImGui::SetDragDropPayload(ext.c_str(), res_path.c_str(), res_path.size() + 1);
				ImGui::EndDragDropSource();
			}

			ImGui::TextWrapped(filename.c_str());


			ImGui::NextColumn();
		}

		ImGui::PopStyleColor(3);
		ImGui::Columns(1);

		OnWindowPopup();

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

		for (uint32_t i = 0; i < m_dirContents.size(); i++)
		{
			std::filesystem::path& path = m_dirContents[i];
			if (!std::filesystem::is_regular_file(path)) continue;

			std::string ext = path.extension().string();
			if (process_callbacks[ext])
			{
				process_callbacks[ext](path);
			}

		}

	}
	void ContentBrowser::ProcessAllDirectory()
	{
		all_files_id.clear();
		all_id_path.clear();

		//TEMP: Find a better way to identify default data

		all_files_id["albedo_map"] = 1;
		all_files_id["specular_map"] = 2;
		all_files_id["normal_map"] = 3;

		all_files_id["Cube"] = 4;
		all_files_id["Sphere"] = 5;

		all_files_id["default"] = 6;
		all_files_id["Plane"] = 7;

		all_files_id["gbuffer_pipeline"] = 8;

		all_id_path[1] = "albedo_map";
		all_id_path[2] = "specular_map";
		all_id_path[3] = "normal_map";

		all_id_path[4] = "Cube";
		all_id_path[5] = "Sphere";

		all_id_path[6] = "default";
		all_id_path[7] = "Plane";
		all_id_path[8] = "gbuffer_pipeline";

		//::-----::

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

		/*for (auto& i : all_files_id)
		{
			auto it = all_id_path.find(i.second);
			if (it == all_id_path.end())
			{
				all_files_id.erase(i.first);
				dirty_ids = true;
			}
		}*/

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
	void ContentBrowser::OnWindowPopup()
	{
		enum CreateItem
		{
			MATERIAL = 1,
			FOLDER
		};

		static CreateItem c_item = (CreateItem)0;

		if (ImGui::BeginPopupContextWindow("Content_Browser_Window", ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
		{
			if (ImGui::BeginMenu("Create"))
			{
				if (ImGui::MenuItem("Material")) c_item = MATERIAL;
				if (ImGui::MenuItem("Folder"))
				{
					c_item = FOLDER;
				}
				ImGui::EndMenu();
			}

			ImGui::EndPopup();
		}

		switch (c_item)
		{
		case FOLDER:
		{

			std::string res;
			if (m_editor->InputTextPopup("Folder Name", res))
			{
				if (!res.empty())
				{
					c_item = (CreateItem)0;
					std::filesystem::create_directory(m_currentDir / res);
					OnDirectoryChanged();
				}
			}
			else c_item = (CreateItem)0;
			break;
		}
		case MATERIAL:
		{
			std::string res;
			if (m_editor->InputTextPopup("Material Name", res))
			{
				if (!res.empty())
				{
					c_item = (CreateItem)0;
					UUID id = GetUUIDFromName(res + ".trmat");
					if (id == 0)
					{
						Ref<GPipeline> sp = PipelineManager::get_instance()->GetPipeline("gbuffer_pipeline");
						Ref<MaterialInstance> mat = MaterialManager::get_instance()->CreateMaterial(res + ".trmat", sp);
						if (mat)
						{
							std::string path = (m_currentDir / (res + ".trmat")).string();
							MaterialSerializer::Serialize(mat, path);
						}
						OnDirectoryChanged();
					}
					else
					{
						TRC_ERROR("{} has already been created", res + ".trmat");
					}
				}
			}
			else c_item = (CreateItem)0;
			break;
		}
		}

	}
	void ContentBrowser::OnItemPopup(std::filesystem::path& path)
	{
		if (ImGui::BeginPopupContextItem())
		{
			if(ImGui::MenuItem("Delete")){}
			ImGui::EndPopup();
		}
	}
	void ContentBrowser::DrawEditMaterial()
	{
		
		Ref<MaterialInstance> mat = m_editMaterial;
		static bool tex_modified = false;
		static std::string tex_name;
		bool dirty = false;
		static bool popup = false;


		ImGui::Text("Material : %s", mat->m_path.string().c_str());
		ImGui::Columns(2);
		ImGui::Text("Render Pipeline");
		ImGui::NextColumn();
		Ref<GPipeline> sp = mat->GetRenderPipline();
		if (ImGui::Button(sp->m_path.string().c_str())) popup = true;

		if (popup)
		{
			std::string pipe_res;
			if (popup = m_editor->DrawPipelinesPopup(pipe_res))
			{
				if (!pipe_res.empty())
				{
					std::filesystem::path p = pipe_res;
					Ref<GPipeline> p_res = PipelineManager::get_instance()->GetPipeline(p.filename().string());
					if (!p_res) p_res = PipelineSerializer::Deserialize(p.string());

					if (p_res)
					{
						if (MaterialManager::get_instance()->RecreateMaterial(mat, p_res))
						{
							m_editMaterialPipeChanged = true;
						}
						else
						{
							m_editMaterial = MaterialSerializer::Deserialize(m_editMaterialPath.string());
							mat = m_editMaterial;
						}

					}

					popup = false;
				}
			}
		}

		ImGui::Columns(1);


		auto lambda = [&](trace::ShaderData type, std::any& dst, const std::string& name)
		{
			switch (type)
			{
			case trace::ShaderData::CUSTOM_DATA_BOOL:
			{
				bool* data = &std::any_cast<bool&>(dst);
				ImGui::Checkbox(name.c_str(), data);
				dst = *data;
				break;
			}
			case trace::ShaderData::CUSTOM_DATA_FLOAT:
			{
				float* data = &std::any_cast<float&>(dst);
				ImGui::DragFloat(name.c_str(), data);
				dst = *data;
				break;
			}
			case trace::ShaderData::CUSTOM_DATA_INT:
			{
				int* data = &std::any_cast<int&>(dst);
				ImGui::DragInt(name.c_str(), data);
				dst = *data;
				break;
			}
			case trace::ShaderData::CUSTOM_DATA_IVEC2:
			{
				glm::ivec2& data = std::any_cast<glm::ivec2&>(dst);
				ImGui::DragInt2(name.c_str(), glm::value_ptr(data));
				dst = data;
				break;
			}
			case trace::ShaderData::CUSTOM_DATA_IVEC3:
			{
				glm::ivec3& data = std::any_cast<glm::ivec3&>(dst);
				ImGui::DragInt3(name.c_str(), glm::value_ptr(data));
				dst = data;
				break;
			}
			case trace::ShaderData::CUSTOM_DATA_IVEC4:
			{
				glm::ivec4* data = &std::any_cast<glm::ivec4&>(dst);
				ImGui::DragInt4(name.c_str(), (int*)data);
				dst = data;
				break;
			}
			case trace::ShaderData::CUSTOM_DATA_MAT2:
			{
				glm::mat2& data = std::any_cast<glm::mat2&>(dst);
				ImGui::Text(name.c_str());
				ImGui::DragFloat2((name + "row_0").c_str(), glm::value_ptr(data[0]));
				ImGui::DragFloat2((name + "row_1").c_str(), glm::value_ptr(data[1]));
				dst = data;
				break;
				break;
			}
			case trace::ShaderData::CUSTOM_DATA_MAT3:
			{
				glm::mat3& data = std::any_cast<glm::mat3&>(dst);
				ImGui::Text(name.c_str());
				ImGui::DragFloat3((name + "row_0").c_str(), glm::value_ptr(data[0]));
				ImGui::DragFloat3((name + "row_1").c_str(), glm::value_ptr(data[1]));
				ImGui::DragFloat3((name + "row_2").c_str(), glm::value_ptr(data[2]));
				dst = data;
				break;
			}
			case trace::ShaderData::CUSTOM_DATA_MAT4:
			{
				glm::mat4& data = std::any_cast<glm::mat4&>(dst);
				ImGui::Text(name.c_str());
				ImGui::DragFloat4((name + "row_0").c_str(), glm::value_ptr(data[0]));
				ImGui::DragFloat4((name + "row_1").c_str(), glm::value_ptr(data[1]));
				ImGui::DragFloat4((name + "row_2").c_str(), glm::value_ptr(data[2]));
				ImGui::DragFloat4((name + "row_3").c_str(), glm::value_ptr(data[3]));
				dst = data;
				break;
			}
			case trace::ShaderData::CUSTOM_DATA_TEXTURE:
			{
				Ref<GTexture> tex = std::any_cast<Ref<GTexture>>(dst);
				ImGui::Columns(2);
				ImGui::Text(name.c_str());
				ImGui::Text(tex->GetName().c_str());
				ImGui::NextColumn();
				void* a = nullptr;
				UIFunc::GetDrawTextureHandle(tex.get(), a);
				ImGui::Image(a, ImVec2(128.0f, 128.0f) , {0.0f, 1.0f}, {1.0f, 0.0f});
				if (ImGui::IsItemClicked())
				{
					tex_modified = true;
					tex_name = name;
				}
				if (ImGui::BeginDragDropTarget())
				{
					static char buf[1024] = { 0 };
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(".png"))
					{
						memcpy_s(buf, 1024, payload->Data, payload->DataSize);
						std::filesystem::path p = buf;
						Ref<GTexture> tex = TextureManager::get_instance()->GetTexture(p.filename().string());
						if (tex) {}
						else tex = TextureManager::get_instance()->LoadTexture_(p.string());
						
						if (tex)
						{
							dst = tex;
							dirty = true;
						}
					}
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(".tga"))
					{
						memcpy_s(buf, 1024, payload->Data, payload->DataSize);
						std::filesystem::path p = buf;
						Ref<GTexture> tex = TextureManager::get_instance()->GetTexture(p.filename().string());
						if (tex) {}
						else tex = TextureManager::get_instance()->LoadTexture_(p.string());

						if (tex)
						{
							dst = tex;
							dirty = true;
						}
					}
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(".jpg"))
					{
						memcpy_s(buf, 1024, payload->Data, payload->DataSize);
						std::filesystem::path p = buf;
						Ref<GTexture> tex = TextureManager::get_instance()->GetTexture(p.filename().string());
						if (tex) {}
						else tex = TextureManager::get_instance()->LoadTexture_(p.string());

						if (tex)
						{
							dst = tex;
							dirty = true;
						}
					}
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(".jpeg"))
					{
						memcpy_s(buf, 1024, payload->Data, payload->DataSize);
						std::filesystem::path p = buf;
						Ref<GTexture> tex = TextureManager::get_instance()->GetTexture(p.filename().string());
						if (tex) {}
						else tex = TextureManager::get_instance()->LoadTexture_(p.string());

						if (tex)
						{
							dst = tex;
							dirty = true;
						}
					}
					ImGui::EndDragDropTarget();
				}
				
				ImGui::Columns(1);
				break;
			}
			case trace::ShaderData::CUSTOM_DATA_VEC2:
			{
				glm::vec2& data = std::any_cast<glm::vec2&>(dst);
				ImGui::DragFloat2(name.c_str(), glm::value_ptr(data));
				dst = data;
				break;
			}
			case trace::ShaderData::CUSTOM_DATA_VEC3:
			{
				glm::vec3& data = std::any_cast<glm::vec3&>(dst);
				ImGui::DragFloat3(name.c_str(), glm::value_ptr(data));
				dst = data;
				break;
			}
			case trace::ShaderData::CUSTOM_DATA_VEC4:
			{
				glm::vec4& data = std::any_cast<glm::vec4&>(dst);
				ImGui::DragFloat4(name.c_str(), glm::value_ptr(data));
				dst = data;
				break;
			}
			}
		};

		for (auto& m_data : mat->m_data)
		{
			trace::UniformMetaData& meta_data = mat->m_renderPipeline->Scene_uniforms[m_data.second.second];
			lambda(meta_data.data_type, m_data.second.first, m_data.first);
		}

		// Select Texture
		if (tex_modified)
		{
			std::string tex_res;
			if (m_editor->DrawTexturesPopup(tex_res))
			{
				if (!tex_res.empty())
				{
					std::filesystem::path p = tex_res;
					Ref<GTexture> tex_r = TextureManager::get_instance()->GetTexture(p.filename().string());
					if (tex_r) {}
					else tex_r = TextureManager::get_instance()->LoadTexture_(p.string());

					if (tex_r)
					{
						mat->m_data[tex_name].first = tex_r;
						dirty = true;
					}
					tex_modified = false;
				}
			}
			else tex_modified = false;
		}

		if(dirty) RenderFunc::PostInitializeMaterial(mat.get(), mat->m_renderPipeline);

		if (ImGui::Button("Apply"))
		{
			m_materialDataCache = mat->m_data;
			m_editMaterialPipe = mat->m_renderPipeline;
			m_editMaterialPipeChanged = false;
			MaterialSerializer::Serialize(mat, m_editMaterialPath.string());
		}

	}
	void ContentBrowser::DrawEditFont()
	{
		if (m_editFont)
		{
			ImGui::Text("Font Name: %s",m_editFont->GetName().c_str());
			ImGui::Text("Font Atlas : ");
			void* texture = nullptr;
			UIFunc::GetDrawTextureHandle(m_editFont->GetAtlas().get(), texture);
			ImVec2 content_ava = ImGui::GetContentRegionAvail();
			ImGui::Image(texture, { content_ava.x, content_ava.y * 0.65f }, {0.0f, 1.0f}, {1.0f, 0.0f});
		}
	}
	void ContentBrowser::DrawEditPipeline()
	{
		ImGui::Text("Name : %s ", m_editPipeline->GetName().c_str());

		static ShaderStage shad_stage = ShaderStage::STAGE_NONE;

		static bool shad_pop = false;

		//Input Layout
		{
			ImGui::InputInt("Stride", (int*)&m_editPipeDesc.input_layout.stride);
			const char* type_string[] = { "Vertex", "Instance" };
			const char* current_type = type_string[(int)m_editPipeDesc.input_layout.input_class];
			if (ImGui::BeginCombo("Input Classification", current_type))
			{
				for (int i = 0; i < 2; i++)
				{
					bool selected = (current_type == type_string[i]);
					if (ImGui::Selectable(type_string[i], selected))
					{
						m_editPipeDesc.input_layout.input_class = (InputClassification)i;
					}

					if (selected)
						ImGui::SetItemDefaultFocus();
				}

				ImGui::EndCombo();
			}

		};

		ImGui::Text("Vertex Shader : ");
		ImGui::SameLine();
		if (ImGui::Button(m_editPipeDesc.vertex_shader->GetName().c_str()))
		{
			shad_pop = true;
			shad_stage = ShaderStage::VERTEX_SHADER;
		}

		ImGui::Text("Pixel Shader : ");
		ImGui::SameLine();
		if (ImGui::Button(m_editPipeDesc.pixel_shader->GetName().c_str()))
		{
			shad_pop = true;
			shad_stage = ShaderStage::PIXEL_SHADER;
		}

		if (shad_pop)
		{
			std::string shad_res;
			if (shad_pop = m_editor->DrawShadersPopup(shad_res))
			{
				if (!shad_res.empty())
				{
					Ref<GShader> res = ShaderManager::get_instance()->CreateShader_(shad_res, shad_stage);
					if (shad_stage == ShaderStage::VERTEX_SHADER)
						m_editPipeDesc.vertex_shader = res.get();
					if (shad_stage == ShaderStage::PIXEL_SHADER)
						m_editPipeDesc.pixel_shader = res.get();
					
					shad_pop = false;
				}
			}
		}

		if (ImGui::Button("Apply"))
		{
			if (PipelineManager::get_instance()->RecreatePipeline(m_editPipeline, m_editPipeDesc))
			{
				m_editPipeline->pipeline_type = m_editPipeType;
				PipelineSerializer::Serialize(m_editPipeline, m_editPipePath.string());
			}
		}

	}
}