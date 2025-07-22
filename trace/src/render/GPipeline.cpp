#include "pch.h"

#include "GPipeline.h"
#include "core/io/Logging.h"
#include "backends/Renderutils.h"
#include "external_utils.h"
#include "core/Coretypes.h"
#include "resource/GenericAssetManager.h"
#include "serialize/PipelineSerializer.h"

namespace trace {



	GPipeline::GPipeline()
	{
	}

	GPipeline::~GPipeline()
	{
	}

	bool GPipeline::Create(PipelineStateDesc desc, bool auto_fill)
	{
		if (!RenderFunc::CreatePipeline(this, desc))
		{
			TRC_ERROR("Failed to create pipeline {}", GetName());
			return false;
		}
		if (!RenderFunc::InitializePipeline(this))
		{
			TRC_ERROR("Failed to initialize pipeline {}", GetName());
			RenderFunc::DestroyPipeline(this);
			return false;
		}

		return true;
	}

	void GPipeline::Destroy()
	{
		RenderFunc::DestroyPipeline(this);
	}

	bool GPipeline::RecreatePipeline(PipelineStateDesc desc)
	{
		RenderFunc::DestroyPipeline(this);

		if (!RenderFunc::CreatePipeline(this, desc))
		{
			TRC_ERROR("Failed to create pipeline {}", GetName());
			GetRenderHandle()->m_internalData = nullptr;
			return false;
		}
		if (!RenderFunc::InitializePipeline(this))
		{
			TRC_ERROR("Failed to initialize pipeline {}", GetName());
			RenderFunc::DestroyPipeline(this);
			GetRenderHandle()->m_internalData = nullptr;
			return false;
		}

		return true;
	}

	Ref<GPipeline> GPipeline::Deserialize(UUID id)
	{
		Ref<GPipeline> result;
		if (AppSettings::is_editor)
		{
			std::string name = GetPathFromUUID(id).string();
			result = PipelineSerializer::Deserialize(name);
		}
		else
		{
			result = GenericAssetManager::get_instance()->Load_Runtime<GPipeline>(id);
		}
		return result;
	}

	Ref<GPipeline> GPipeline::Deserialize(DataStream* stream)
	{
		return PipelineSerializer::Deserialize(stream);
	}


}