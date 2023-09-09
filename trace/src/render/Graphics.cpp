#include "pch.h"

#include "core/Core.h"
#include "core/Enums.h"

#include "Graphics.h"

namespace trace {

	uint32_t getFmtSize(Format format)
	{
		switch (format)
		{
		case Format::R16G16B16A16_FLOAT:
		{
			return 8;
		}
		case Format::R16G16B16_FLOAT:
		{
			return 6;
		}
		case Format::R16_FLOAT:
		{
			return 2;
		}
		case Format::R32G32B32A32_FLOAT:
		{
			return 16;
		}
		case Format::R32G32B32_FLOAT:
		{
			return 12;
		}
		case Format::R32G32B32_UINT:
		{
			return 12;
		}
		case Format::R32G32_FLOAT:
		{
			return 8;
		}
		case Format::R32G32_UINT:
		{
			return 8;
		}
		case Format::R8G8B8A8_RBG:
		case Format::R8G8B8A8_SNORM:
		case Format::R8G8B8A8_SRBG:
		case Format::R8G8B8A8_UNORM:
		{
			return 4;
		}
		case Format::R8G8B8_SNORM:
		case Format::R8G8B8_UNORM:
		case Format::R8G8B8_SRBG:
		{
			return 3;
		}
		}
	}

}