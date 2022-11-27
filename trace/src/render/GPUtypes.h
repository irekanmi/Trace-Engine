#pragma once



#include "core/Core.h"
#include "core/Enums.h"
#include "glm/glm.hpp"

namespace trace {
	

	enum BufferUsage
	{

		Null = 0x00,
		VERTEX_BUFFER = BIT(0),
		INDEX_BUFFER = BIT(1),
		STAGING_BUFFER = BIT(2)

	};

	struct BufferInfo
	{
		uint32_t m_size = 0;
		uint32_t m_stide = 0;
		BufferUsage m_usage = BufferUsage::Null;
		void* m_data = nullptr;
	};

	struct Vertex
	{
		glm::vec3 pos;
		glm::vec3 color;
	};

	enum RenderAPI
	{
		None,
		OpenGL,
		Vulkan
	};


}
