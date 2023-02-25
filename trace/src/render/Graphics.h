#pragma once
#include "core/Enums.h"
#include "core/Core.h"
#include "EASTL/vector.h"
#include "EASTL/string.h"
#include "glm/glm.hpp"

namespace trace {

	class GShader;
	class GRenderPass;

	enum class AddressMode
	{
		NONE,
		REPEAT,
		MIRRORED_REPEAT
	};

	enum class FilterMode
	{
		NONE,
		LINEAR
	};


	enum class UsageFlag
	{
		NONE,
		DEFAULT,
		UPLOAD,
		READBACK
	};


	enum BindFlag
	{
		NIL,
		VERTEX_BIT = BIT(1),
		INDEX_BIT = BIT(2),
		CONSTANT_BUFFER_BIT = BIT(3),
		RENDER_TARGET_BIT = BIT(4),
		DEPTH_STENCIL_BIT = BIT(5),
		SHADER_RESOURCE_BIT = BIT(6)
	};

	struct BufferInfo
	{
		uint32_t m_size = 0;
		uint32_t m_stide = 0;
		BindFlag m_flag = BindFlag::NIL;
		UsageFlag m_usageFlag = UsageFlag::NONE;
		void* m_data = nullptr;
	};




	enum RenderAPI
	{
		None,
		OpenGL,
		Vulkan
	};




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
		R8G8B8_SNORM,
		D32_SFLOAT_S8_SUINT
	};

	enum class InputClassification
	{
		PER_VERTEX_DATA,
		PER_INSTANCE_DATA,
	};

	enum class TextureFormat
	{
		UNKNOWN,
		SHADER_READ,
		COLOR_ATTACHMENT,
		DEPTH_STENCIL,
		PRESENT
	};

	enum class AttachmentLoadOp
	{
		OP_NONE,
		LOAD_OP_LOAD,
		LOAD_OP_CLEAR,
		LOAD_OP_DISCARD,
	};
	
	enum class AttachmentStoreOp
	{
		OP_NONE,
		STORE_OP_STORE,
		STORE_OP_DISCARD
	};

	enum class AttachmentType
	{
		NONE,
		COLOR,
		DEPTH,
		SWAPCHAIN
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

	enum class ShaderResourceType
	{
		SHADER_RESOURCE_TYPE_NOUSE,
		SHADER_RESOURCE_TYPE_UNIFORM_BUFFER,
		SHADER_RESOURCE_TYPE_COMBINED_SAMPLER
	};

	enum class ShaderResourceStage
	{
		RESOURCE_STAGE_NONE = -1,
		RESOURCE_STAGE_GLOBAL,
		RESOURCE_STAGE_INSTANCE,
		RESOURCE_STAGE_LOCAL
	};

	struct UniformMetaData
	{
		uint32_t _id = INVAILD_ID;
		uint32_t _offset = INVAILD_ID;
		uint32_t _size = 0;
		uint32_t _slot = 0;
		uint32_t _index = 0;
		uint32_t _count = 0;
		ShaderResourceType _resource_type = ShaderResourceType::SHADER_RESOURCE_TYPE_NOUSE;
	};

	struct Rect2D
	{
		uint32_t left = 0;
		uint32_t right = 0;
		uint32_t top = 0;
		uint32_t bottom = 0;
	};
	
	struct Rect2D_f
	{
		float left = .0f;
		float right = .0f;
		float top = .0f;
		float bottom = .0f;
	};

	struct ShaderResourceBinding
	{
		ShaderStage shader_stage = ShaderStage::NONE;
		eastl::string resource_name = "";
		ShaderResourceType resource_type = ShaderResourceType::SHADER_RESOURCE_TYPE_NOUSE;
		uint32_t resource_size = 0;
		ShaderResourceStage resource_stage = ShaderResourceStage::RESOURCE_STAGE_NONE;
		uint32_t slot = 0;
		uint32_t index = 0;
		uint32_t count = 1;
		
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
		uint32_t resource_bindings_count = 0;
		ShaderResourceBinding* resource_bindings = {};
		GRenderPass* render_pass = nullptr;
		uint32_t subpass_index = uint32_t(-1);
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

	struct TextureDesc
	{
		uint32_t m_width = 0;
		uint32_t m_height = 0;
		Format m_format = Format::NONE;
		BindFlag m_flag = BindFlag::NIL;
		UsageFlag m_usage = UsageFlag::NONE;
		uint32_t m_channels = 0;
		unsigned char* m_data = nullptr;
		AddressMode m_addressModeU = AddressMode::NONE;
		AddressMode m_addressModeV = AddressMode::NONE;
		AddressMode m_addressModeW = AddressMode::NONE;
		FilterMode m_minFilterMode = FilterMode::NONE;
		FilterMode m_magFilterMode = FilterMode::NONE;
	};

	

}