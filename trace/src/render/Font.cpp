#include "pch.h"

#include "Font.h"
#include "GTexture.h"
#include "external_utils.h"
#include "core/Coretypes.h"
#include "resource/FontManager.h"

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

	Ref<Font> Font::Deserialize(UUID id)
	{
		Ref<Font> result;

		if (AppSettings::is_editor)
		{
			std::filesystem::path p = GetPathFromUUID(id);
			result = FontManager::get_instance()->GetFont(p.filename().string());
			if (!result)
			{
				FontManager::get_instance()->LoadFont_(p.string());
			}
		}
		else
		{
			result = FontManager::get_instance()->LoadFont_Runtime(id);
		}

		return result;
	}

}