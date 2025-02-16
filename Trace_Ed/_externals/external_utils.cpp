
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
		//TODO: Add Error Handling
		TraceEditor* editor = TraceEditor::get_instance();
		return editor->GetContentBrowser()->GetUUIDName()[uuid];
	}

	Ref<Model> LoadModel(UUID uuid, std::string& model_name)
	{
		TraceEditor* editor = TraceEditor::get_instance();
		ContentBrowser* content_browser = editor->GetContentBrowser();
		Importer* importer = editor->GetImporter();

		Ref<Model> model = ModelManager::get_instance()->GetModel(model_name);
		std::filesystem::path p = GetPathFromUUID(uuid);

		if (model)
		{
			return model;
		}
		else
		{

			if (!model_name.empty())
			{
				if (model_name.empty())
				{
					return model;
				}
				return importer->LoadModel(model_name);
			}

		}
		return model;
	}

	Ref<SkinnedModel> LoadSkinnedModel(UUID uuid, std::string& model_name)
	{
		TraceEditor* editor = TraceEditor::get_instance();
		ContentBrowser* content_browser = editor->GetContentBrowser();
		Importer* importer = editor->GetImporter();

		Ref<SkinnedModel> model = GenericAssetManager::get_instance()->Get<SkinnedModel>(model_name);

		if (model)
		{
			return model;
		}
		else
		{

			if (!model_name.empty())
			{
				if (model_name.empty())
				{
					return model;
				}
				return importer->LoadSkinnedModel(model_name);
			}

		}
		return model;
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