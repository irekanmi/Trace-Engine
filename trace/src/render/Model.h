#pragma once

#include "core/Enums.h"
#include "core/Core.h"
#include "EASTL/vector.h"
#include "Graphics.h"
#include "GBuffer.h"
#include "Material.h"

namespace trace {

	class TRACE_API Model
	{

	public:
		Model();
		Model(const eastl::vector<Vertex>& data, const eastl::vector<uint32_t> indices);
		~Model();

		void Init(const eastl::vector<Vertex>& data, const eastl::vector<uint32_t>& indices);
		uint32_t GetIndexCount() { return m_indices.size(); }
		GBuffer* GetIndexBuffer() { return m_indexBuffer; }
		GBuffer* GetVertexBuffer() { return m_vertexBuffer; }
		Material* GetMaterial() { return &m_material; }

		void SetMaterial(Material material);

	private:
		eastl::vector<Vertex> m_vertices;
		eastl::vector<uint32_t> m_indices;
		Material m_material;
		GBuffer* m_vertexBuffer;
		GBuffer* m_indexBuffer;

	protected:

	};

	void generateDefaultCube(eastl::vector<Vertex>& data, eastl::vector<uint32_t>& indices);
	void generateVertexTangent(eastl::vector<Vertex>& data, eastl::vector<uint32_t>& indices);

}
