#pragma once

#include "core/Core.h"
#include "core/Enums.h"
#include "Graphics.h"
#include "resource/Resource.h"

namespace trace {


	class TRACE_API GTexture : public Resource
	{

	public:
		GTexture(){};
		virtual ~GTexture(){};

		TextureDesc GetTextureDescription() { return m_desc; }

		static GTexture* Create_(TextureDesc description);

	private:
	protected:
		TextureDesc m_desc;

	};

}
