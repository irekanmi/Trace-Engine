#pragma once

#include "core/Enums.h"
#include "core/Core.h"
#include "resource/Ref.h"
#include "render/GTexture.h"
#include "render/GPipeline.h"
#include "scene/UUID.h"
#include "serialize/DataStream.h"

#include <variant>


namespace trace {

	using Texture_Ref = Ref<GTexture>;
	

	using MaterialData = std::unordered_map<std::string, InternalMaterialData>;
	class GPipeline;

	enum class MaterialType
	{
		NONE,
		OPAQUE_LIT,
		OPAQUE_UNLIT,
		TRANSPARENT_LIT,
		TRANSPARENT_UNLIT,
		PARTICLE_LIT,
		PARTICLE_UNLIT
	};

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

		MaterialType GetType() { return m_type; }
		void SetType(MaterialType type) { m_type = type; }


		static Ref<MaterialInstance> Deserialize(UUID id);
		static Ref<MaterialInstance> Deserialize(DataStream* stream);

	private:
		Ref<GPipeline> m_renderPipeline;
		MaterialData m_data;
		GHandle m_renderHandle;
		MaterialType m_type = MaterialType::OPAQUE_LIT;
	protected:
	};
}
