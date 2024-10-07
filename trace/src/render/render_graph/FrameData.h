#pragma once

#include "core/Core.h"
#include "core/Enums.h"
#include "core/defines.h"

#include "glm/glm.hpp"
#include <stdint.h>
#include <array>

namespace trace {

	struct GBufferData
	{
		uint32_t position_index = INVALID_ID;
		uint32_t normal_index = INVALID_ID;
		uint32_t color_index = INVALID_ID;
		uint32_t depth_index = INVALID_ID;
		uint32_t emissive_index = INVALID_ID;
	};

	struct LightingData
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

	struct ShadowMapData
	{
		int32_t start_texture_index = -1; // NOTE: Used to link shadow pass with lighting pass
		uint32_t total_shadowed_lights = 0;
		std::array<glm::mat4, MAX_SHADOW_SUN_LIGHTS> sun_view_proj_matrices;
		std::array<glm::mat4, MAX_SHADOW_SPOT_LIGHTS> spot_view_proj_matrices;
	};

}
