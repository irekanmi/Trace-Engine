#pragma once
#include "core/Enums.h"
#include "core/Core.h"
#include "EASTL/vector.h"
#include "glm/glm.hpp"

namespace trace {

	class GShader;

	

	enum class ShaderStage
	{
		NONE,
		VERTEX_SHADER,
		PIXEL_SHADER
	};

	enum class FillMode
	{
		NONE,
		WIREFRAME,
		SOLID,
	};
	enum class CullMode
	{
		NONE,
		FRONT,
		BACK,
	};

	enum class Format
	{
		NONE,
		R32G32B32_FLOAT,
		R32G32B32_UINT,
		R32G32_FLOAT,
		R32G32_UINT,
		R8G8B8A8_SNORM,
		R8G8B8A8_SRBG,
		R8G8B8_SRBG,
		R8G8B8_SNORM
	};

	enum class InputClassification
	{
		PER_VERTEX_DATA,
		PER_INSTANCE_DATA,
	};


	struct Viewport
	{
		float x = 0.0f;
		float y = 0.0f;
		float width = 0.0f;
		float height = 0.0f;
		float minDepth = 0.0f;
		float maxDepth = 1.0f;
	};


	struct InputLayout
	{
		uint32_t stride = 0;
		InputClassification input_class = InputClassification::PER_VERTEX_DATA;
		struct Element
		{
			uint32_t index = 0;
			uint32_t offset = 0;
			uint32_t stride = 0;
			Format format = Format::NONE;
		};
		eastl::vector<Element> elements;
	};

	struct RaterizerState
	{
		CullMode cull_mode = CullMode::NONE;
		FillMode fill_mode = FillMode::NONE;
	};

	struct DepthStencilState
	{
		bool depth_test_enable = false;
		bool stencil_test_enable = false;
		float minDepth = 0.0f;
		float maxDepth = 1.0f;
	};

	struct ColorBlendState
	{
		bool alpha_to_blend_coverage = false; // TODO
	};

	enum class PrimitiveTopology
	{
		NONE,
		TRIANGLE_LIST,
		TRIANGLE_STRIP
	};

	struct PipelineStateDesc
	{
		GShader* vertex_shader = nullptr;
		GShader* pixel_shader = nullptr;
		InputLayout input_layout = {};
		RaterizerState rateriser_state = {};
		DepthStencilState depth_sten_state = {};
		ColorBlendState blend_state = {};
		PrimitiveTopology topology = PrimitiveTopology::NONE;
		Viewport view_port = {};
	};


	struct Vertex
	{
		glm::vec3 pos;
		glm::vec2 texCoord;


		static InputLayout get_input_layout()
		{
			InputLayout layout;
			layout.stride = sizeof(Vertex);
			layout.input_class = InputClassification::PER_VERTEX_DATA;

			InputLayout::Element _pos;
			_pos.format = Format::R32G32B32_FLOAT;
			_pos.index = 0;
			_pos.offset = offsetof(Vertex, pos);
			_pos.stride = sizeof(glm::vec3);

			layout.elements.push_back(_pos);

			InputLayout::Element _texCoord;
			_texCoord.format = Format::R32G32_FLOAT;
			_texCoord.index = 1;
			_texCoord.offset = offsetof(Vertex, texCoord);
			_texCoord.stride = sizeof(glm::vec2);

			layout.elements.push_back(_texCoord);

			return layout;
		}

	};

	struct SceneGlobals
	{
		glm::mat4 view;
		glm::mat4 projection;
		glm::vec2 _test;
	};

}