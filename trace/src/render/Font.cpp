#include "pch.h"

#include "Font.h"
#include "GTexture.h"

namespace trace {

	

	Ref<GTexture> Font::GetAtlas()
	{
		return m_atlas;
	}

	void* Font::GetInternal()
	{
		return m_internal;
	}

	std::string& Font::GetFontName()
	{
		return m_fontName;
	}

	void Font::SetAtlas(Ref<GTexture> atlas)
	{
		m_atlas = atlas;
	}

	void Font::SetInternal(void* value)
	{
		m_internal = value;
	}

	void Font::SetFontName(const std::string& name)
	{
		m_fontName = name;
	}

}