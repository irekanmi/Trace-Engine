#pragma once

#include "core/Core.h"
#include "core/Enums.h"
#include "Graphics.h"
#include "resource/Resource.h"
#include "resource/Ref.h"
#include "GHandle.h"
#include "scene/UUID.h"

namespace trace {


	class GTexture : public Resource
	{

	public:
		GTexture(){};
		virtual ~GTexture(){};

		TextureDesc& GetTextureDescription();
		void SetTextureDescription(TextureDesc& description) { m_desc = description; }

		GHandle* GetRenderHandle() { return &m_renderHandle; }


		static Ref<GTexture> Deserialize(UUID id);
	private:
		GHandle m_renderHandle;
		TextureDesc m_desc;

	protected:

	};

}
