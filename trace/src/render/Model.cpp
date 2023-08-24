#include "pch.h"
#include "Model.h"
#include "glm/ext.hpp"
#include "Renderutils.h"

namespace trace {



	Model::Model()
	{
	}

	Model::Model(const std::vector<Vertex>& data, const std::vector<uint32_t> indices)
	{
		Init(data, indices);
	}

	Model::~Model()
	{
		RenderFunc::DestroyBuffer(&m_vertexBuffer);
		RenderFunc::DestroyBuffer(&m_indexBuffer);

	}

	void Model::Init(const std::vector<Vertex>& data, const std::vector<uint32_t>& indices)
	{
		m_vertices = data;
		m_indices = indices;
		BufferInfo vertex_buffer_info;
		vertex_buffer_info.m_data = m_vertices.data();
		vertex_buffer_info.m_flag = BindFlag::VERTEX_BIT;
		vertex_buffer_info.m_size = static_cast<uint32_t>(m_vertices.size() * sizeof(Vertex));
		vertex_buffer_info.m_stide = sizeof(Vertex);
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

	void generateDefaultCube(std::vector<Vertex>& data, std::vector<uint32_t>& indices)
	{


		float half_width = /*width*/  1.5f * 0.5f;
		float half_height = /*height*/1.5f * 0.5f;
		float half_depth = /*depth*/  1.5f * 0.5f;
		float min_x = -half_width;
		float min_y = -half_height;
		float min_z = -half_depth;
		float max_x = half_width;
		float max_y = half_height;
		float max_z = half_depth;
		float min_uvx = 0.0f;
		float min_uvy = 0.0f;
		float max_uvx = 1.0f; //tile_x;
		float max_uvy =  1.0f;//tile_y;
		
		
		
		std::vector<Vertex> verts(24);
		
		// Front face
		verts[(0 * 4) + 0].pos = { min_x, min_y, max_z };
		verts[(0 * 4) + 1].pos = { max_x, max_y, max_z };
		verts[(0 * 4) + 2].pos = { min_x, max_y, max_z };
		verts[(0 * 4) + 3].pos = { max_x, min_y, max_z };
		verts[(0 * 4) + 0].texCoord = { min_uvx, min_uvy };
		verts[(0 * 4) + 1].texCoord = { max_uvx, max_uvy };
		verts[(0 * 4) + 2].texCoord = { min_uvx, max_uvy };
		verts[(0 * 4) + 3].texCoord = { max_uvx, min_uvy };
		verts[(0 * 4) + 0].normal = { 0.0f, 0.0f, 1.0f };
		verts[(0 * 4) + 1].normal = { 0.0f, 0.0f, 1.0f };
		verts[(0 * 4) + 2].normal = { 0.0f, 0.0f, 1.0f };
		verts[(0 * 4) + 3].normal = { 0.0f, 0.0f, 1.0f };
		
		// Back face
		verts[(1 * 4) + 0].pos = { max_x, min_y, min_z };
		verts[(1 * 4) + 1].pos = { min_x, max_y, min_z };
		verts[(1 * 4) + 2].pos = { max_x, max_y, min_z };
		verts[(1 * 4) + 3].pos = { min_x, min_y, min_z };
		verts[(1 * 4) + 0].texCoord = { min_uvx, min_uvy };
		verts[(1 * 4) + 1].texCoord = { max_uvx, max_uvy };
		verts[(1 * 4) + 2].texCoord = { min_uvx, max_uvy };
		verts[(1 * 4) + 3].texCoord = { max_uvx, min_uvy };
		verts[(1 * 4) + 0].normal = { 0.0f, 0.0f, -1.0f };
		verts[(1 * 4) + 1].normal = { 0.0f, 0.0f, -1.0f };
		verts[(1 * 4) + 2].normal = { 0.0f, 0.0f, -1.0f };
		verts[(1 * 4) + 3].normal = { 0.0f, 0.0f, -1.0f };
		
		// Left
		verts[(2 * 4) + 0].pos = { min_x, min_y, min_z };
		verts[(2 * 4) + 1].pos = { min_x, max_y, max_z };
		verts[(2 * 4) + 2].pos = { min_x, max_y, min_z };
		verts[(2 * 4) + 3].pos = { min_x, min_y, max_z };
		verts[(2 * 4) + 0].texCoord = { min_uvx, min_uvy };
		verts[(2 * 4) + 1].texCoord = { max_uvx, max_uvy };
		verts[(2 * 4) + 2].texCoord = { min_uvx, max_uvy };
		verts[(2 * 4) + 3].texCoord = { max_uvx, min_uvy };
		verts[(2 * 4) + 0].normal = { -1.0f, 0.0f, 0.0f };
		verts[(2 * 4) + 1].normal = { -1.0f, 0.0f, 0.0f };
		verts[(2 * 4) + 2].normal = { -1.0f, 0.0f, 0.0f };
		verts[(2 * 4) + 3].normal = { -1.0f, 0.0f, 0.0f };
		
		// Right face
		verts[(3 * 4) + 0].pos = { max_x, min_y, max_z };
		verts[(3 * 4) + 1].pos = { max_x, max_y, min_z };
		verts[(3 * 4) + 2].pos = { max_x, max_y, max_z };
		verts[(3 * 4) + 3].pos = { max_x, min_y, min_z };
		verts[(3 * 4) + 0].texCoord = { min_uvx, min_uvy };
		verts[(3 * 4) + 1].texCoord = { max_uvx, max_uvy };
		verts[(3 * 4) + 2].texCoord = { min_uvx, max_uvy };
		verts[(3 * 4) + 3].texCoord = { max_uvx, min_uvy };
		verts[(3 * 4) + 0].normal = { 1.0f, 0.0f, 0.0f };
		verts[(3 * 4) + 1].normal = { 1.0f, 0.0f, 0.0f };
		verts[(3 * 4) + 2].normal = { 1.0f, 0.0f, 0.0f };
		verts[(3 * 4) + 3].normal = { 1.0f, 0.0f, 0.0f };
		
		// Bottom face
		verts[(4 * 4) + 0].pos = { max_x, min_y, max_z };
		verts[(4 * 4) + 1].pos = { min_x, min_y, min_z };
		verts[(4 * 4) + 2].pos = { max_x, min_y, min_z };
		verts[(4 * 4) + 3].pos = { min_x, min_y, max_z };
		verts[(4 * 4) + 0].texCoord = { min_uvx, min_uvy };
		verts[(4 * 4) + 1].texCoord = { max_uvx, max_uvy };
		verts[(4 * 4) + 2].texCoord = { min_uvx, max_uvy };
		verts[(4 * 4) + 3].texCoord = { max_uvx, min_uvy };
		verts[(4 * 4) + 0].normal = { 0.0f, -1.0f, 0.0f };
		verts[(4 * 4) + 1].normal = { 0.0f, -1.0f, 0.0f };
		verts[(4 * 4) + 2].normal = { 0.0f, -1.0f, 0.0f };
		verts[(4 * 4) + 3].normal = { 0.0f, -1.0f, 0.0f };
		
		// Top face
		verts[(5 * 4) + 0].pos = { min_x, max_y, max_z };
		verts[(5 * 4) + 1].pos = { max_x, max_y, min_z };
		verts[(5 * 4) + 2].pos = { min_x, max_y, min_z };
		verts[(5 * 4) + 3].pos = { max_x, max_y, max_z };
		verts[(5 * 4) + 0].texCoord = { min_uvx, min_uvy };
		verts[(5 * 4) + 1].texCoord = { max_uvx, max_uvy };
		verts[(5 * 4) + 2].texCoord = { min_uvx, max_uvy };
		verts[(5 * 4) + 3].texCoord = { max_uvx, min_uvy };
		verts[(5 * 4) + 0].normal = { 0.0f, 1.0f, 0.0f };
		verts[(5 * 4) + 1].normal = { 0.0f, 1.0f, 0.0f };
		verts[(5 * 4) + 2].normal = { 0.0f, 1.0f, 0.0f };
		verts[(5 * 4) + 3].normal = { 0.0f, 1.0f, 0.0f };

		std::vector<uint32_t> _ind(36);

		for (uint32_t i = 0; i < 6; ++i) {
			uint32_t v_offset = i * 4;
			uint32_t i_offset = i * 6;
			_ind[i_offset + 0] = v_offset + 0;
			_ind[i_offset + 1] = v_offset + 1;
			_ind[i_offset + 2] = v_offset + 2;
			_ind[i_offset + 3] = v_offset + 0;
			_ind[i_offset + 4] = v_offset + 3;
			_ind[i_offset + 5] = v_offset + 1;
		}

		data = verts;
		indices = _ind;

	}
	void generateSphere(std::vector<Vertex>& data, std::vector<uint32_t>& indices, float radius, uint32_t num_vertical, uint32_t num_horizontal)
	{
		std::vector<glm::vec3> positions;
		positions.reserve(num_vertical * num_horizontal);
		std::vector<glm::vec2> tex_coords;
		tex_coords.reserve(num_vertical * num_horizontal);
		for (uint32_t lon = 1; lon < num_horizontal + 2; lon++)
		{
			float theta = glm::pi<float>() * (float)(lon) / (float)(num_horizontal + 2);
			float u = (float)(lon) / (float)(num_horizontal + 2);
			for (uint32_t lat = 1; lat < num_vertical + 2; lat++)
			{
				float phi = glm::two_pi<float>() * (float)(lat) / (float)(num_vertical + 2);
				float v = (float)(lat) / (float)(num_vertical + 2);
				float x = radius * glm::sin(theta) * glm::cos(phi);
				float y = radius * glm::sin(theta) * glm::sin(phi);
				float z = radius * glm::cos(theta);

				positions.push_back({ x, y, z });
				tex_coords.push_back({ u, v });


			}
		}

		for (uint32_t lon = 1; lon < num_horizontal; lon++)
		{
			for (uint32_t lat = 0; lat < num_vertical; lat++)
			{
				uint32_t _lat = ((lon - 1) * num_horizontal) + lat;
				Vertex vert0 = {};
				vert0.pos = positions[_lat];
				vert0.texCoord = tex_coords[_lat];

				Vertex vert1 = {};
				vert1.pos = positions[(lon * num_horizontal) + _lat];
				vert1.texCoord = tex_coords[(lon * num_horizontal) + _lat];

				Vertex vert2 = {};
				vert2.pos = positions[_lat + 1];
				vert2.texCoord = tex_coords[_lat + 1];

				glm::vec3 edge1 = vert1.pos - vert0.pos;
				glm::vec3 edge2 = vert2.pos - vert0.pos;

				glm::vec3 normal = glm::cross(edge1, edge2);
				normal = glm::normalize(normal);
				vert0.normal = normal;
				vert1.normal = normal;
				vert2.normal = normal;

				data.push_back(vert0);
				data.push_back(vert1);
				data.push_back(vert2);
			}
		}

		for (uint32_t lon = 1; lon < num_horizontal; lon++)
		{
			for (uint32_t lat = 1; lat < num_vertical; lat++)
			{
				uint32_t _lat = ((lon - 1) * num_horizontal) + lat;
				Vertex vert0 = {};
				vert0.pos = positions[_lat];
				vert0.texCoord = tex_coords[_lat];

				Vertex vert2 = {};
				vert2.pos = positions[((lon * num_horizontal) + _lat) - 1];
				vert2.texCoord = tex_coords[((lon * num_horizontal) + _lat) - 1];

				Vertex vert1 = {};
				vert1.pos = positions[((lon * num_horizontal) + _lat) + 1];
				vert1.texCoord = tex_coords[((lon * num_horizontal) + _lat) + 1];

				glm::vec3 edge1 = vert1.pos - vert0.pos;
				glm::vec3 edge2 = vert2.pos - vert0.pos;

				glm::vec3 normal = glm::cross(edge1, edge2);
				normal = glm::normalize(normal);
				vert0.normal = normal;
				vert1.normal = normal;
				vert2.normal = normal;

				data.push_back(vert0);
				data.push_back(vert1);
				data.push_back(vert2);
			}
		}

		indices.reserve(data.size());
		for (uint32_t i = 0; i < static_cast<uint32_t>(data.size()); i++) indices.push_back(i);

	}
	void generateVertexTangent(std::vector<Vertex>& data, std::vector<uint32_t>& indices)
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
			float handeness = glm::dot( glm::cross(tangent, bitangent), data[indices[i + 0]].normal) > 0.0f ? 1.0f : -1.0f;
			
			//HACK: fix
			glm::vec4 _tangent = glm::vec4(glm::normalize(tangent), hand);

			data[indices[i + 0]].tangent = _tangent;
			data[indices[i + 1]].tangent = _tangent;
			data[indices[i + 2]].tangent = _tangent;

		}

	}
}
