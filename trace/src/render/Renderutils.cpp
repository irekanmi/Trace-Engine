#include "pch.h"

#include "Renderutils.h"

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

}