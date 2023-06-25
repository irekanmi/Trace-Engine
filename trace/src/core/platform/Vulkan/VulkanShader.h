#pragma once

#include "render/GShader.h"
#include  "VKtypes.h"



namespace vk {

	bool __CreateShader(trace::GShader* shader, const std::string& src, trace::ShaderStage stage);
	bool __DestroyShader(trace::GShader* shader);

}