#pragma once

#include "core/Enums.h"
#include "core/Core.h"
#include "resource/Ref.h"
#include "render/GTexture.h"
#include "render/GPipeline.h"



namespace trace {

	using Texture_Ref = Ref<GTexture>;
	class GPipeline;

	struct Material
	{
		glm::vec4 m_diffuseColor;
		float m_shininess;
		Texture_Ref m_albedoMap;
		Texture_Ref m_specularMap;
		Texture_Ref m_normalMap;
	};

	class TRACE_API MaterialInstance : public Resource
	{

	public:
		MaterialInstance();
		virtual ~MaterialInstance();

		virtual bool Init(Ref<GPipeline> pipeline, Material material) = 0;
		virtual void Apply() = 0;

		Ref<GPipeline> GetRenderPipline() { return m_renderPipeline; }

		GHandle* GetRenderHandle() { return &m_renderHandle; }

	private:
		GHandle m_renderHandle;
	protected:
		Ref<GPipeline> m_renderPipeline;
		Material m_material;
		std::unordered_map<ShaderData, std::pair<void*, uint32_t>> m_shaderData;
	};
}
