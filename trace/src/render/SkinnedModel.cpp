#include "pch.h"
#include "SkinnedModel.h"
#include "backends/Renderutils.h"
#include "core/Coretypes.h"
#include "external_utils.h"

#include "glm/ext.hpp"

namespace trace {



	SkinnedModel::SkinnedModel()
	{
	}

	SkinnedModel::SkinnedModel(const std::vector<SkinnedVertex>& data, const std::vector<uint32_t> indices)
	{
		Init(data, indices);
	}

	SkinnedModel::~SkinnedModel()
	{


	}

	void SkinnedModel::Init(const std::vector<SkinnedVertex>& data, const std::vector<uint32_t>& indices)
	{
		m_vertices = data;
		m_indices = indices;
		BufferInfo vertex_buffer_info;
		vertex_buffer_info.m_data = m_vertices.data();
		vertex_buffer_info.m_flag = BindFlag::VERTEX_BIT;
		vertex_buffer_info.m_size = static_cast<uint32_t>(m_vertices.size() * sizeof(SkinnedVertex));
		vertex_buffer_info.m_stide = sizeof(SkinnedVertex);
		vertex_buffer_info.m_usageFlag = UsageFlag::DEFAULT;

		RenderFunc::CreateBuffer(&m_vertexBuffer, vertex_buffer_info);

		BufferInfo index_buffer_info;
		index_buffer_info.m_data = m_indices.data();
		index_buffer_info.m_flag = BindFlag::INDEX_BIT;
		index_buffer_info.m_size = static_cast<uint32_t>(m_indices.size() * sizeof(uint32_t));
		index_buffer_info.m_stide = sizeof(uint32_t);
		index_buffer_info.m_usageFlag = UsageFlag::DEFAULT;

		RenderFunc::CreateBuffer(&m_indexBuffer, index_buffer_info);
	}

	void SkinnedModel::Release()
	{
		m_vertices.clear();
		m_indices.clear();
		RenderFunc::DestroyBuffer(&m_vertexBuffer);
		RenderFunc::DestroyBuffer(&m_indexBuffer);
	}

	void SkinnedModel::Destroy()
	{
		Release();
	}

	Ref<SkinnedModel> SkinnedModel::Deserialize(UUID id)
	{
		Ref<SkinnedModel> result;
		if (AppSettings::is_editor)
		{
			std::string model_name = GetNameFromUUID(id);
			result = LoadSkinnedModel(id, model_name);
		}
		else
		{
		}
		return result;
	}

	
	void generateVertexTangent(std::vector<SkinnedVertex>& data, std::vector<uint32_t>& indices)
	{


		for (uint32_t i = 0; i < indices.size(); i += 3)
		{
			glm::vec3 p0 = data[indices[i + 0]].pos;
			glm::vec3 p1 = data[indices[i + 1]].pos;
			glm::vec3 p2 = data[indices[i + 2]].pos;

			glm::vec2 t0 = data[indices[i + 0]].texCoord;
			glm::vec2 t1 = data[indices[i + 1]].texCoord;
			glm::vec2 t2 = data[indices[i + 2]].texCoord;

			glm::vec3 egde1 = p1 - p0;
			glm::vec3 egde2 = p2 - p0;

			glm::vec2 deltaUV1 = t1 - t0;
			glm::vec2 deltaUV2 = t2 - t0;

			float f = 1 / ((deltaUV1.x * deltaUV2.y) - (deltaUV2.x * deltaUV1.y));

			glm::vec3 tangent;
			tangent.x = f * ((deltaUV2.y * egde1.x) + (-deltaUV1.y * egde2.x));
			tangent.y = f * ((deltaUV2.y * egde1.y) + (-deltaUV1.y * egde2.y));
			tangent.z = f * ((deltaUV2.y * egde1.z) + (-deltaUV1.y * egde2.z));

			glm::vec3 bitangent;
			bitangent.y = f * ((-deltaUV2.x * egde1.y) + (deltaUV1.x * egde2.y));
			bitangent.x = f * ((-deltaUV2.x * egde1.x) + (deltaUV1.x * egde2.x));
			bitangent.z = f * ((-deltaUV2.x * egde1.z) + (deltaUV1.x * egde2.z));

			float sx = deltaUV1.x, sy = deltaUV2.x;
			float tx = deltaUV1.y, ty = deltaUV2.y;
			float hand = (tx * sy - ty * sx) < 0.0f ? -1.0f : 1.0f;
			float handeness = glm::dot(glm::cross(tangent, bitangent), data[indices[i + 0]].normal) > 0.0f ? 1.0f : -1.0f;

			//HACK: fix
			glm::vec4 _tangent = glm::vec4(glm::normalize(tangent), handeness);

			data[indices[i + 0]].tangent = _tangent;
			data[indices[i + 1]].tangent = _tangent;
			data[indices[i + 2]].tangent = _tangent;

		}

	}
}
