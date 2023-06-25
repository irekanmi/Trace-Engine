#pragma once

#include "render/GTexture.h"
#include "VKtypes.h"


namespace vk {

	bool __CreateTexture(trace::GTexture* texture, trace::TextureDesc desc);
	bool __DestroyTexture(trace::GTexture* texture);

}
