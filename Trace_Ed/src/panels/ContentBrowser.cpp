
#include "ContentBrowser.h"
#include "../TraceEditor.h"
#include "resource/TextureManager.h"
#include "resource/MeshManager.h"
#include "resource/MaterialManager.h"
#include "resource/PipelineManager.h"
#include "resource/ShaderManager.h"
#include "resource/TextureManager.h"
#include "resource/FontManager.h"
#include "resource/PrefabManager.h"
#include "backends/UIutils.h"
#include "scene/UUID.h"
#include "serialize/MaterialSerializer.h"
#include "serialize/PipelineSerializer.h"
#include "InspectorPanel.h"
#include "resource/AnimationsManager.h"
#include "serialize/AnimationsSerializer.h"
#include "serialize/SceneSerializer.h"
#include "core/Utils.h"
#include "../utils/ImGui_utils.h"
#include "HierachyPanel.h"
#include "../import/Importer.h"
#include "external_utils.h"


#include "imgui.h"
#include "imgui_internal.h"
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
		TraceEditor* editor = TraceEditor::get_instance();
		if(editor->GetCurrentProject()) m_currentDir = editor->GetCurrentProject()->GetAssetsDirectory();

		directory_icon = TextureManager::get_instance()->LoadTexture("directory.png");
		default_icon = TextureManager::get_instance()->LoadTexture("default_file_icon.png");


		//Process callbacks
		{
			process_callbacks[".obj"] = [editor](std::filesystem::path& path)
			{
				/*auto it = editor->GetAllProjectAssets().meshes.find(path);
				if (it == editor->GetAllProjectAssets().meshes.end())
				{
					editor->GetAllProjectAssets().meshes.emplace(path);
					std::vector<std::string> models;
					ImportOBJ(path.string(), models, true);
					for (auto& i : models)
					{
						editor->GetAllProjectAssets().models.emplace(path / i);
					}
				}*/
			};
			process_callbacks[".trmat"] = [editor](std::filesystem::path& path)
			{
				editor->GetAllProjectAssets().materials.emplace(path);

			};

			process_callbacks[".trpip"] = [editor](std::filesystem::path& path)
			{
				editor->GetAllProjectAssets().pipelines.emplace(path);

			};

			process_callbacks[".trscn"] = [editor](std::filesystem::path& path)
			{
				editor->GetAllProjectAssets().scenes.emplace(path);

			};

			process_callbacks[".glsl"] = [editor](std::filesystem::path& path)
			{
				editor->GetAllProjectAssets().shaders.emplace(path);

			};

			
			auto tex_lambda = [editor](std::filesystem::path& path)
			{
				editor->GetAllProjectAssets().textures.emplace(path);
			};
			

			process_callbacks[".png"] = tex_lambda;
			process_callbacks[".jpg"] = tex_lambda;
			process_callbacks[".tga"] = tex_lambda;
		};

		//extensions callbacks
		{

			extensions_callbacks[".trmat"] = [editor](std::filesystem::path& path)
			{
				AssetsEdit& assets_edit = editor->GetContentBrowser()->GetAssetsEdit();
				assets_edit.editMaterialPath = path;
				editor->GetInspectorPanel()->SetDrawCallbackFn([editor]() { editor->GetContentBrowser()->DrawEditMaterial(); },
					[editor]()
					{
						AssetsEdit& assets_edit = editor->GetContentBrowser()->GetAssetsEdit();
						std::string filename = assets_edit.editMaterialPath.filename().string();
						assets_edit.editMaterial = MaterialManager::get_instance()->GetMaterial(filename);
						if (!assets_edit.editMaterial)
						{
							assets_edit.editMaterial = MaterialSerializer::Deserialize(assets_edit.editMaterialPath.string());
						}
						assets_edit.editMaterialPipe = assets_edit.editMaterial->GetRenderPipline();

						if (assets_edit.editMaterial)
						{
							assets_edit.materialDataCache.clear();
							assets_edit.materialDataCache = assets_edit.editMaterial->GetMaterialData();
						}

					},
					[editor]()
					{
						AssetsEdit& assets_edit = editor->GetContentBrowser()->GetAssetsEdit();
						if (assets_edit.editMaterialPipeChanged)
						{
							MaterialManager::get_instance()->RecreateMaterial(assets_edit.editMaterial, assets_edit.editMaterialPipe);
							assets_edit.editMaterialPipeChanged = false;
						}
						assets_edit.editMaterial->SetMaterialData(assets_edit.materialDataCache);
						RenderFunc::PostInitializeMaterial(assets_edit.editMaterial.get(), assets_edit.editMaterialPipe);

						
						assets_edit.editMaterial.free();
						assets_edit.materialDataCache.clear();
						assets_edit.editMaterialPipe.free();

					});
			};

			extensions_callbacks[".trprf"] = [editor](std::filesystem::path& path)
			{
				editor->GetInspectorPanel()->SetDrawCallbackFn([editor]() 
					{ 
						if (editor->GetHierachyPanel()->GetSelectedEntity())
							editor->GetInspectorPanel()->DrawEntityComponent(editor->GetHierachyPanel()->GetSelectedEntity());
					},
					[editor, path]()
					{
						editor->GetHierachyPanel()->SetPrefabEdit(SceneSerializer::DeserializePrefab(path.string()));
					},
					[editor]()
					{
						Ref<Prefab> prefab = editor->GetHierachyPanel()->GetPrefabEdit();
						SceneSerializer::SerializePrefab(prefab,prefab->m_path.string());
					});
			};

			extensions_callbacks[".trscn"] = [editor](std::filesystem::path& path)
			{
				editor->OpenScene(path.string());
			};

			extensions_callbacks[".ttf"] = [editor](std::filesystem::path& path)
			{
				editor->GetInspectorPanel()->SetDrawCallbackFn([editor]() { editor->GetContentBrowser()->DrawEditFont(); },
					[editor, path]()
					{
						AssetsEdit& assets_edit = editor->GetContentBrowser()->GetAssetsEdit();
						assets_edit.editFont = FontManager::get_instance()->LoadFont_(path.string());
					},
					[editor]()
					{
						AssetsEdit& assets_edit = editor->GetContentBrowser()->GetAssetsEdit();
						assets_edit.editFont.free();
					});
			};

			extensions_callbacks[".TTF"] = [editor](std::filesystem::path& path)
			{
				editor->GetInspectorPanel()->SetDrawCallbackFn([editor]() { editor->GetContentBrowser()->DrawEditFont(); },
					[editor, path]()
					{
						AssetsEdit& assets_edit = editor->GetContentBrowser()->GetAssetsEdit();
						assets_edit.editFont = FontManager::get_instance()->LoadFont_(path.string());
					},
					[editor]()
					{
						AssetsEdit& assets_edit = editor->GetContentBrowser()->GetAssetsEdit();
						assets_edit.editFont.free();
					});
			};

			extensions_callbacks[".trpip"] = [editor](std::filesystem::path& path)
			{
				editor->GetInspectorPanel()->SetDrawCallbackFn([editor]() { editor->GetContentBrowser()->DrawEditPipeline(); },
					[editor, path]()
					{
						AssetsEdit& assets_edit = editor->GetContentBrowser()->GetAssetsEdit();
						assets_edit.editPipeline = PipelineSerializer::Deserialize(path.string());
						assets_edit.editPipePath = path;
						if (assets_edit.editPipeline)
						{
							assets_edit.editPipeDesc = assets_edit.editPipeline->GetDesc();
							assets_edit.editPipeType = (PipelineType)assets_edit.editPipeline->GetPipelineType();
						}
					},
					[editor]()
					{
						AssetsEdit& assets_edit = editor->GetContentBrowser()->GetAssetsEdit();
						assets_edit.editPipeline.free();
						assets_edit.editPipePath = "";
						assets_edit.editPipeDesc = {};
						assets_edit.editPipeType = PipelineType::Unknown;
					});
			};
			auto mesh_lambda = [editor](std::filesystem::path& path)
			{
				ContentBrowser* content_broswer = editor->GetContentBrowser();
				auto it = content_broswer->GetImportedAssets().find(path.filename().string());
				if (it == content_broswer->GetImportedAssets().end())
				{
					Importer* importer = editor->GetImporter();
					if (importer->ImportMeshFile(path.string()))
					{
						content_broswer->GetImportedAssets().emplace(path.filename().string());
					}
				}
			};

			extensions_callbacks[".fbx"] = mesh_lambda;
			extensions_callbacks[".blend"] = mesh_lambda;
			extensions_callbacks[".obj"] = mesh_lambda;
			extensions_callbacks[".gltf"] = mesh_lambda;

		};

		if (!m_currentDir.empty())
		{
			OnDirectoryChanged();
			ProcessAllDirectory();
		}
		return true;
	}
	void ContentBrowser::Shutdown()
	{

		// TODO: Add a data structure that holds and releases all textures
		directory_icon.free();
		default_icon.free();
		m_assetsEdit.editMaterial.free();
	}
	void ContentBrowser::Render(float deltaTime)
	{
		TraceEditor* editor = TraceEditor::get_instance();
		ImGui::Begin("Content Browser");

		if (editor->GetCurrentProject())
		{
			if (ImGui::ArrowButton("<<", ImGuiDir_Left))
			{
				if (m_currentDir != editor->GetCurrentProject()->GetAssetsDirectory())
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

			ImGui::PushStyleColor(ImGuiCol_Button, { 0.0f, 0.0f, 0.0f, 0.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { button_hov.x, button_hov.y, button_hov.z, 0.5f });
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, { button_act.x, button_act.y, button_act.z, 0.5f });

			int selected = -1;
			for (int i = 0; i < m_dirContents.size(); i++)
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
						selected = -1;
					}
				}
				else if (std::filesystem::is_regular_file(path))
				{
					UIFunc::GetDrawTextureHandle(default_icon.get(), textureID);

					ImGui::ImageButton(filename.c_str(), textureID, { thumbnail_size, thumbnail_size }, { 0, 1 }, { 1, 0 });
					if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && ImGui::IsItemFocused())
					{
						if (extensions_callbacks[extension]) extensions_callbacks[extension](path);
						selected = -1;
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
			if (m_dirContents.empty())
			{
				ImGui::Text("This Folder is empty");
			}

			ImGui::PopStyleColor(3);
			ImGui::Columns(1);

			OnWindowPopup();
		}

		static bool prefab_popup = false;
		static Entity prefab_entity;

		ImRect d_r = {};
		d_r.Min = ImGui::GetWindowPos();
		d_r.Max.x = d_r.Min.x + ImGui::GetWindowWidth();
		d_r.Max.y = d_r.Min.y + ImGui::GetWindowHeight();

		d_r.Min.x += 10.0f;
		d_r.Min.y += GetLineHeight() + 8.0f;
		d_r.Max.x -= 10.0f;
		d_r.Max.y -= 10.0f;
		if (ImGui::BeginDragDropTargetCustom(d_r, ImGui::GetID("Content Browser Drag_Drop")))
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Entity"))
			{
				UUID uuid = *(UUID*)payload->Data;
				prefab_entity = editor->GetCurrentScene()->GetEntity(uuid);
				prefab_popup = true;
			}
			ImGui::EndDragDropTarget();
		}
		ImGui::End();

		// Creation of prefabs
		if (prefab_popup)
		{
			std::string res;
			if (editor->InputTextPopup("Prefab Name", res))
			{
				if (!res.empty())
				{
					UUID id = GetUUIDFromName(res + ".trprf");
					if (id == 0)
					{
						std::string prefab_path = (m_currentDir / (res + ".trprf")).string();
						Ref<Prefab> prefab = PrefabManager::get_instance()->Create(res + ".trprf", prefab_entity);
						SceneSerializer::SerializePrefab(prefab, prefab_path);
						OnDirectoryChanged();
						ProcessAllDirectory();
					}
					else
					{
						TRC_ERROR("{} has already been created", res + ".trprf");
					}
					prefab_popup = false;
				}
				else prefab_popup = false;
			}
		}

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
	void ContentBrowser::ProcessAllDirectory(bool update_assets_db)
	{
		TraceEditor* editor = TraceEditor::get_instance();

		LoadImportedAssets();

		/*m_allFilesID.clear();
		m_allIDPath.clear();*/

		//TEMP: Find a better way to identify default data
		uint64_t def_id0 = 1;
		uint64_t def_id1 = 1;
		m_allFilesID["albedo_map"] = def_id0++;
		m_allFilesID["specular_map"] = def_id0++;
		m_allFilesID["normal_map"] = def_id0++;

		m_allFilesID["Cube"] = def_id0++;
		m_allFilesID["Sphere"] = def_id0++;

		m_allFilesID["default"] = def_id0++;
		m_allFilesID["Plane"] = def_id0++;

		m_allFilesID["gbuffer_pipeline"] = def_id0++;
		m_allFilesID["skybox_pipeline"] = def_id0++;
		m_allFilesID["light_pipeline"] = def_id0++;
		m_allFilesID["quad_batch_pipeline"] = def_id0++;
		m_allFilesID["text_batch_pipeline"] = def_id0++;
		m_allFilesID["text_pipeline"] = def_id0++;
		m_allFilesID["debug_line_pipeline"] = def_id0++;
		m_allFilesID["bloom_prefilter_pass_pipeline"] = def_id0++;
		m_allFilesID["bloom_downsample_pass_pipeline"] = def_id0++;
		m_allFilesID["bloom_upsample_pass_pipeline"] = def_id0++;
		m_allFilesID["lighting_pass_pipeline"] = def_id0++;
		m_allFilesID["ssao_main_pass_pipeline"] = def_id0++;
		m_allFilesID["ssao_blur_pass_pipeline"] = def_id0++;
		m_allFilesID["tone_map_pass_pipeline"] = def_id0++;
		m_allFilesID["black_texture"] = def_id0++;


		m_allIDPath[def_id1++] = "albedo_map";
		m_allIDPath[def_id1++] = "specular_map";
		m_allIDPath[def_id1++] = "normal_map";

		m_allIDPath[def_id1++] = "Cube";
		m_allIDPath[def_id1++] = "Sphere";

		m_allIDPath[def_id1++] = "default";
		m_allIDPath[def_id1++] = "Plane";
		m_allIDPath[def_id1++] = "gbuffer_pipeline";
		m_allIDPath[def_id1++] = "skybox_pipeline";
		m_allIDPath[def_id1++] = "light_pipeline";
		m_allIDPath[def_id1++] = "quad_batch_pipeline";
		m_allIDPath[def_id1++] = "text_batch_pipeline";
		m_allIDPath[def_id1++] = "text_pipeline";
		m_allIDPath[def_id1++] = "debug_line_pipeline";
		m_allIDPath[def_id1++] = "bloom_prefilter_pass_pipeline";
		m_allIDPath[def_id1++] = "bloom_downsample_pass_pipeline";
		m_allIDPath[def_id1++] = "bloom_upsample_pass_pipeline";
		m_allIDPath[def_id1++] = "lighting_pass_pipeline";
		m_allIDPath[def_id1++] = "ssao_main_pass_pipeline";
		m_allIDPath[def_id1++] = "ssao_blur_pass_pipeline";
		m_allIDPath[def_id1++] = "tone_map_pass_pipeline";
		m_allIDPath[def_id1++] = "black_texture";

		//::-----::


		std::filesystem::path db_path = std::filesystem::path(editor->GetCurrentProject()->GetProjectCurrentDirectory()) / "InternalAssetsDB";
		std::filesystem::path assetsDB_path = db_path / "assets.trdb";

		if (std::filesystem::exists(assetsDB_path))
		{


			YAML::Node data;
			YAML::load_yaml_data(assetsDB_path.string(), data);

			std::string trace_version = data["Trace Version"].as<std::string>(); // TODO: To be used later
			std::string db_version = data["DataBase Version"].as<std::string>(); // TODO: To be used later
			std::string db_type = data["DataBase Type"].as<std::string>(); // TODO: To be used later
			YAML::Node DATA = data["DATA"];
			for (auto i : DATA)
			{
				std::string filename = i["Name"].as<std::string>();
				UUID id = i["UUID"].as<uint64_t>();
				m_allFilesID[filename] = id;
			}

		}



		bool dirty_ids = false;

		// Adding builtin Shaders ================
		std::string shader_dir;
		FindDirectory(AppSettings::exe_path, "assets/shaders", shader_dir);
		TraceEditor::AllProjectAssets& all_assets = TraceEditor::get_instance()->GetAllProjectAssets();
		for (auto p : std::filesystem::directory_iterator(shader_dir))
		{
			std::filesystem::path path = p.path();
			if (std::filesystem::is_directory(path))
			{
				for (auto r : std::filesystem::recursive_directory_iterator(path))
				{
					if (std::filesystem::is_regular_file(r.path()) && path.extension().string() != ".shcode")
					{
						all_assets.shaders.emplace(r.path());
						std::string filename = r.path().filename().string();
						auto it = m_allFilesID.find(filename);
						if (it == m_allFilesID.end())
						{
							dirty_ids = true;
							m_allFilesID[filename] = UUID::GenUUID();
						}
						UUID id = m_allFilesID[filename];
						m_allIDPath[id] = r.path();
					}
				}
			}
			else if (std::filesystem::is_regular_file(path) && path.extension().string() != ".shcode")
			{
				all_assets.shaders.emplace(p.path());
				std::string filename = path.filename().string();
				auto it = m_allFilesID.find(filename);
				if (it == m_allFilesID.end())
				{
					dirty_ids = true;
					m_allFilesID[filename] = UUID::GenUUID();
				}
				UUID id = m_allFilesID[filename];
				m_allIDPath[id] = path;
			}


		}

		// =======================================


		
		if (!std::filesystem::exists(assetsDB_path))
		{
			if (!std::filesystem::exists(db_path)) std::filesystem::create_directory(db_path);

			YAML::Emitter emit;

			emit << YAML::BeginMap;
			emit << YAML::Key << "Trace Version" << YAML::Value << "0.0.0.0";
			emit << YAML::Key << "DataBase Version" << YAML::Value << "0.0.0.0";
			emit << YAML::Key << "DataBase Type" << YAML::Value << "Assets";
			emit << YAML::Key << "DATA" << YAML::Value << YAML::BeginSeq;
			for (auto p : std::filesystem::directory_iterator(editor->GetCurrentProject()->GetAssetsDirectory()))
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
		for (auto p : std::filesystem::directory_iterator(editor->GetCurrentProject()->GetAssetsDirectory()))
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
						auto it = m_allFilesID.find(filename);
						if (it == m_allFilesID.end())
						{
							dirty_ids = true;
							m_allFilesID[filename] = UUID::GenUUID();
						}
						UUID id = m_allFilesID[filename];
						m_allIDPath[id] = r.path();
					}
				}
			}
			else if (std::filesystem::is_regular_file(path))
			{
				std::string filename = path.filename().string();
				auto it = m_allFilesID.find(filename);
				if (it == m_allFilesID.end())
				{
					dirty_ids = true;
					m_allFilesID[filename] = UUID::GenUUID();
				}
				UUID id = m_allFilesID[filename];
				m_allIDPath[id] = path;
			}
		}


		if (dirty_ids || update_assets_db)
		{
			YAML::Emitter emit;

			emit << YAML::BeginMap;
			emit << YAML::Key << "Trace Version" << YAML::Value << "0.0.0.0";
			emit << YAML::Key << "DataBase Version" << YAML::Value << "0.0.0.0";
			emit << YAML::Key << "DataBase Type" << YAML::Value << "Assets";
			emit << YAML::Key << "DATA" << YAML::Value << YAML::BeginSeq;

			for (auto i : m_allFilesID)
			{
				emit << YAML::BeginMap;
				emit << YAML::Key << "Name" << YAML::Value << i.first;
				emit << YAML::Key << "UUID" << YAML::Value << i.second;
				emit << YAML::EndMap;
			}

			emit << YAML::EndSeq;
			emit << YAML::EndMap;

			
			YAML::save_emitter_data(emit, assetsDB_path.string());
		}

		for (auto i : m_allFilesID)
		{
			m_allIDNames[i.second] = i.first;
		}

	}
	void ContentBrowser::OnWindowPopup()
	{
		TraceEditor* editor = TraceEditor::get_instance();
		enum CreateItem
		{
			MATERIAL = 1,
			PIPELINE,
			FOLDER,
			SCENE,
			ANIMATION_CLIP,
			ANIMATION_GRAPH,
		};

		static CreateItem c_item = (CreateItem)0;

		if (ImGui::BeginPopupContextWindow("Content_Browser_Window", ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
		{
			if (ImGui::BeginMenu("Create"))
			{
				if (ImGui::MenuItem("Material"))
				{
					c_item = MATERIAL;
					
				}
				if (ImGui::MenuItem("Pipeline"))
				{
					c_item = PIPELINE;

				}
				if (ImGui::MenuItem("Folder"))
				{
					c_item = FOLDER;
				}
				if (ImGui::MenuItem("Scene"))
				{
					c_item = SCENE;
				}
				if (ImGui::MenuItem("Animation Clip"))
				{
					c_item = ANIMATION_CLIP;
				}
				if (ImGui::MenuItem("Animation Graph"))
				{
					c_item = ANIMATION_GRAPH;
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
			if (editor->InputTextPopup("Folder Name", res))
			{
				if (!res.empty())
				{
					c_item = (CreateItem)0;
					std::filesystem::create_directory(m_currentDir / res);
					OnDirectoryChanged();
				}
			}
			else c_item = (CreateItem)0;
			ProcessAllDirectory();
			break;
		}
		case MATERIAL:
		{
			std::string res;
			if (editor->InputTextPopup("Material Name", res))
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
			ProcessAllDirectory();
			break;
		}
		case PIPELINE:
		{
			std::string res;
			if (editor->InputTextPopup("Pipeline Name", res))
			{
				if (!res.empty())
				{
					c_item = (CreateItem)0;
					UUID id = GetUUIDFromName(res + ".trpip");
					if (id == 0)
					{
						Ref<GPipeline> sp = PipelineManager::get_instance()->GetPipeline("gbuffer_pipeline");
						Ref<GPipeline> new_pipeline = PipelineManager::get_instance()->CreatePipeline(sp->GetDesc(), res + ".trpip", false);
						new_pipeline->SetPipelineType(PipelineType::Surface_Material);
						std::string path = (m_currentDir / (res + ".trpip")).string();
						PipelineSerializer::Serialize(new_pipeline, path);
						OnDirectoryChanged();
					}
					else
					{
						TRC_ERROR("{} has already been created", res + ".trpip");
					}
				}
			}
			else c_item = (CreateItem)0;
			ProcessAllDirectory();
			break;
		}
		case SCENE:
		{
			std::string res;
			if (editor->InputTextPopup("Scene Name", res))
			{
				if (!res.empty())
				{
					c_item = (CreateItem)0;
					UUID id = GetUUIDFromName(res + ".trscn");
					if (id == 0)
					{
						std::string scene_path = (m_currentDir / (res + ".trscn")).string();
						editor->CreateScene(scene_path);
						OnDirectoryChanged();
					}
					else
					{
						TRC_ERROR("{} has already been created", res + ".trscn");
					}
				}
			}
			else c_item = (CreateItem)0;
			ProcessAllDirectory();
			break;
		}
		case ANIMATION_CLIP:
		{
			std::string res;
			if (editor->InputTextPopup("Animation Clip Name", res))
			{
				if (!res.empty())
				{
					c_item = (CreateItem)0;
					UUID id = GetUUIDFromName(res + ".trcac");
					if (id == 0)
					{
						std::string clip_path = (m_currentDir / (res + ".trcac")).string();
						Ref<AnimationClip> clip = AnimationsManager::get_instance()->LoadClip_(clip_path);
						AnimationsSerializer::SerializeAnimationClip(clip, clip_path);
						OnDirectoryChanged();
					}
					else
					{
						TRC_ERROR("{} has already been created", res + ".trcac");
					}
				}
			}
			else c_item = (CreateItem)0;
			ProcessAllDirectory();
			break;
		}
		case ANIMATION_GRAPH:
		{
			std::string res;
			if (editor->InputTextPopup("Animation Graph Name", res))
			{
				if (!res.empty())
				{
					c_item = (CreateItem)0;
					UUID id = GetUUIDFromName(res + ".trcag");
					if (id == 0)
					{
						std::string graph_path = (m_currentDir / (res + ".trcag")).string();
						Ref<AnimationGraph> graph = AnimationsManager::get_instance()->LoadGraph_(graph_path);
						AnimationsSerializer::SerializeAnimationGraph(graph, graph_path);
						OnDirectoryChanged();
					}
					else
					{
						TRC_ERROR("{} has already been created", res + ".trcag");
					}
				}
			}
			else c_item = (CreateItem)0;
			ProcessAllDirectory();
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
		TraceEditor* editor = TraceEditor::get_instance();
		Ref<MaterialInstance> mat = m_assetsEdit.editMaterial;
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
			if (popup = editor->DrawPipelinesPopup(pipe_res))
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
							m_assetsEdit.editMaterialPipeChanged = true;
						}
						else
						{
							m_assetsEdit.editMaterial = MaterialSerializer::Deserialize(m_assetsEdit.editMaterialPath.string());
							mat = m_assetsEdit.editMaterial;
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
				ImGui::DragFloat2(name.c_str(), glm::value_ptr(data), 0.15f);
				dst = data;
				break;
			}
			case trace::ShaderData::CUSTOM_DATA_VEC3:
			{
				glm::vec3& data = std::any_cast<glm::vec3&>(dst);
				ImGui::DragFloat3(name.c_str(), glm::value_ptr(data), 0.15f);
				dst = data;
				break;
			}
			case trace::ShaderData::CUSTOM_DATA_VEC4:
			{
				glm::vec4& data = std::any_cast<glm::vec4&>(dst);
				ImGui::DragFloat4(name.c_str(), glm::value_ptr(data), 0.15f);
				dst = data;
				break;
			}
			}
		};

		for (auto& m_data : mat->GetMaterialData())
		{
			trace::UniformMetaData& meta_data = mat->GetRenderPipline()->GetSceneUniforms()[m_data.second.second];
			lambda(meta_data.data_type, m_data.second.first, m_data.first);
		}

		// Select Texture
		if (tex_modified)
		{
			std::string tex_res;
			if (editor->DrawTexturesPopup(tex_res))
			{
				if (!tex_res.empty())
				{
					std::filesystem::path p = tex_res;
					UUID id = m_allFilesID[p.filename().string()];
					Ref<GTexture> tex_r = LoadTexture(id);
					if (tex_r)
					{
						mat->GetMaterialData()[tex_name].first = tex_r;
						dirty = true;
					}
					tex_modified = false;
				}
			}
			else tex_modified = false;
		}

		if(dirty) RenderFunc::PostInitializeMaterial(mat.get(), mat->GetRenderPipline());

		if (ImGui::Button("Apply"))
		{
			m_assetsEdit.materialDataCache = mat->GetMaterialData();
			m_assetsEdit.editMaterialPipe = mat->GetRenderPipline();
			m_assetsEdit.editMaterialPipeChanged = false;
			MaterialSerializer::Serialize(mat, m_assetsEdit.editMaterialPath.string());
		}

	}
	void ContentBrowser::DrawEditFont()
	{
		if (m_assetsEdit.editFont)
		{
			ImGui::Text("Font Name: %s", m_assetsEdit.editFont->GetName().c_str());
			ImGui::Text("Font Atlas : ");
			void* texture = nullptr;
			UIFunc::GetDrawTextureHandle(m_assetsEdit.editFont->GetAtlas().get(), texture);
			ImVec2 content_ava = ImGui::GetContentRegionAvail();
			ImGui::Image(texture, { content_ava.x, content_ava.y * 0.65f }, { 0.0f, 1.0f }, { 1.0f, 0.0f });
		}
	}
	void ContentBrowser::DrawEditPipeline()
	{
		TraceEditor* editor = TraceEditor::get_instance();
		ImGui::Text("Name : %s ", m_assetsEdit.editPipeline->GetName().c_str());

		static ShaderStage shad_stage = ShaderStage::STAGE_NONE;

		static bool shad_pop = false;

		//Input Layout
		{
			ImGui::InputInt("Stride", (int*)&m_assetsEdit.editPipeDesc.input_layout.stride);
			const char* type_string[] = { "Vertex", "Instance" };
			const char* current_type = type_string[(int)m_assetsEdit.editPipeDesc.input_layout.input_class];
			if (ImGui::BeginCombo("Input Classification", current_type))
			{
				for (int i = 0; i < 2; i++)
				{
					bool selected = (current_type == type_string[i]);
					if (ImGui::Selectable(type_string[i], selected))
					{
						m_assetsEdit.editPipeDesc.input_layout.input_class = (InputClassification)i;
					}

					if (selected)
						ImGui::SetItemDefaultFocus();
				}

				ImGui::EndCombo();
			}

		};

		ImGui::Text("Vertex Shader : ");
		ImGui::SameLine();
		if (ImGui::Button(m_assetsEdit.editPipeDesc.vertex_shader->GetName().c_str()))
		{
			shad_pop = true;
			shad_stage = ShaderStage::VERTEX_SHADER;
		}

		ImGui::Text("Pixel Shader : ");
		ImGui::SameLine();
		if (ImGui::Button(m_assetsEdit.editPipeDesc.pixel_shader->GetName().c_str()))
		{
			shad_pop = true;
			shad_stage = ShaderStage::PIXEL_SHADER;
		}

		if (shad_pop)
		{
			std::string shad_res;
			if (shad_pop = editor->DrawShadersPopup(shad_res))
			{
				if (!shad_res.empty())
				{
					Ref<GShader> res = ShaderManager::get_instance()->CreateShader_(shad_res, shad_stage);
					if (TRC_HAS_FLAG(shad_stage, ShaderStage::VERTEX_SHADER))
						m_assetsEdit.editPipeDesc.vertex_shader = res.get();
					if (TRC_HAS_FLAG(shad_stage, ShaderStage::PIXEL_SHADER))
						m_assetsEdit.editPipeDesc.pixel_shader = res.get();

					shad_pop = false;
				}
			}
		}

		if (ImGui::Button("Apply"))
		{
			if (PipelineManager::get_instance()->RecreatePipeline(m_assetsEdit.editPipeline, m_assetsEdit.editPipeDesc))
			{
				m_assetsEdit.editPipeline->SetPipelineType(m_assetsEdit.editPipeType);
				PipelineSerializer::Serialize(m_assetsEdit.editPipeline, m_assetsEdit.editPipePath.string());
			}
		}

	}
	void ContentBrowser::SetDirectory(const std::string& dir)
	{
		m_currentDir = dir;
		OnDirectoryChanged();
		ProcessAllDirectory();
	}
	void ContentBrowser::LoadImportedAssets()
	{
		TraceEditor* editor = TraceEditor::get_instance();
		std::filesystem::path import_path = std::filesystem::path(editor->GetCurrentProject()->GetProjectCurrentDirectory()) / "InternalAssetsDB";
		std::filesystem::path importDB_path = import_path / "imports.trdb";

		if (!std::filesystem::exists(importDB_path))
		{
			return;
		}

		YAML::Node data;
		YAML::load_yaml_data(importDB_path.string(), data);

		std::string trace_version = data["Trace Version"].as<std::string>(); // TODO: To be used later

		YAML::Node DATA = data["DATA"];
		for (auto i : DATA)
		{
			std::string filename = i["Name"].as<std::string>();
			m_importedAssets.insert(filename);
		}

	}
	void ContentBrowser::SerializeImportedAssets()
	{
		TraceEditor* editor = TraceEditor::get_instance();

		std::filesystem::path import_path = std::filesystem::path(editor->GetCurrentProject()->GetProjectCurrentDirectory()) / "InternalAssetsDB";
		std::filesystem::path importDB_path = import_path / "imports.trdb";

		YAML::Emitter emit;

		emit << YAML::BeginMap;
		emit << YAML::Key << "Trace Version" << YAML::Value << "0.0.0.0";
		emit << YAML::Key << "DATA" << YAML::Value << YAML::BeginSeq;
		for (const std::string& imported_asset : m_importedAssets)
		{
			emit << YAML::BeginMap;
			emit << YAML::Key << "Name" << YAML::Value << imported_asset;
			emit << YAML::EndMap;
		}

		emit << YAML::EndSeq;

		emit << YAML::EndMap;

		YAML::save_emitter_data(emit, importDB_path.string());
	}
}