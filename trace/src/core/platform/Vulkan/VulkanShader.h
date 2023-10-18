#pragma once

#include "render/GShader.h"
#include  "VKtypes.h"



namespace vk {

	bool __CreateShader(trace::GShader* shader, const std::string& src, trace::ShaderStage stage);
	bool __CreateShader_(trace::GShader* shader, std::vector<uint32_t>& src, trace::ShaderStage stage);
	bool __DestroyShader(trace::GShader* shader);

}