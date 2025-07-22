#pragma once

#include "core/Enums.h"
#include "core/Core.h"
#include "Graphics.h"
#include "GBuffer.h"
#include "Material.h"
#include "resource/Resource.h"
#include "core/defines.h"
#include "reflection/TypeRegistry.h"
#include "serialize/DataStream.h"

namespace trace {

	class TRACE_API SkinnedModel : public Resource
	{

	public:
		SkinnedModel();
		SkinnedModel(const std::vector<SkinnedVertex>& data, const std::vector<uint32_t> indices);
		~SkinnedModel();

		bool Create(std::vector<SkinnedVertex>& data, std::vector<uint32_t>& indices);
		virtual void Destroy() override;

		void Init(const std::vector<SkinnedVertex>& data, const std::vector<uint32_t>& indices);
		uint32_t GetIndexCount() { return static_cast<uint32_t>(m_indices.size()); }
		GBuffer* GetIndexBuffer() { return &m_indexBuffer; }
		GBuffer* GetVertexBuffer() { return &m_vertexBuffer; }
		std::vector<SkinnedVertex>& GetVertices() { return m_vertices; }
		std::vector<uint32_t>& GetIndices() { return m_indices; }
		void Release();

		static Ref<SkinnedModel> Deserialize(UUID id);
		static Ref<SkinnedModel> Deserialize(DataStream* stream);
	private:
		std::vector<SkinnedVertex> m_vertices;
		std::vector<uint32_t> m_indices;
		GBuffer m_vertexBuffer;
		GBuffer m_indexBuffer;

	protected:
		ACCESS_CLASS_MEMBERS(SkinnedModel);

	};

	void generateVertexTangent(std::vector<SkinnedVertex>& data, std::vector<uint32_t>& indices);

}
