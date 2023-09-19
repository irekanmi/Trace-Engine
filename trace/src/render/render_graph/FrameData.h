#pragma once

#include "core/Core.h"
#include "core/Enums.h"
#include <stdint.h>

namespace trace {

	struct GBufferData
	{
		uint32_t position_index = INVALID_ID;
		uint32_t normal_index = INVALID_ID;
		uint32_t color_index = INVALID_ID;
		uint32_t depth_index = INVALID_ID;
	};

	struct LightningData
	{
		uint32_t light_output = INVALID_ID;
	};

	struct FrameData
	{
		uint32_t hdr_index = INVALID_ID;
		uint32_t ldr_index = INVALID_ID;
		uint32_t swapchain_index = INVALID_ID;
		uint32_t frame_width;
		uint32_t frame_height;
		FrameSettings frame_settings;
	};

	struct SSAOData
	{
		uint32_t ssao_main = INVALID_ID;
		uint32_t ssao_blur = INVALID_ID;
	};

	struct BloomData
	{
		uint32_t bloom_samples[10];
		uint8_t samples_count;
	};

}
