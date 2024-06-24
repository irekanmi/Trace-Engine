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

		TextureDesc& GetTextureDescription();
		void SetTextureDescription(TextureDesc& description) { m_desc = description; }

		GHandle* GetRenderHandle() { return &m_renderHandle; }


	private:
		GHandle m_renderHandle;
		TextureDesc m_desc;

	protected:

	};

}
