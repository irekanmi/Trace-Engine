
#include "ContentBrowser.h"
#include "../TraceEditor.h"
#include "resource/GenericAssetManager.h"
#include "resource/PrefabManager.h"
#include "backends/UIutils.h"
#include "scene/UUID.h"
#include "serialize/MaterialSerializer.h"
#include "serialize/PipelineSerializer.h"
#include "InspectorPanel.h"
#include "serialize/AnimationsSerializer.h"
#include "serialize/SceneSerializer.h"
#include "core/Utils.h"
#include "../utils/ImGui_utils.h"
#include "HierachyPanel.h"
#include "../import/Importer.h"
#include "external_utils.h"
#include "core/defines.h"
#include "shader_graph/ShaderGraph.h"

#include "serialize/GenericSerializer.h"
#include "motion_matching/MotionMatchDatabase.h"
#include "resource/DefaultAssetsManager.h"


#include "imgui.h"
#include "imgui_internal.h"
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "serialize/yaml_util.h"
#include <unordered_map>
#include <functional>
#include <string>

namespace trace {


	enum CreateItem
	{
		MATERIAL = 1,
		PIPELINE,
		FOLDER,
		SCENE,
		ANIMATION_CLIP,
		ANIMATION_GRAPH,
		ANIMATION_SEQUENCE,
		HUMANOID_RIG,
		FEATURE_DB,
		MMT_INFO,
		PARTICLE_EFFECT,
		PARTICLE_GENERATOR,
	};

	static bool rename_file = false;
	static float thumbnail_size = 96.0f;
	std::unordered_map<std::string, std::function<void(std::filesystem::path&)>> process_callbacks;
	std::unordered_map<std::string, std::function<void(std::filesystem::path&)>> extensions_callbacks;

	bool ContentBrowser::Init()
	{
		TraceEditor* editor = TraceEditor::get_instance();
		if(editor->GetCurrentProject()) m_currentDir = editor->GetCurrentProject()->GetAssetsDirectory();

		directory_icon = GenericAssetManager::get_instance()->CreateAssetHandle<GTexture>("directory.png", DefaultAssetsManager::assets_path + "/textures" + "/directory.png");
		default_icon = GenericAssetManager::get_instance()->CreateAssetHandle<GTexture>("default_file_icon.png", DefaultAssetsManager::assets_path + "/textures" + "/default_file_icon.png");


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
			process_callbacks[MATERIAL_FILE_EXTENSION] = [editor](std::filesystem::path& path)
			{
				editor->GetAllProjectAssets().materials.emplace(path);

			};

			process_callbacks[RENDER_PIPELINE_FILE_EXTENSION] = [editor](std::filesystem::path& path)
			{
				editor->GetAllProjectAssets().pipelines.emplace(path);

			};

			process_callbacks[SCENE_FILE_EXTENSION] = [editor](std::filesystem::path& path)
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

			extensions_callbacks[MATERIAL_FILE_EXTENSION] = [editor](std::filesystem::path& path)
			{
				editor->OpenMaterial(path.string());
			};

			
			extensions_callbacks[SCENE_FILE_EXTENSION] = [editor](std::filesystem::path& path)
			{
				editor->OpenScene(path.string());
			};
			
			extensions_callbacks[PREFAB_FILE_EXTENSION] = [editor](std::filesystem::path& path)
			{
				editor->OpenPrefab(path.string());
			};
			
			extensions_callbacks[ANIMATION_GRAPH_FILE_EXTENSION] = [editor](std::filesystem::path& path)
			{
				editor->OpenAnimationGraph(path.string());
			};
			
			extensions_callbacks[SEQUENCE_FILE_EXTENSION] = [editor](std::filesystem::path& path)
			{
				editor->OpenAnimationSequence(path.string());
			};
			
			extensions_callbacks[ANIMATION_CLIP_FILE_EXTENSION] = [editor](std::filesystem::path& path)
			{
				editor->OpenAnimationClip(path.string());
			};
			
			extensions_callbacks[SHADER_GRAPH_FILE_EXTENSION] = [editor](std::filesystem::path& path)
			{
				editor->OpenShaderGraph(path.string());
			};

			extensions_callbacks[SKELETON_FILE_EXTENSION] = [editor](std::filesystem::path& path)
			{
				editor->OpenSkeleton(path.string());
			};

			extensions_callbacks[FEATURE_DB_FILE_EXTENSION] = [editor](std::filesystem::path& path)
			{
				editor->OpenFeatureDB(path.string());
			};
			
			extensions_callbacks[MMT_INFO_FILE_EXTENSION] = [editor](std::filesystem::path& path)
			{
				editor->OpenMMTInfo(path.string());
			};

			extensions_callbacks[PARTICLE_EFFECT_FILE_EXTENSION] = [editor](std::filesystem::path& path)
			{
				editor->OpenParticleEffect(path.string());
			};

			extensions_callbacks[PARTICLE_GENERATOR_FILE_EXTENSION] = [editor](std::filesystem::path& path)
			{
				editor->OpenParticleGenerator(path.string());
			};

			extensions_callbacks[".ttf"] = [editor](std::filesystem::path& path)
			{
				/*editor->GetInspectorPanel()->SetDrawCallbackFn([editor]() { editor->GetContentBrowser()->DrawEditFont(); },
					[editor, path]()
					{
						AssetsEdit& assets_edit = editor->GetContentBrowser()->GetAssetsEdit();
						assets_edit.editFont = GenericAssetManager::get_instance()->CreateAssetHandle_<Font>(path.string(), path.string());
					},
					[editor]()
					{
						AssetsEdit& assets_edit = editor->GetContentBrowser()->GetAssetsEdit();
						assets_edit.editFont.free();
					});*/
			};

			extensions_callbacks[".TTF"] = [editor](std::filesystem::path& path)
			{
				/*editor->GetInspectorPanel()->SetDrawCallbackFn([editor]() { editor->GetContentBrowser()->DrawEditFont(); },
					[editor, path]()
					{
						AssetsEdit& assets_edit = editor->GetContentBrowser()->GetAssetsEdit();
						assets_edit.editFont = GenericAssetManager::get_instance()->CreateAssetHandle_<Font>(path.string(), path.string());
					},
					[editor]()
					{
						AssetsEdit& assets_edit = editor->GetContentBrowser()->GetAssetsEdit();
						assets_edit.editFont.free();
					});*/
			};

			extensions_callbacks[RENDER_PIPELINE_FILE_EXTENSION] = [editor](std::filesystem::path& path)
			{
				/*editor->GetInspectorPanel()->SetDrawCallbackFn([editor]() { editor->GetContentBrowser()->DrawEditPipeline(); },
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
					});*/
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
			extensions_callbacks[".bvh"] = mesh_lambda;
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
		TraceEditor* editor = TraceEditor::get_instance();
		if (editor->GetCurrentProject())
		{
			std::filesystem::path db_path = std::filesystem::path(editor->GetCurrentProject()->GetProjectCurrentDirectory()) / "InternalAssetsDB";
			std::filesystem::path assetsDB_path = db_path / "assets.trdb";

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
				uintptr_t scene_loc = 0;
				UUID uuid = *(UUID*)payload->Data;
				memcpy(&uuid, payload->Data, sizeof(UUID));
				memcpy(&scene_loc, (char*)payload->Data + sizeof(UUID), sizeof(uintptr_t));
				Scene* src_scene = (Scene*)scene_loc;
				prefab_entity = src_scene->GetEntity(uuid);
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
						Ref<Prefab> prefab = GenericAssetManager::get_instance()->CreateAssetHandle<Prefab>(res + ".trprf", prefab_entity);
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
			}
			else
			{
				prefab_popup = false;
			}
		}

		std::string new_filename;
		if (rename_file && !m_renamePath.empty() && editor->InputTextPopup("New File Name", new_filename))
		{
			if (!new_filename.empty())
			{
				RenameFile(m_renamePath, new_filename);
				rename_file = false;
				m_renamePath = "";
			}
		}
		else
		{
			rename_file = false;
			m_renamePath = "";
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

	static std::vector<std::string> built_in_assets = {
		"albedo_map",
		"specular_map",
		"normal_map",

		"Cube",
		"Sphere",

		"default",
		"Plane",

		"gbuffer_pipeline",
		"skybox_pipeline",
		"light_pipeline",
		"quad_batch_pipeline",
		"text_batch_pipeline",
		"text_pipeline",
		"debug_line_pipeline",
		"bloom_prefilter_pass_pipeline",
		"bloom_downsample_pass_pipeline",
		"bloom_upsample_pass_pipeline",
		"lighting_pass_pipeline",
		"ssao_main_pass_pipeline",
		"ssao_blur_pass_pipeline",
		"tone_map_pass_pipeline",
		"weightedOIT_pass_pipeline",
		"black_texture",
		"skinned_gbuffer_pipeline",
		"transparent_texture",
		"sun_shadow_map_pass_pipeline",
		"spot_shadow_map_pass_pipeline",
		"sun_shadow_skinned_map_pass_pipeline",
		"spot_shadow_skinned_map_pass_pipeline",
		"particle_billboard_pipeline",
		"particle_velocity_aligned_pipeline"
	};

	void ContentBrowser::ProcessAllDirectory(bool update_assets_db)
	{
		TraceEditor* editor = TraceEditor::get_instance();

		LoadImportedAssets();

		/*m_allFilesID.clear();
		m_allIDPath.clear();*/

		for (std::string& asset : built_in_assets)
		{
			UUID id = STR_ID(asset);
			m_allFilesID[asset] = id;
			m_allIDPath[id] = asset;
		}

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
				if (id == 0)
				{
					continue;
				}
				m_allFilesID[filename] = id;
				m_allIDNames[id] = filename;
			}

		}
		else if (!std::filesystem::exists(db_path))
		{
			std::filesystem::create_directory(db_path);
		}

		bool dirty_ids = false;

		// Adding builtin Shaders ================
		std::string shader_dir;
		FindDirectory(AppSettings::exe_path, "assets/shaders", shader_dir);
		TraceEditor::AllProjectAssets& all_assets = TraceEditor::get_instance()->GetAllProjectAssets();
		for (auto r : std::filesystem::recursive_directory_iterator(shader_dir))
		{
			std::filesystem::path path = r.path();
			if (std::filesystem::is_regular_file(r.path()) && path.extension().string() != ".shcode")
			{
				all_assets.shaders.emplace(r.path());
				std::string filename = r.path().filename().string();
				UUID id = STR_ID(filename);
				m_allFilesID[filename] = id;
				m_allIDPath[id] = r.path();
				dirty_ids = true;
			}
		}
		// =======================================

		for (auto r : std::filesystem::recursive_directory_iterator(editor->GetCurrentProject()->GetAssetsDirectory()))
		{
			std::filesystem::path path = r.path();
			if (std::filesystem::is_regular_file(r.path()))
			{
				std::string filename = r.path().filename().string();
				UUID id = STR_ID(filename);
				m_allFilesID[filename] = id;
				m_allIDPath[id] = r.path();
			}
		}

		for (auto i : m_allFilesID)
		{
			m_allIDNames[i.second] = i.first;
		}


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
	void ContentBrowser::OnWindowPopup()
	{
		TraceEditor* editor = TraceEditor::get_instance();
		

		static CreateItem c_item = (CreateItem)0;

		bool create_shader_graph = false;

		if (ImGui::BeginPopupContextWindow("Content_Browser_Window", ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
		{
			if (ImGui::BeginMenu("Create"))
			{
				if (ImGui::MenuItem("Material"))
				{
					c_item = MATERIAL;

				}
				if (ImGui::MenuItem("Shader Graph"))
				{
					create_shader_graph = true;

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
				if (ImGui::MenuItem("Animation Sequence"))
				{
					c_item = ANIMATION_SEQUENCE;
				}
				if (ImGui::MenuItem("Humanoid Rig"))
				{
					c_item = HUMANOID_RIG;
				}
				if (ImGui::MenuItem("Feature Database"))
				{
					c_item = FEATURE_DB;
				}
				if (ImGui::MenuItem("Motion Matching Info"))
				{
					c_item = MMT_INFO;
				}
				if (ImGui::MenuItem("Particle Effect"))
				{
					c_item = PARTICLE_EFFECT;
				}
				if (ImGui::MenuItem("Particle Generator"))
				{
					c_item = PARTICLE_GENERATOR;
				}
				ImGui::EndMenu();
			}

			ImGui::EndPopup();
		}

		static auto lambda = [&](UUID id)
		{

		};

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
					ProcessAllDirectory();
					OnDirectoryChanged();
				}
			}
			else c_item = (CreateItem)0;
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
					UUID id = GetUUIDFromName(res + MATERIAL_FILE_EXTENSION);
					if (id == 0)
					{
						Ref<GPipeline> sp = GenericAssetManager::get_instance()->Get<GPipeline>("gbuffer_pipeline");
						Ref<MaterialInstance> mat = GenericAssetManager::get_instance()->CreateAssetHandle<MaterialInstance>(res + MATERIAL_FILE_EXTENSION, sp);
						if (mat)
						{
							std::string path = (m_currentDir / (res + MATERIAL_FILE_EXTENSION)).string();
							MaterialSerializer::Serialize(mat, path);
							std::string filename = res + MATERIAL_FILE_EXTENSION;
							UUID new_id = STR_ID(filename);
							m_allFilesID[res + MATERIAL_FILE_EXTENSION] = new_id;
							m_allIDPath[new_id] = path;
							ProcessAllDirectory(true);
							OnDirectoryChanged();
						}
					}
					else
					{
						TRC_ERROR("{} has already been created", res + MATERIAL_FILE_EXTENSION);
					}
				}
			}
			else c_item = (CreateItem)0;
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
					UUID id = GetUUIDFromName(res + RENDER_PIPELINE_FILE_EXTENSION);
					if (id == 0)
					{
						Ref<GPipeline> sp = GenericAssetManager::get_instance()->Get<GPipeline>("gbuffer_pipeline");
						Ref<GPipeline> new_pipeline = GenericAssetManager::get_instance()->CreateAssetHandle<GPipeline>(res + RENDER_PIPELINE_FILE_EXTENSION, sp->GetDesc(), false);
						new_pipeline->SetPipelineType(PipelineType::Surface_Material);
						std::string path = (m_currentDir / (res + RENDER_PIPELINE_FILE_EXTENSION)).string();
						PipelineSerializer::Serialize(new_pipeline, path);
						std::string filename = res + RENDER_PIPELINE_FILE_EXTENSION;
						UUID new_id = STR_ID(filename);
						m_allFilesID[res + RENDER_PIPELINE_FILE_EXTENSION] = new_id;
						m_allIDPath[new_id] = path;
						ProcessAllDirectory(true);
						OnDirectoryChanged();
					}
					else
					{
						TRC_ERROR("{} has already been created", res + RENDER_PIPELINE_FILE_EXTENSION);
					}
				}
			}
			else c_item = (CreateItem)0;
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
					UUID id = GetUUIDFromName(res + SCENE_FILE_EXTENSION);
					if (id == 0)
					{
						std::string scene_path = (m_currentDir / (res + SCENE_FILE_EXTENSION)).string();
						Ref<Scene> asset = GenericAssetManager::get_instance()->CreateAssetHandle_<Scene>(scene_path);
						SceneSerializer::Serialize(asset, scene_path);
						std::string filename = res + SCENE_FILE_EXTENSION;
						UUID new_id = STR_ID(filename);
						m_allFilesID[res + SCENE_FILE_EXTENSION] = new_id;
						m_allIDPath[new_id] = scene_path;
						ProcessAllDirectory(true);
						OnDirectoryChanged();
					}
					else
					{
						TRC_ERROR("{} has already been created", res + SCENE_FILE_EXTENSION);
					}
				}
			}
			else c_item = (CreateItem)0;
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
					UUID id = GetUUIDFromName(res + ANIMATION_CLIP_FILE_EXTENSION);

					if (id == 0)
					{
						std::string clip_path = (m_currentDir / (res + ANIMATION_CLIP_FILE_EXTENSION)).string();
						Ref<AnimationClip> clip = GenericAssetManager::get_instance()->CreateAssetHandle_<AnimationClip>(clip_path);
						AnimationsSerializer::SerializeAnimationClip(clip, clip_path);
						std::string filename = res + ANIMATION_CLIP_FILE_EXTENSION;
						UUID new_id = STR_ID(filename);
						m_allFilesID[res + ANIMATION_CLIP_FILE_EXTENSION] = new_id;
						m_allIDPath[new_id] = clip_path;
						ProcessAllDirectory(true);
						OnDirectoryChanged();
					}
					else
					{
						TRC_ERROR("{} has already been created", res + ANIMATION_CLIP_FILE_EXTENSION);
					}
				}
			}
			else c_item = (CreateItem)0;

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
					UUID id = GetUUIDFromName(res + ANIMATION_GRAPH_FILE_EXTENSION);
					if (id == 0)
					{
						std::string graph_path = (m_currentDir / (res + ANIMATION_GRAPH_FILE_EXTENSION)).string();
						Ref<Animation::Graph> graph = GenericAssetManager::get_instance()->CreateAssetHandle_<Animation::Graph>(graph_path);
						AnimationsSerializer::SerializeAnimGraph(graph, graph_path);
						std::string filename = res + ANIMATION_GRAPH_FILE_EXTENSION;
						UUID new_id = STR_ID(filename);
						m_allFilesID[res + ANIMATION_GRAPH_FILE_EXTENSION] = new_id;
						m_allIDPath[new_id] = graph_path;
						ProcessAllDirectory(true);
						OnDirectoryChanged();
					}
					else
					{
						TRC_ERROR("{} has already been created", res + ANIMATION_GRAPH_FILE_EXTENSION);
					}
				}
			}
			else c_item = (CreateItem)0;
			break;
		}
		case ANIMATION_SEQUENCE:
		{
			std::string res;
			if (editor->InputTextPopup("Animation Sequence Name", res))
			{
				if (!res.empty())
				{
					c_item = (CreateItem)0;
					UUID id = GetUUIDFromName(res + SEQUENCE_FILE_EXTENSION);

					if (id == 0)
					{
						std::string sequence_path = (m_currentDir / (res + SEQUENCE_FILE_EXTENSION)).string();
						Ref<Animation::Sequence> sequence = GenericAssetManager::get_instance()->CreateAssetHandle_<Animation::Sequence>(sequence_path);
						AnimationsSerializer::SerializeSequence(sequence, sequence_path);
						std::string filename = res + SEQUENCE_FILE_EXTENSION;
						UUID new_id = STR_ID(filename);
						m_allFilesID[res + SEQUENCE_FILE_EXTENSION] = new_id;
						m_allIDPath[new_id] = sequence_path;
						ProcessAllDirectory(true);
						OnDirectoryChanged();
					}
					else
					{
						TRC_ERROR("{} has already been created", res + SEQUENCE_FILE_EXTENSION);
					}
				}
			}
			else c_item = (CreateItem)0;
			break;
		}
		case HUMANOID_RIG:
		{
			std::string res;
			if (editor->InputTextPopup("Humanoid Rig Name", res))
			{
				if (!res.empty())
				{
					c_item = (CreateItem)0;
					UUID id = GetUUIDFromName(res + HUMANOID_RIG_FILE_EXTENSION);

					if (id == 0)
					{
						std::string rig_path = (m_currentDir / (res + HUMANOID_RIG_FILE_EXTENSION)).string();
						Ref<Animation::HumanoidRig> rig = GenericAssetManager::get_instance()->CreateAssetHandle_<Animation::HumanoidRig>(rig_path);
						AnimationsSerializer::SerializeHumanoidRig(rig, rig_path);
						std::string filename = res + HUMANOID_RIG_FILE_EXTENSION;
						UUID new_id = STR_ID(filename);
						m_allFilesID[res + HUMANOID_RIG_FILE_EXTENSION] = new_id;
						m_allIDPath[new_id] = rig_path;
						ProcessAllDirectory(true);
						OnDirectoryChanged();
					}
					else
					{
						TRC_ERROR("{} has already been created", res + HUMANOID_RIG_FILE_EXTENSION);
					}
				}
			}
			else c_item = (CreateItem)0;
			break;
		}
		case FEATURE_DB:
		{
			std::string res;
			if (editor->InputTextPopup("Database Name", res))
			{
				if (!res.empty())
				{
					c_item = (CreateItem)0;
					UUID id = GetUUIDFromName(res + FEATURE_DB_FILE_EXTENSION);

					if (id == 0)
					{
						std::string db_path = (m_currentDir / (res + FEATURE_DB_FILE_EXTENSION)).string();
						Ref<MotionMatching::FeatureDatabase> db = GenericAssetManager::get_instance()->CreateAssetHandle_<MotionMatching::FeatureDatabase>(db_path);
						GenericSerializer::Serialize<MotionMatching::FeatureDatabase>(db, db_path);
						std::string filename = res + FEATURE_DB_FILE_EXTENSION;
						UUID new_id = STR_ID(filename);
						m_allFilesID[res + FEATURE_DB_FILE_EXTENSION] = new_id;
						m_allIDPath[new_id] = db_path;
						ProcessAllDirectory(true);
						OnDirectoryChanged();
					}
					else
					{
						TRC_ERROR("{} has already been created", res + FEATURE_DB_FILE_EXTENSION);
					}
				}
			}
			else c_item = (CreateItem)0;
			break;
		}
		case MMT_INFO:
		{
			std::string res;
			if (editor->InputTextPopup("MMT Info Name", res))
			{
				if (!res.empty())
				{
					c_item = (CreateItem)0;
					UUID id = GetUUIDFromName(res + MMT_INFO_FILE_EXTENSION);

					if (id == 0)
					{
						std::string info_path = (m_currentDir / (res + MMT_INFO_FILE_EXTENSION)).string();
						Ref<MotionMatching::MotionMatchingInfo> info = GenericAssetManager::get_instance()->CreateAssetHandle_<MotionMatching::MotionMatchingInfo>(info_path);
						GenericSerializer::Serialize<MotionMatching::MotionMatchingInfo>(info, info_path);
						std::string filename = res + MMT_INFO_FILE_EXTENSION;
						UUID new_id = STR_ID(filename);
						m_allFilesID[res + MMT_INFO_FILE_EXTENSION] = new_id;
						m_allIDPath[new_id] = info_path;
						ProcessAllDirectory(true);
						OnDirectoryChanged();
					}
					else
					{
						TRC_ERROR("{} has already been created", res + MMT_INFO_FILE_EXTENSION);
					}
				}
			}
			else c_item = (CreateItem)0;
			break;
		}
		case PARTICLE_EFFECT:
		{
			std::string res;
			if (editor->InputTextPopup("Particle Effect Name", res))
			{
				if (!res.empty())
				{
					c_item = (CreateItem)0;
					UUID id = GetUUIDFromName(res + PARTICLE_EFFECT_FILE_EXTENSION);

					if (id == 0)
					{
						std::string asset_path = (m_currentDir / (res + PARTICLE_EFFECT_FILE_EXTENSION)).string();
						Ref<ParticleEffect> asset = GenericAssetManager::get_instance()->CreateAssetHandle_<ParticleEffect>(asset_path);
						GenericSerializer::Serialize<ParticleEffect>(asset, asset_path);
						std::string filename = res + PARTICLE_EFFECT_FILE_EXTENSION;
						UUID new_id = STR_ID(filename);
						m_allFilesID[res + PARTICLE_EFFECT_FILE_EXTENSION] = new_id;
						m_allIDPath[new_id] = asset_path;
						ProcessAllDirectory(true);
						OnDirectoryChanged();
					}
					else
					{
						TRC_ERROR("{} has already been created", res + PARTICLE_EFFECT_FILE_EXTENSION);
					}
				}
			}
			else c_item = (CreateItem)0;
			break;
		}
		case PARTICLE_GENERATOR:
		{
			std::string res;
			if (editor->InputTextPopup("Particle Generator Name", res))
			{
				if (!res.empty())
				{
					c_item = (CreateItem)0;
					UUID id = GetUUIDFromName(res + PARTICLE_GENERATOR_FILE_EXTENSION);

					if (id == 0)
					{
						std::string asset_path = (m_currentDir / (res + PARTICLE_GENERATOR_FILE_EXTENSION)).string();
						Ref<ParticleGenerator> asset = GenericAssetManager::get_instance()->CreateAssetHandle_<ParticleGenerator>(asset_path, nullptr);
						GenericSerializer::Serialize<ParticleGenerator>(asset, asset_path);
						std::string filename = res + PARTICLE_GENERATOR_FILE_EXTENSION;
						UUID new_id = STR_ID(filename);
						m_allFilesID[res + PARTICLE_GENERATOR_FILE_EXTENSION] = new_id;
						m_allIDPath[new_id] = asset_path;
						ProcessAllDirectory(true);
						OnDirectoryChanged();
					}
					else
					{
						TRC_ERROR("{} has already been created", res + PARTICLE_GENERATOR_FILE_EXTENSION);
					}
				}
			}
			else c_item = (CreateItem)0;
			break;
		}
		}

		static MaterialType shader_graph_type = MaterialType::OPAQUE_LIT;

		if (create_shader_graph)
		{
			ImGui::OpenPopup("Shader Graph Create");
		}

		if (ImGui::BeginPopupModal("Shader Graph Create", NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			static std::string asset_name;
			ImGui::InputText("Shader Graph Name", &asset_name);
			
			char* shader_type_string[] =
			{
				"None",
				"Opaque Lit",
				"Opaque UnLit",
				"Transparent Lit",
				"Transparent UnLit",
				"Particle Lit",
				"Particle UnLit",
				"Particle Billboard",
			};

			if (ImGui::BeginCombo("Material Type", shader_type_string[(int32_t)shader_graph_type]))
			{
				for (uint32_t j = 1; j < 8; j++)
				{
					bool selected = (shader_graph_type == (MaterialType)j);
					if (ImGui::Selectable(shader_type_string[j], selected))
					{
						shader_graph_type = (MaterialType)j;
					}

					if (selected)
					{
						ImGui::SetItemDefaultFocus();
					}
				}
				ImGui::EndCombo();
			}

			if (ImGui::Button("Create"))
			{
				UUID id = GetUUIDFromName(asset_name + SHADER_GRAPH_FILE_EXTENSION);

				if (id == 0)
				{
					std::string asset_path = (m_currentDir / (asset_name + SHADER_GRAPH_FILE_EXTENSION)).string();
					Ref<ShaderGraph> asset = GenericAssetManager::get_instance()->CreateAssetHandle_<ShaderGraph>(asset_path, shader_graph_type);
					if (asset)
					{
						GenericSerializer::Serialize<ShaderGraph>(asset, asset_path);
						std::string filename = asset_name + SHADER_GRAPH_FILE_EXTENSION;
						UUID new_id = STR_ID(filename);
						m_allFilesID[asset_name + SHADER_GRAPH_FILE_EXTENSION] = new_id;
						m_allIDPath[new_id] = asset_path;
						ProcessAllDirectory(true);
						OnDirectoryChanged();
					}
				}
				else
				{
					TRC_ERROR("{} has already been created", asset_name + SHADER_GRAPH_FILE_EXTENSION);
				}

				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel"))
			{
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}

	}
	void ContentBrowser::OnItemPopup(std::filesystem::path& path)
	{
		static std::string prev_file_name = "";
		bool is_directory = std::filesystem::is_directory(path);
		std::string filename = path.filename().string();

		TraceEditor* editor = TraceEditor::get_instance();

		static CreateItem s_item = (CreateItem)0;
		static std::string file_path = "";

		if (ImGui::BeginPopupContextItem())
		{
			if(ImGui::MenuItem("Rename"))
			{
				rename_file = true && !is_directory;//TODO: Add logic to rename directories
				m_renamePath = path;
			}
			if(ImGui::MenuItem("Delete")){}
			if (path.extension() == SHADER_GRAPH_FILE_EXTENSION)
			{
				if (ImGui::MenuItem("Create Material"))
				{
					s_item = CreateItem::MATERIAL;
					file_path = path.string();
				}
			}


			ImGui::EndPopup();
		}

		switch (s_item)
		{
		case MATERIAL:
		{
			if (file_path != path.string())
			{
				break;
			}
			std::string res;
			if (editor->InputTextPopup("Material Name", res))
			{
				if (!res.empty())
				{
					s_item = (CreateItem)0;
					UUID id = GetUUIDFromName(res + MATERIAL_FILE_EXTENSION);
					if (id == 0)
					{
						Ref<ShaderGraph> shader_graph = GenericSerializer::Deserialize<ShaderGraph>(file_path);
						Ref<GPipeline> sp = shader_graph->GetPipeline();
						UUID sp_id = GetUUIDFromName(shader_graph->GetName() + RENDER_PIPELINE_FILE_EXTENSION);
						if (sp_id == 0 && sp)
						{
							PipelineSerializer::Serialize(sp, file_path + RENDER_PIPELINE_FILE_EXTENSION);
						}
						Ref<MaterialInstance> mat = GenericAssetManager::get_instance()->CreateAssetHandle<MaterialInstance>(res + MATERIAL_FILE_EXTENSION, sp);
						if (mat)
						{
							std::string path = (m_currentDir / (res + MATERIAL_FILE_EXTENSION)).string();
							MaterialSerializer::Serialize(mat, path);
							std::string filename = res + MATERIAL_FILE_EXTENSION;
							UUID new_id = STR_ID(filename);
							m_allFilesID[res + MATERIAL_FILE_EXTENSION] = new_id;
							m_allIDPath[new_id] = path;
							ProcessAllDirectory(true);
							OnDirectoryChanged();
						}

						file_path = "";
					}
					else
					{
						TRC_ERROR("{} has already been created", res + MATERIAL_FILE_EXTENSION);
					}
				}
			}
			else s_item = (CreateItem)0;
			break;
		}
		}

	}
	void ContentBrowser::RenameFile(std::filesystem::path& path, std::string& new_name)
	{
		std::string filename = path.filename().string();
		std::string extension = path.extension().string();

		std::string new_filename = new_name + extension;

		std::filesystem::path new_path = path.parent_path() / new_filename;


		UUID file_id = m_allFilesID[filename];
		m_allFilesID.erase(filename);
		m_allFilesID[new_filename] = file_id;
		
		m_allIDNames[file_id] = new_filename;
		m_allIDPath[file_id] = new_path;

		std::filesystem::rename(path, new_path);

		if (extension == ANIMATION_CLIP_FILE_EXTENSION)
		{
			Ref<AnimationClip> clip = GenericAssetManager::get_instance()->Get<AnimationClip>(filename);

			if (clip)
			{
				GenericAssetManager::get_instance()->RenameAsset(clip, new_filename);
				clip->m_path = new_path;
			}
		}
		else if (extension == ANIMATION_GRAPH_FILE_EXTENSION)
		{
			Ref<Animation::Graph> asset = GenericAssetManager::get_instance()->Get<Animation::Graph>(filename);

			if (asset)
			{
				GenericAssetManager::get_instance()->RenameAsset<Animation::Graph>(asset, new_filename);
				asset->m_path = new_path;
			}
		}
		else if (extension == PREFAB_FILE_EXTENSION)
		{
			Ref<Prefab> asset = GenericAssetManager::get_instance()->Get<Prefab>(filename);

			if (asset)
			{
				GenericAssetManager::get_instance()->RenameAsset(asset, new_filename);
				asset->m_path = new_path;
			}
		}
		else if (extension == MATERIAL_FILE_EXTENSION)
		{
			Ref<MaterialInstance> asset = GenericAssetManager::get_instance()->Get<MaterialInstance>(filename);

			if (asset)
			{
				GenericAssetManager::get_instance()->RenameAsset(asset, new_filename);
				asset->m_path = new_path;
			}
		}
		else if (extension == RENDER_PIPELINE_FILE_EXTENSION)
		{
			Ref<GPipeline> asset = GenericAssetManager::get_instance()->Get<GPipeline>(filename);

			if (asset)
			{
				GenericAssetManager::get_instance()->RenameAsset(asset, new_filename);
				asset->m_path = new_path;
			}
		}
		else if (extension == SKELETON_FILE_EXTENSION)
		{
			Ref<Animation::Skeleton> asset = GenericAssetManager::get_instance()->Get<Animation::Skeleton>(filename);

			if (asset)
			{
				GenericAssetManager::get_instance()->RenameAsset<Animation::Skeleton>(asset, new_filename);
				asset->m_path = new_path;
			}
		}
		else if (extension == SEQUENCE_FILE_EXTENSION)
		{
			Ref<Animation::Sequence> asset = GenericAssetManager::get_instance()->Get<Animation::Sequence>(filename);

			if (asset)
			{
				GenericAssetManager::get_instance()->RenameAsset<Animation::Sequence>(asset, new_filename);
				asset->m_path = new_path;
			}
		}
		else if (extension == SCENE_FILE_EXTENSION)
		{
			Ref<Scene> asset = GenericAssetManager::get_instance()->Get<Scene>(filename);

			if (asset)
			{
				GenericAssetManager::get_instance()->RenameAsset(asset, new_filename);
				asset->m_path = new_path;
			}
		}

		ProcessAllDirectory(true);
		OnDirectoryChanged();
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
					Ref<GPipeline> p_res = GenericAssetManager::get_instance()->Get<GPipeline>(p.filename().string());
					if (!p_res) p_res = PipelineSerializer::Deserialize(p.string());

					if (p_res)
					{
						if (mat->RecreateMaterial(p_res))
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
						Ref<GTexture> tex = GenericAssetManager::get_instance()->TryGet<GTexture>(p.filename().string());
						if (tex) {}
						else tex = GenericAssetManager::get_instance()->CreateAssetHandle_<GTexture>(p.string(), p.string());
						
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
						Ref<GTexture> tex = GenericAssetManager::get_instance()->TryGet<GTexture>(p.filename().string());
						if (tex) {}
						else tex = GenericAssetManager::get_instance()->CreateAssetHandle_<GTexture>(p.string(), p.string());

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
						Ref<GTexture> tex = GenericAssetManager::get_instance()->TryGet<GTexture>(p.filename().string());
						if (tex) {}
						else tex = GenericAssetManager::get_instance()->CreateAssetHandle_<GTexture>(p.string(), p.string());

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
						Ref<GTexture> tex = GenericAssetManager::get_instance()->TryGet<GTexture>(p.filename().string());
						if (tex) {}
						else tex = GenericAssetManager::get_instance()->CreateAssetHandle_<GTexture>(p.string(), p.string());

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
			//trace::UniformMetaData& meta_data = mat->GetRenderPipline()->GetSceneUniforms()[m_data.second.hash];
			lambda(m_data.second.type, m_data.second.internal_data, m_data.first);
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
						mat->GetMaterialData()[tex_name].internal_data = tex_r;
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
					Ref<GShader> res = GenericAssetManager::get_instance()->CreateAssetHandle_<GShader>(shad_res, shad_res, shad_stage);
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
			if (m_assetsEdit.editPipeline->RecreatePipeline(m_assetsEdit.editPipeDesc))
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