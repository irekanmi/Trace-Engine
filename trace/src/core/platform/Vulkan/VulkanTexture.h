#pragma once

#include "render/GTexture.h"
#include "VKtypes.h"


namespace vk {

	bool __CreateTexture(trace::GTexture* texture, trace::TextureDesc desc);
	bool __DestroyTexture(trace::GTexture* texture);
	bool __GetTextureNativeHandle(trace::GTexture* texture, void*& out_handle);
	bool __GetTextureData(trace::GTexture* texture, void*& out_data);

}
