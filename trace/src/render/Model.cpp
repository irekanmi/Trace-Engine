#include "pch.h"
#include "Model.h"
#include "glm/ext.hpp"
#include "backends/Renderutils.h"

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

	void Model::Release()
	{
		//m_matInstance.free();
		m_vertices.clear();
		m_indices.clear();
		RenderFunc::DestroyBuffer(&m_vertexBuffer);
		RenderFunc::DestroyBuffer(&m_indexBuffer);
	}

	void generateDefaultCube(std::vector<Vertex>& data, std::vector<uint32_t>& indices)
	{


		float half_width = /*width*/  1.0f * 0.5f;
		float half_height = /*height*/1.0f * 0.5f;
		float half_depth = /*depth*/  1.0f * 0.5f;
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

		data = std::move(verts);
		indices = std::move(_ind);

	}
	void generateDefaultPlane(std::vector<Vertex>& data, std::vector<uint32_t>& indices)
	{
		Vertex p0;
		Vertex p1;
		Vertex p2;
		Vertex p3;

		p0.pos = glm::vec3(-1.0f, 0.0f, -1.0f);
		p0.normal = glm::vec3(0.0f, 1.0f, 0.0f);
		p0.texCoord = glm::vec2(0.0f, 1.0f);
		data.push_back(p0);

		p1.pos = glm::vec3(-1.0f, 0.0f, 1.0f);
		p1.normal = glm::vec3(0.0f, 1.0f, 0.0f);
		p1.texCoord = glm::vec2(0.0f, 0.0f);
		data.push_back(p1);

		p2.pos = glm::vec3(1.0f, 0.0f, 1.0f);
		p2.normal = glm::vec3(0.0f, 1.0f, 0.0f);
		p2.texCoord = glm::vec2(1.0f, 0.0f);
		data.push_back(p2);

		p3.pos = glm::vec3(1.0f, 0.0f, -1.0f);
		p3.normal = glm::vec3(0.0f, 1.0f, 0.0f);
		p3.texCoord = glm::vec2(1.0f, 1.0f);
		data.push_back(p3);

		indices.push_back(0);
		indices.push_back(1);
		indices.push_back(2);
		indices.push_back(2);
		indices.push_back(3);
		indices.push_back(0);


	}
	void generateSphere(std::vector<Vertex>& data, std::vector<uint32_t>& indices, float radius, uint32_t num_vertical, uint32_t num_horizontal)
	{
		std::vector<std::vector<glm::vec3>> positions;
		positions.resize(num_horizontal + 1);
		std::vector<std::vector<glm::vec2>> tex_coords;
		tex_coords.resize(num_horizontal + 1);
		std::vector<std::vector<glm::vec3>> normals;
		normals.resize(num_horizontal + 1);

		std::vector<Vertex> vertices;
		vertices.resize((num_horizontal + 1) * (num_vertical + 1));
		positions.resize(num_horizontal + 1);
		for (uint32_t lon = 0; lon <= num_horizontal; lon++)
		{
			float theta = glm::pi<float>() * (float)(lon) / (float)(num_horizontal);
			float v = (float)(lon) / (float)(num_horizontal);
			positions[lon].resize(num_vertical + 1);
			tex_coords[lon].resize(num_vertical + 1);
			normals[lon].resize(num_vertical + 1);
			for (uint32_t lat = 0; lat <= num_vertical; lat++)
			{
				float phi = glm::two_pi<float>() * (float)(lat) / (float)(num_vertical);
				float u = (float)(lat) / (float)(num_vertical);
				float x = radius * glm::sin(theta) * glm::cos(phi);
				float y = radius * glm::sin(theta) * glm::sin(phi);
				float z = radius * glm::cos(theta);

				glm::vec3 location = glm::vec3(x, y, z);
				glm::vec3 normal = glm::normalize(location - glm::vec3(0.0f));
				glm::vec2 tex_coord = glm::vec2(u, v);
				
				positions[lon][lat] = location;
				tex_coords[lon][lat] = tex_coord;
				normals[lon][lat] = normal;

				Vertex vert;
				vert.pos = location;
				vert.texCoord = tex_coord;
				vert.normal = normal;

				vertices[(lon * (num_horizontal)) + lat] = vert;
			}
		}

		for (uint32_t lon = 0; lon < num_horizontal; lon++)
		{
			for (uint32_t lat = 0; lat < num_vertical; lat++)
			{
				
				uint32_t index0 = (lon * num_horizontal) + lat;
				uint32_t index1 = ((lon + 1) * num_horizontal) + lat;
				uint32_t index2 = (lon * num_horizontal) + (lat + 1);

				indices.push_back(index0);
				indices.push_back(index1);
				indices.push_back(index2);
			}
		}

		for (uint32_t lon = 0; lon < num_horizontal; lon++)
		{
			for (uint32_t lat = 1; lat < num_vertical + 1; lat++)
			{
				

				uint32_t index0 = (lon * num_horizontal) + lat;
				uint32_t index1 = ((lon + 1) * num_horizontal) + (lat - 1);
				uint32_t index2 = ((lon + 1) * num_horizontal) + (lat);

				indices.push_back(index0);
				indices.push_back(index1);
				indices.push_back(index2);
			}
		}

		data = std::move(vertices);


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
			glm::vec4 _tangent = glm::vec4(glm::normalize(tangent), handeness);

			data[indices[i + 0]].tangent = _tangent;
			data[indices[i + 1]].tangent = _tangent;
			data[indices[i + 2]].tangent = _tangent;

		}

	}
}
