#pragma once

#include "core/Enums.h"
#include "core/Core.h"
#include "EASTL/vector.h"
#include "Graphics.h"
#include "GBuffer.h"
#include "Material.h"
#include "resource/Resource.h"

namespace trace {

	class TRACE_API Model : public Resource
	{

	public:
		Model();
		Model(const std::vector<Vertex>& data, const std::vector<uint32_t> indices);
		~Model();

		void Init(const std::vector<Vertex>& data, const std::vector<uint32_t>& indices);
		uint32_t GetIndexCount() { return m_indices.size(); }
		GBuffer* GetIndexBuffer() { return m_indexBuffer; }
		GBuffer* GetVertexBuffer() { return m_vertexBuffer; }


		Ref<MaterialInstance> m_matInstance;
	private:
		std::vector<Vertex> m_vertices;
		std::vector<uint32_t> m_indices;
		GBuffer* m_vertexBuffer;
		GBuffer* m_indexBuffer;

	protected:

	};

	void generateDefaultCube(std::vector<Vertex>& data, std::vector<uint32_t>& indices);
	void generateVertexTangent(std::vector<Vertex>& data, std::vector<uint32_t>& indices);

}
