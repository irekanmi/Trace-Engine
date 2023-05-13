#include "pch.h"

#include "Renderutils.h"
#include "core/platform/Vulkan/VulkanRenderFunc.h"

#define RENDER_FUNC_IS_VALID(function)                           \
	if(!function)                                                \
	{                                                            \
		result = false;                                          \
		TRC_ERROR(                                               \
	"{} is not available, please check for any errors"           \
		, #function);                                           \
		return result;                                           \
	}
	

namespace trace {



	void AutoFillPipelineDesc(
		PipelineStateDesc& desc,
		bool input_layout, 
		bool raterizer_state, 
		bool depth_sten_state, 
		bool color_blend_state, 
		bool view_port, 
		bool scissor,
		bool render_pass,
		bool primitive_topology
	)
	{

		if (input_layout)
		{
			InputLayout _layout = Vertex::get_input_layout();
			desc.input_layout = _layout;
		}

		if (raterizer_state)
		{
			RaterizerState rs = {};
			rs.cull_mode = CullMode::BACK;
			rs.fill_mode = FillMode::SOLID;

			desc.rateriser_state = rs;
		}

		if (depth_sten_state)
		{
			DepthStencilState dss = {};
			dss.depth_test_enable = true;
			dss.minDepth = 0.0f;
			dss.maxDepth = 1.0f;
			dss.stencil_test_enable = false;
			
			desc.depth_sten_state = dss;
		}

		if (color_blend_state)
		{
			ColorBlendState blds = {};
			blds.alpha_to_blend_coverage = true;

			desc.blend_state = blds;
		}

		if (view_port)
		{
			Viewport vp = {};
			vp.x = vp.y = 0.0f;
			vp.width = 800.0f;
			vp.height = 600.0f;
			vp.minDepth = 0.0f;
			vp.maxDepth = 1.0f;
			
			desc.view_port = vp;
		}

		if (scissor)
		{
			Rect2D rect;
			rect.top = rect.left = 0;
			rect.right = 800;
			rect.bottom = 600;

		}
		
		if (render_pass)
		{
			desc._renderPass = RENDERPASS::MAIN_PASS;
			desc.subpass_index = 0;
		}
		if (primitive_topology)
		{
			desc.topology = PrimitiveTopology::TRIANGLE_LIST;
		}
	}

	bool operator==(ShaderResourceBinding lhs, ShaderResourceBinding rhs)
	{
		bool result = (lhs.count == rhs.count) &&
			(lhs.resource_stage == rhs.resource_stage) &&
			(lhs.resource_type == rhs.resource_type) &&
			(lhs.slot == rhs.slot);
		return result;
	}

	__CreateContext RenderFunc::_createContext = nullptr;
	__DestroyContext RenderFunc::_destroyContext = nullptr;

	__CreateDevice RenderFunc::_createDevice = nullptr;
	__DestroyDevice RenderFunc::_destroyDevice = nullptr;

	__CreateBuffer RenderFunc::_createBuffer = nullptr;
	__ValidateHandle RenderFunc::_validateHandle = nullptr;

	bool RenderFunc::CreateContext(GContext* context)
	{
		bool result = true;

		RENDER_FUNC_IS_VALID(_createContext);
		result = _createContext(context);

		return result;
	}

	bool RenderFunc::DestroyContext(GContext* context)
	{
		bool result = true;

		RENDER_FUNC_IS_VALID(_destroyContext);
		result = _destroyContext(context);

		return result;
	}

	bool RenderFunc::CreateDevice(GDevice* device)
	{
		bool result = true;

		RENDER_FUNC_IS_VALID(_createDevice);
		result = _createDevice(device);

		return result;
	}

	bool RenderFunc::DestroyDevice(GDevice* device)
	{
		bool result = true;

		RENDER_FUNC_IS_VALID(_destroyDevice);
		result = _destroyDevice(device);

		return result;
	}

	bool RenderFunc::CreateBuffer(GBuffer* buffer, BufferInfo* buffer_info)
	{
		bool result = true;

		RENDER_FUNC_IS_VALID(_createBuffer);
		result = _createBuffer(buffer, buffer_info);

		return result;
	}

	bool RenderFunc::ValidateHandle(GHandle* handle)
	{
		bool result = true;

		RENDER_FUNC_IS_VALID(_validateHandle);
		result = _validateHandle(handle);

		return result;
	}

	bool RenderFuncLoader::LoadVulkanRenderFunctions()
	{
		RenderFunc::_createContext = vk::__CreateContext;
		RenderFunc::_destroyContext = vk::__DestroyContext;
		RenderFunc::_createDevice = vk::__CreateDevice;
		RenderFunc::_destroyDevice = vk::__DestroyDevice;
		return false;
	}

}