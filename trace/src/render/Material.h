#pragma once

#include "core/Enums.h"
#include "core/Core.h"
#include "resource/Ref.h"
#include "render/GTexture.h"
#include "render/GPipeline.h"

#include <any>
#include <variant>


namespace trace {

	using Texture_Ref = Ref<GTexture>;
	using MaterialData = std::unordered_map<std::string, std::pair<std::any, uint32_t>>;
	class GPipeline;

	class TRACE_API MaterialInstance : public Resource
	{

	public:
		MaterialInstance();
		~MaterialInstance();

		bool Init(Ref<GPipeline> pipeline) { return false; };
		void Apply() {};

		Ref<GPipeline> GetRenderPipline() {	return m_renderPipeline; }
		MaterialData& GetMaterialData() { return m_data; }

		GHandle* GetRenderHandle() { return &m_renderHandle; }

		void SetRenderPipeline(Ref<GPipeline> pipeline) { m_renderPipeline = pipeline; }
		void SetMaterialData(MaterialData& material_data) { m_data = std::move(material_data); }

	private:
		Ref<GPipeline> m_renderPipeline;
		MaterialData m_data;
		GHandle m_renderHandle;
	protected:
	};
}
