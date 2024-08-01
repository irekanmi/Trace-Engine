
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

	bool LoadModel(UUID uuid, ModelComponent& out_model, YAML::Node& comp)
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

			if (p.string().empty())
			{
				std::string filename = content_browser->GetUUIDName()[uuid];
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