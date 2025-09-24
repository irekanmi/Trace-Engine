#pragma once
#include "core/Enums.h"
#include "core/Core.h"
#include "core/defines.h"


#include <any>
#include <vector>
#include <array>
#include <string>
#include "glm/glm.hpp"

#define MAX_LIGHT_COUNT 15

namespace trace {

	class GShader;
	class GTexture;
	class GRenderPass;

	enum class AddressMode
	{
		NONE,
		REPEAT,
		MIRRORED_REPEAT,
		CLAMP_TO_EDGE,
		CLAMP_TO_BORDER
	};

	enum class FilterMode
	{
		NONE,
		LINEAR,
		NEAREST
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
		SHADER_RESOURCE_BIT = BIT(6),
		UNORDERED_RESOURCE_BIT = BIT(7)
	};


	enum class RenderAPI
	{
		None,
		OpenGL,
		Vulkan
	};




	enum ShaderStage
	{
		STAGE_NONE = BIT(1),
		VERTEX_SHADER = BIT(2),
		PIXEL_SHADER = BIT(3)
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
		R16_FLOAT,
		R32G32B32A32_FLOAT,
		R32G32B32_FLOAT,
		R32G32B32A32_UINT,
		R32G32B32_UINT,
		R32G32_FLOAT,
		R32_FLOAT,
		R32G32_UINT,
		R32_UINT,
		R32_SINT,
		R16G16B16_FLOAT,
		R16G16B16A16_FLOAT,
		R8G8B8A8_SNORM,
		R8G8B8A8_SRBG,
		R8G8B8A8_RBG,
		R8G8B8_SRBG,
		R8G8B8_SNORM,
		R8G8B8A8_UNORM,
		R8G8B8_UNORM,
		R8_UNORM,
		D32_SFLOAT_S8_SUINT,
		D32_SFLOAT,
		R32G32B32A32_SINT
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
		DEPTH,
		DEPTH_STENCIL,
		PRESENT
	};

	enum class ImageType
	{
		NO_TYPE,
		IMAGE_1D,
		IMAGE_2D,
		IMAGE_3D,
		CUBE_MAP
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

	

	enum class PRIMITIVETOPOLOGY
	{
		NONE,
		TRIANGLE_LIST,
		LINE_LIST,
		TRIANGLE_STRIP,
		LINE_STRIP
	};

	enum class ShaderData
	{
		NONE,
		MATERIAL_ALBEDO,
		MATERIAL_SPECULAR,
		MATERIAL_NORMAL,
		MATERIAL_DIFFUSE_COLOR,
		MATERIAL_SHININESS,
		CUSTOM_DATA_UNKNOWN,
		CUSTOM_DATA_TEXTURE,
		CUSTOM_DATA_INT,
		CUSTOM_DATA_FLOAT,
		CUSTOM_DATA_BOOL,
		CUSTOM_DATA_VEC2,
		CUSTOM_DATA_IVEC2,
		CUSTOM_DATA_VEC3,
		CUSTOM_DATA_IVEC3,
		CUSTOM_DATA_VEC4,
		CUSTOM_DATA_IVEC4,
		CUSTOM_DATA_MAT2,
		CUSTOM_DATA_MAT3,
		CUSTOM_DATA_MAT4,
	};

	enum class ShaderResourceType
	{
		SHADER_RESOURCE_TYPE_NOUSE,
		SHADER_RESOURCE_TYPE_UNIFORM_BUFFER,
		SHADER_RESOURCE_TYPE_STORAGE_BUFFER,
		SHADER_RESOURCE_TYPE_COMBINED_SAMPLER
	};

	enum class ShaderResourceStage
	{
		RESOURCE_STAGE_NONE = 0,
		RESOURCE_STAGE_GLOBAL,
		RESOURCE_STAGE_INSTANCE,
		RESOURCE_STAGE_LOCAL
	};

	enum RENDERPASS
	{
		PRE_PASS,
		MAIN_PASS,
		POST_PASS,
		UI_PASS,
		RENDER_PASS_COUNT
	};

	enum GPU_QUEUE
	{
		GRAPHICS = BIT(1),
		COMPUTE = BIT(2)
	};

	enum ShaderDataDef
	{
		NO_DEFINITION,
		STRUCTURE,
		IMAGE
	};

	enum BlendFactor
	{
		BLEND_NONE,
		BLEND_ONE,
		BLEND_ZERO,
		BLEND_ONE_MINUS_SRC_ALPHA,
		BLEND_ONE_MINUS_DST_ALPHA,
		BLEND_ONE_MINUS_SRC_COLOR,
		BLEND_ONE_MINUS_DST_COLOR,
		BLEND_SRC_COLOR,
		BLEND_DST_COLOR,
		BLEND_SRC_ALPHA,
		BLEND_DST_ALPHA,
		BLEND_FACTOR_COUNT
	};

	enum BlendOp
	{
		BLEND_OP_NONE,
		BLEND_OP_ADD,
		BLEND_OP_SUBTRACT,
		BLEND_OP_REVERSE_SUBTRACT,
		BLEND_OP_MIN,
		BLEND_OP_MAX,
		BLEND_OP_COUNT
	};

	enum FrameSettingsBit
	{
		RENDER_NONE = BIT(0),
		RENDER_DEFAULT = BIT(1),
		RENDER_SSAO = BIT(2),
		RENDER_HDR = BIT(3),
		RENDER_BLOOM = BIT(4),
		RENDER_SETTING_MAX
	};

	enum class LightType
	{
		UNKNOWN = -1,
		DIRECTIONAL,
		POINT,
		SPOT
	};

	enum PipelineType
	{
		Unknown = BIT(1),
		Surface_Material = BIT(2),
		Post_Process = BIT(3)
	};

	typedef uint32_t FrameSettings;

	struct BufferInfo
	{
		uint32_t m_size = 0;
		uint32_t m_stide = 0;
		BindFlag m_flag = BindFlag::NIL;
		UsageFlag m_usageFlag = UsageFlag::NONE;
		void* m_data = nullptr;
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

	struct Viewport
	{
		float x = 0.0f;
		float y = 0.0f;
		float width = 0.0f;
		float height = 0.0f;
		float minDepth = 0.0f;
		float maxDepth = 1.0f;
	};


	struct Element
	{
		uint32_t index = 0;
		uint32_t offset = 0;
		uint32_t stride = 0;
		Format format = Format::NONE;
	};

	struct InputLayout
	{
		uint32_t stride = 0;
		InputClassification input_class = InputClassification::PER_VERTEX_DATA;
		std::vector<Element> elements;
	};

	struct RasterizerState
	{
		CullMode cull_mode = CullMode::NONE;
		FillMode fill_mode = FillMode::NONE;
	};

	struct DepthStencilState
	{
		bool depth_test_enable = false;
		bool depth_write_enable = true;
		bool stencil_test_enable = false;
		float minDepth = 0.0f;
		float maxDepth = 1.0f;
	};

	struct FrameInfo
	{
		BlendFactor src_color = BlendFactor::BLEND_NONE;
		BlendFactor dst_color = BlendFactor::BLEND_NONE;
		BlendOp color_op = BlendOp::BLEND_OP_NONE;

		BlendFactor src_alpha = BlendFactor::BLEND_NONE;
		BlendFactor dst_alpha = BlendFactor::BLEND_NONE;
		BlendOp alpha_op = BlendOp::BLEND_OP_NONE;
	};

	struct ColorBlendState
	{
		bool alpha_to_blend_coverage = false; // TODO

		std::array<FrameInfo, 5> render_targets;//TODO: Make size configurable
		uint32_t num_render_target = 1;

	};

	struct UniformMetaData
	{
		uint32_t _id = INVALID_ID;
		uint32_t _offset = INVALID_ID;
		uint32_t _size = 0;
		uint32_t _slot = 0;
		uint32_t _index = 0;
		uint32_t _count = 0;
		uint32_t meta_id = 0;// NOTE: It is the combination of the resource stage and the slot
		uint32_t _struct_index = INVALID_ID;
		uint16_t _frame_index = uint16_t(-1); // TODO: Create enum for maximum 16bit integer
		uint16_t _num_frame_update = 0;
		ShaderResourceType _resource_type = ShaderResourceType::SHADER_RESOURCE_TYPE_NOUSE;
		ShaderStage _shader_stage = ShaderStage::STAGE_NONE;
		ShaderData data_type = ShaderData::NONE;
	};

	
	struct ShaderResource
	{

		ShaderStage shader_stage = ShaderStage::STAGE_NONE;
		std::string resource_name = "";
		ShaderResourceType resource_type = ShaderResourceType::SHADER_RESOURCE_TYPE_NOUSE;
		ShaderResourceStage resource_stage = ShaderResourceStage::RESOURCE_STAGE_NONE;
		uint32_t resource_size = 0;
		uint32_t slot = 0;
		uint32_t index = 0;
		uint32_t count = 1;

		struct Member
		{
			std::string resource_name = "";// Used for bindings that of structure types
			uint32_t resource_size = 0;
			ShaderData resource_data_type = ShaderData::NONE;
			uint32_t offset = 0;
		};

		std::vector<Member> members;
		ShaderDataDef def;
	};

	struct ShaderResources
	{		
		std::vector<ShaderResource> resources;
	};

	

	struct PipelineStateDesc
	{
		GShader* vertex_shader = nullptr;
		GShader* pixel_shader = nullptr;
		InputLayout input_layout = {};
		RasterizerState rasteriser_state = {};
		DepthStencilState depth_sten_state = {};
		ColorBlendState blend_state = {};
		PRIMITIVETOPOLOGY topology = PRIMITIVETOPOLOGY::NONE;
		Viewport view_port = {};
		GRenderPass* render_pass = nullptr;
		uint32_t subpass_index = 0;
		RENDERPASS _renderPass = RENDERPASS::MAIN_PASS;

		ShaderResources resources;
	};


	struct Vertex
	{
		glm::vec3 pos;
		glm::vec3 normal;
		glm::vec2 texCoord;
		glm::vec4 tangent;


		static InputLayout get_input_layout()
		{
			InputLayout layout;
			layout.stride = sizeof(Vertex);
			layout.input_class = InputClassification::PER_VERTEX_DATA;

			Element _pos;
			_pos.format = Format::R32G32B32_FLOAT;
			_pos.index = 0;
			_pos.offset = offsetof(Vertex, pos);
			_pos.stride = sizeof(glm::vec3);

			layout.elements.push_back(_pos);

			Element _normal;
			_normal.format = Format::R32G32B32_FLOAT;
			_normal.index = 1;
			_normal.offset = offsetof(Vertex, normal);
			_normal.stride = sizeof(glm::vec3);

			layout.elements.push_back(_normal);

			Element _texCoord;
			_texCoord.format = Format::R32G32_FLOAT;
			_texCoord.index = 2;
			_texCoord.offset = offsetof(Vertex, texCoord);
			_texCoord.stride = sizeof(glm::vec2);

			layout.elements.push_back(_texCoord);

			Element _tangent;
			_tangent.format = Format::R32G32B32A32_FLOAT;
			_tangent.index = 3;
			_tangent.offset = offsetof(Vertex, tangent);
			_tangent.stride = sizeof(glm::vec4);

			layout.elements.push_back(_tangent);

			return layout;
		}

		bool operator==(const Vertex& other) const {
			return pos == other.pos && normal == other.normal && texCoord == other.texCoord;
		}

	};

	struct SkinnedVertex
	{
		glm::vec3 pos;
		glm::vec3 normal;
		glm::vec2 texCoord;
		glm::vec4 tangent;
		int32_t bones_id[MAX_BONE_PER_VERTEX] = {0};
		float bone_weights[MAX_BONE_PER_VERTEX] = {0.0f};


		static InputLayout get_input_layout()
		{
			InputLayout layout;
			layout.stride = sizeof(SkinnedVertex);
			layout.input_class = InputClassification::PER_VERTEX_DATA;

			Element _pos;
			_pos.format = Format::R32G32B32_FLOAT;
			_pos.index = 0;
			_pos.offset = offsetof(SkinnedVertex, pos);
			_pos.stride = sizeof(glm::vec3);

			layout.elements.push_back(_pos);

			Element _normal;
			_normal.format = Format::R32G32B32_FLOAT;
			_normal.index = 1;
			_normal.offset = offsetof(SkinnedVertex, normal);
			_normal.stride = sizeof(glm::vec3);

			layout.elements.push_back(_normal);

			Element _texCoord;
			_texCoord.format = Format::R32G32_FLOAT;
			_texCoord.index = 2;
			_texCoord.offset = offsetof(SkinnedVertex, texCoord);
			_texCoord.stride = sizeof(glm::vec2);

			layout.elements.push_back(_texCoord);

			Element _tangent;
			_tangent.format = Format::R32G32B32A32_FLOAT;
			_tangent.index = 3;
			_tangent.offset = offsetof(SkinnedVertex, tangent);
			_tangent.stride = sizeof(glm::vec4);

			layout.elements.push_back(_tangent);

			Element _bones_id;
			_bones_id.format = Format::R32G32B32A32_SINT;
			_bones_id.index = 4;
			_bones_id.offset = offsetof(SkinnedVertex, bones_id);
			_bones_id.stride = sizeof(int[MAX_BONE_PER_VERTEX]);

			layout.elements.push_back(_bones_id);


			Element _bone_weights;
			_bone_weights.format = Format::R32G32B32A32_FLOAT;
			_bone_weights.index = 5;
			_bone_weights.offset = offsetof(SkinnedVertex, bone_weights);
			_bone_weights.stride = sizeof(float[MAX_BONE_PER_VERTEX]);

			layout.elements.push_back(_bone_weights);

			return layout;
		}

		bool operator==(const SkinnedVertex& other) const {
			return pos == other.pos && normal == other.normal && texCoord == other.texCoord;
		}

	};

	struct Vertex2D
	{
		glm::vec2 pos;
		glm::vec2 texCoord;


		static InputLayout get_input_layout()
		{
			InputLayout layout;
			layout.stride = sizeof(Vertex2D);
			layout.input_class = InputClassification::PER_VERTEX_DATA;

			Element _pos;
			_pos.format = Format::R32G32_FLOAT;
			_pos.index = 0;
			_pos.offset = offsetof(Vertex2D, pos);
			_pos.stride = sizeof(glm::vec2);

			layout.elements.push_back(_pos);

			Element _texCoord;
			_texCoord.format = Format::R32G32_FLOAT;
			_texCoord.index = 1;
			_texCoord.offset = offsetof(Vertex2D, texCoord);
			_texCoord.stride = sizeof(glm::vec2);

			layout.elements.push_back(_texCoord);

			return layout;
		}

		bool operator==(const Vertex2D& other) const {
			return pos == other.pos && texCoord == other.texCoord;
		}

	};

	struct TextVertex
	{
		glm::vec3 pos;
		glm::vec3 color;
		glm::vec2 texCoord;

		static InputLayout get_input_layout()
		{
			InputLayout layout;
			layout.stride = sizeof(TextVertex);
			layout.input_class = InputClassification::PER_VERTEX_DATA;

			Element _pos;
			_pos.format = Format::R32G32B32_FLOAT;
			_pos.index = 0;
			_pos.offset = offsetof(TextVertex, pos);
			_pos.stride = sizeof(glm::vec3);

			layout.elements.push_back(_pos);

			Element _color;
			_color.format = Format::R32G32B32_FLOAT;
			_color.index = 1;
			_color.offset = offsetof(TextVertex, color);
			_color.stride = sizeof(glm::vec3);

			layout.elements.push_back(_color);

			Element _texCoord;
			_texCoord.format = Format::R32G32_FLOAT;
			_texCoord.index = 2;
			_texCoord.offset = offsetof(TextVertex, texCoord);
			_texCoord.stride = sizeof(glm::vec2);

			layout.elements.push_back(_texCoord);

			return layout;
		}

	};


	struct SceneGlobals
	{
		alignas(16) glm::mat4 projection;
		alignas(16) glm::mat4 view;
		alignas(16) glm::vec3 view_position;
		alignas(16) glm::vec2 _test;
	};

	struct TextureDesc
	{
		uint32_t m_width = 0;
		uint32_t m_height = 0;
		uint32_t m_mipLevels = 1;
		Format m_format = Format::NONE;
		BindFlag m_flag = BindFlag::NIL;
		UsageFlag m_usage = UsageFlag::NONE;
		uint32_t m_channels = 0;
		uint32_t m_numLayers = 0;
		ImageType m_image_type = ImageType::NO_TYPE;
		AddressMode m_addressModeU = AddressMode::NONE;
		AddressMode m_addressModeV = AddressMode::NONE;
		AddressMode m_addressModeW = AddressMode::NONE;
		FilterMode m_minFilterMode = FilterMode::NONE;
		FilterMode m_magFilterMode = FilterMode::NONE;
		AttachmentType m_attachmentType = AttachmentType::NONE;
		std::vector<unsigned char*> m_data;
	};

	struct Light
	{
		glm::vec4 position;
		glm::vec4 direction = {0.0f, -1.0f, 0.0f, 0.0f};
		glm::vec4 color = { 0.32f, 0.55f, 0.42f, 0.0f };
		glm::vec4 params1 = { 1.0f, 0.02f, 0.002f, 0.0f }; // x: constant, y: linear, z:quadratic, w: innerCutOff
		glm::vec4 params2 = { 0.0f, 1.0f, 0.0f, 0.0f }; // x: outerCutOff, y: intensity, z:cast_shadow, w: null
	};
	
	uint32_t getFmtSize(Format format);
	uint32_t getShaderDataSize(ShaderData type);

	struct InternalMaterialData
	{
		std::any internal_data;
		uint32_t hash;
		uint32_t offset;
		ShaderData type;
	};
}