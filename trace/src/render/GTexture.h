#pragma once

#include "core/Core.h"
#include "core/Enums.h"
#include "Graphics.h"
#include "resource/Resource.h"
#include "GHandle.h"

namespace trace {


	class TRACE_API GTexture : public Resource
	{

	public:
		GTexture(){};
		virtual ~GTexture(){};

		TextureDesc GetTextureDescription();

		GHandle* GetRenderHandle() { return &m_renderHandle; }

		static GTexture* Create_(TextureDesc description);

	private:
		GHandle m_renderHandle;

	protected:
		TextureDesc m_desc;

	};

}
