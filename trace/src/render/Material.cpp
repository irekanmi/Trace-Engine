#include "pch.h"

#include "render/GPipeline.h"
#include "Material.h"
#include "core/Coretypes.h"
#include "external_utils.h"
#include "serialize/MaterialSerializer.h"
#include "resource/GenericAssetManager.h"
#include "backends/Renderutils.h"

namespace trace {
	MaterialInstance::MaterialInstance()
	{
	}
	MaterialInstance::~MaterialInstance()
	{
	}

	bool MaterialInstance::Create(Ref<GPipeline> pipeline)
	{
		auto res = GetPipelineMaterialData(pipeline);
		m_data.clear();
		SetMaterialData(res);

		if (!RenderFunc::InitializeMaterial(
			this,
			pipeline
		))
		{
			TRC_WARN("Failed to initialize material, Funtion: {}", __FUNCTION__);
			return false;
		}
		return true;
	}

	void MaterialInstance::Destroy()
	{
		RenderFunc::DestroyMaterial(this);
		m_renderPipeline.free();
	}

	bool MaterialInstance::RecreateMaterial(Ref<GPipeline> pipeline)
	{
		RenderFunc::DestroyMaterial(this);

		auto res = GetPipelineMaterialData(pipeline);
		m_data.clear();
		SetMaterialData(res);

		if (!RenderFunc::InitializeMaterial(
			this,
			pipeline
		))
		{
			TRC_WARN("Failed to initialize material {}", this->GetName());
			this->m_id = INVALID_ID;
			this->GetRenderHandle()->m_internalData = nullptr;
			return false;
		}

		//RenderFunc::PostInitializeMaterial(mat.get(), pipeline);

		return true;
	}

	Ref<MaterialInstance> MaterialInstance::Deserialize(UUID id)
	{
		Ref<MaterialInstance> result;

		if (AppSettings::is_editor)
		{

			std::filesystem::path p = GetPathFromUUID(id);
			result = MaterialSerializer::Deserialize(p.string());
		}
		else
		{
			result = GenericAssetManager::get_instance()->Load_Runtime<MaterialInstance>(id);
		}

		return result;
	}
	Ref<MaterialInstance> MaterialInstance::Deserialize(DataStream* stream)
	{
		return MaterialSerializer::Deserialize(stream);
	}
}