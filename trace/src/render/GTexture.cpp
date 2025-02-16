#include "pch.h"

#include "GTexture.h"
#include "external_utils.h"
#include "core/Coretypes.h"
#include "resource/TextureManager.h"

namespace trace {

	TextureDesc& GTexture::GetTextureDescription()
	{
		return m_desc;
	}

	Ref<GTexture> GTexture::Deserialize(UUID id)
	{
		Ref<GTexture> result;

		if (AppSettings::is_editor)
		{
			result = LoadTexture(id);
		}
		else
		{
			result = TextureManager::get_instance()->LoadTexture_Runtime(id);
		}

		return result;
	}


}