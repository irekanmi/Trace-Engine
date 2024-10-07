
#include "../src/TraceEditor.h"
#include "../src/panels/ContentBrowser.h"
#include "scene/UUID.h"
#include "resource/ModelManager.h"
#include "resource/MeshManager.h"
#include "scene/Components.h"
#include "serialize/yaml_util.h"
#include "render/GTexture.h"
#include "resource/Ref.h"
#include "resource/TextureManager.h"
#include "../src/import/Importer.h"
#include "render/SkinnedModel.h"
#include "resource/GenericAssetManager.h"

#include <vector>

namespace trace {

	std::filesystem::path GetPathFromUUID(UUID uuid)
	{
		//TODO: Add Error Handling
		TraceEditor* editor = TraceEditor::get_instance();
		return editor->GetContentBrowser()->GetUUIDPath()[uuid];
	}
	UUID GetUUIDFromName(const std::string& name)
	{
		//TODO: Add Error Handling
		TraceEditor* editor = TraceEditor::get_instance();
		return editor->GetContentBrowser()->GetAllFilesID()[name];
	}

	std::string GetNameFromUUID(UUID uuid)
	{
		return "";
	}

	bool LoadModel(UUID uuid, std::string& filename, ModelComponent& out_model, YAML::Node& comp)
	{
		TraceEditor* editor = TraceEditor::get_instance();
		ContentBrowser* content_browser = editor->GetContentBrowser();
		Importer* importer = editor->GetImporter();

		Ref<Model> model = ModelManager::get_instance()->GetModel(comp["Name"].as<std::string>());
		std::filesystem::path p = GetPathFromUUID(uuid);

		if (model)
		{
			out_model._model = model;
			return true;
		}
		else
		{

			if (!filename.empty())
			{
				if (filename.empty())
				{
					return false;
				}
				out_model._model = importer->LoadModel(filename);
				return true;
			}

		}
		return false;
	}

	bool LoadSkinnedModel(UUID uuid, std::string& filename, SkinnedModelRenderer& out_model, YAML::Node& node)
	{
		TraceEditor* editor = TraceEditor::get_instance();
		ContentBrowser* content_browser = editor->GetContentBrowser();
		Importer* importer = editor->GetImporter();

		Ref<SkinnedModel> model = GenericAssetManager::get_instance()->Get<SkinnedModel>(node["Model Name"].as<std::string>());

		if (model)
		{
			out_model._model = model;
			return true;
		}
		else
		{

			if (!filename.empty())
			{
				if (filename.empty())
				{
					return false;
				}
				out_model._model = importer->LoadSkinnedModel(filename);
				return true;
			}

		}
		return false;
	}

	Ref<GTexture> LoadTexture(UUID uuid)
	{
		TraceEditor* editor = TraceEditor::get_instance();
		ContentBrowser* content_browser = editor->GetContentBrowser();
		Importer* importer = editor->GetImporter();

		Ref<GTexture> result;
		std::filesystem::path p = GetPathFromUUID(uuid);
		result = TextureManager::get_instance()->GetTexture(p.filename().string());
		if (!result && !p.string().empty())
		{
			result = TextureManager::get_instance()->LoadTexture_(p.string());
		}
		else if (!result && p.string().empty())
		{
			std::string filename = content_browser->GetUUIDName()[uuid];
			if (filename.empty())
			{
				return result;
			}
			result = importer->LoadTexture(filename);
		}

		return result;
	}

}