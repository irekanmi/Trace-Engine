#pragma once

#include "core/Core.h"
#include "core/Enums.h"
#include <stdint.h>

namespace trace {

	struct GBufferData
	{
		uint32_t position_index;
		uint32_t normal_index;
		uint32_t color_index;
		uint32_t depth_index;
	};

	struct LightningData
	{
		uint32_t light_output;
	};

	struct FrameData
	{
		uint32_t ldr_index;
		uint32_t frame_width;
		uint32_t frame_height;
	};

	struct SSAOData
	{
		uint32_t ssao_main;
		uint32_t ssao_blur;
	};

}
