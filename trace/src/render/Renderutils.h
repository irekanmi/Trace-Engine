#pragma once

#include "Graphics.h"

namespace trace {

	void AutoFillPipelineDesc(PipelineStateDesc& desc, bool input_layout = true, bool raterizer_state = true, bool depth_sten_state = true, bool color_blend_state = true, bool view_port = true, bool scissor = true, bool render_pass = true, bool primitive_topology = true);
	bool operator ==(ShaderResourceBinding lhs, ShaderResourceBinding rhs);
}
