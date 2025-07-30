#include "pch.h"

#include "Font.h"
#include "GTexture.h"
#include "external_utils.h"
#include "core/Coretypes.h"

#include "backends/Fontutils.h"
#include "core/io/Logging.h"
#include "resource/GenericAssetManager.h"

#include <filesystem>

namespace trace {

	bool Font::Create(const std::string& path)
	{
		std::filesystem::path p(path);
		std::string name = p.filename().string();

		if (!FontFunc::LoadAndInitializeFont(path, this))
		{
			TRC_ERROR("Failed to load to font, path->{}", path);
			return false;
		}

		m_fontName = name;
		return true;
	}
	
	bool Font::Create(const std::string& name, DataStream* stream)
	{
		int file_size = 0;
		stream->Read<int>(file_size);
		char* data = new char[file_size];//TODO: Use Custom Allocator
		stream->Read(data, file_size);

		if (!FontFunc::LoadAndInitializeFont(name, this, data, file_size))
		{
			TRC_ERROR("Failed to load to font, name->{}", name);
			delete[] data;//TODO: Use Custom Allocator
			return false;
		}

		delete[] data;//TODO: Use Custom Allocator
		m_fontName = name;
		return true;
	}

	void Font::Destroy()
	{
		// TODO: Destroy unreferenced font
		m_atlas.free();
	}

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
			result = GenericAssetManager::get_instance()->CreateAssetHandle_<Font>(p.string(), p.string());	
		}
		else
		{
			result = GenericAssetManager::get_instance()->Load_Runtime<Font>(id);
		}

		return result;
	}

	Ref<Font> Font::Deserialize(DataStream* stream)
	{
		std::string name;
		stream->Read<std::string>(name);
		Ref<Font> result = GenericAssetManager::get_instance()->CreateAssetHandle_<Font>(name, name, stream);
		return result;
	}

}