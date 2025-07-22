#pragma once

#include "core/Enums.h"
#include "core/Core.h"
#include "resource/Ref.h"
#include "render/GTexture.h"
#include "render/GPipeline.h"
#include "scene/UUID.h"
#include "serialize/DataStream.h"

#include <any>
#include <variant>


namespace trace {

	using Texture_Ref = Ref<GTexture>;
	using MaterialData = std::unordered_map<std::string, std::pair<std::any, uint32_t>>;
	class GPipeline;

	class MaterialInstance : public Resource
	{

	public:
		MaterialInstance();
		virtual ~MaterialInstance();

		bool Create(Ref<GPipeline> pipeline);
		virtual void Destroy() override;
		bool RecreateMaterial(Ref<GPipeline> pipeline);

		Ref<GPipeline> GetRenderPipline() {	return m_renderPipeline; }
		MaterialData& GetMaterialData() { return m_data; }

		GHandle* GetRenderHandle() { return &m_renderHandle; }

		void SetRenderPipeline(Ref<GPipeline> pipeline) { m_renderPipeline = pipeline; }
		void SetMaterialData(MaterialData& material_data) { m_data = std::move(material_data); }


		static Ref<MaterialInstance> Deserialize(UUID id);
		static Ref<MaterialInstance> Deserialize(DataStream* stream);

	private:
		Ref<GPipeline> m_renderPipeline;
		MaterialData m_data;
		GHandle m_renderHandle;
	protected:
	};
}
