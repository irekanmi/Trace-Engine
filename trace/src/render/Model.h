#pragma once

#include "core/Enums.h"
#include "core/Core.h"
#include "Graphics.h"
#include "GBuffer.h"
#include "Material.h"
#include "resource/Resource.h"
#include "resource/Ref.h"
#include "scene/UUID.h"
#include "reflection/TypeRegistry.h"
#include "serialize/DataStream.h"

namespace trace {

	class Model : public Resource
	{

	public:
		Model();
		Model(const std::vector<Vertex>& data, const std::vector<uint32_t> indices);
		virtual ~Model();

		bool Create(const std::vector<Vertex>& data, const std::vector<uint32_t>& indices);
		virtual void Destroy();

		void Init(const std::vector<Vertex>& data, const std::vector<uint32_t>& indices);
		uint32_t GetIndexCount() { return static_cast<uint32_t>(m_indices.size()); }
		GBuffer* GetIndexBuffer() { return &m_indexBuffer; }
		GBuffer* GetVertexBuffer() { return &m_vertexBuffer; }
		std::vector<Vertex>& GetVertices() { return m_vertices; }
		std::vector<uint32_t>& GetIndices() { return m_indices; }
		void Release();

		static Ref<Model> Deserialize(UUID id);
		static Ref<Model> Deserialize(DataStream* stream);

	private:
		std::vector<Vertex> m_vertices;
		std::vector<uint32_t> m_indices;
		GBuffer m_vertexBuffer;
		GBuffer m_indexBuffer;

	protected:
		ACCESS_CLASS_MEMBERS(Model);

	};

	void generateDefaultCube(std::vector<Vertex>& data, std::vector<uint32_t>& indices);
	void generateDefaultPlane(std::vector<Vertex>& data, std::vector<uint32_t>& indices);
	void generateSphere(std::vector<Vertex>& data, std::vector<uint32_t>& indices, float radius, uint32_t num_vertical, uint32_t num_horizontal);
	void generateVertexTangent(std::vector<Vertex>& data, std::vector<uint32_t>& indices);

}
