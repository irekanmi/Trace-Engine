#pragma once

#include "core/Core.h"
#include "core/Enums.h"
#include "Graphics.h"
#include "resource/Resource.h"

namespace trace {

	enum class AddressMode
	{
		NONE,
		REPEAT,
		MIRRORED_REPEAT
	};

	enum class FilterMode
	{
		NONE,
		LINEAR
	};

	struct TextureDesc
	{
		uint32_t m_width = 0;
		uint32_t m_height = 0;
		Format m_format = Format::NONE;
		uint32_t m_channels = 0;
		unsigned char* m_data = nullptr;
		AddressMode m_addressModeU = AddressMode::NONE;
		AddressMode m_addressModeV = AddressMode::NONE;
		AddressMode m_addressModeW = AddressMode::NONE;
		FilterMode m_minFilterMode = FilterMode::NONE;
		FilterMode m_magFilterMode = FilterMode::NONE;
	};

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
