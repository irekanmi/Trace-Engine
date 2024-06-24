#include "pch.h"

#include "FontManager.h"
#include "backends/Fontutils.h"
#include "render/GTexture.h"
#include "core/Utils.h"
#include "serialize/AssetsInfo.h"
#include "serialize/FileStream.h"
#include "core/Coretypes.h"

namespace trace {

	extern std::string GetNameFromUUID(UUID uuid);


	bool FontManager::Init(uint32_t num_font_units)
	{
		m_numEntries = num_font_units;

		m_fonts.resize(m_numEntries);
		for (uint32_t i = 0; i < m_numEntries; i++)
		{
			m_fonts[i].m_id = INVALID_ID;
		}

		hashTable.Init(m_numEntries);
		hashTable.Fill(INVALID_ID);

		// Temp
		FontFuncLoader::Load_MSDF_Func();

		// Trying to determine directory where textures are located..............
		std::filesystem::path current_search_path("./assets");

		if (std::filesystem::exists(current_search_path))
		{
			current_search_path /= "fonts";
			if (std::filesystem::exists(current_search_path))
			{
				font_resource_path = current_search_path;
			}
		}
		else if (std::filesystem::exists(std::filesystem::path("../../assets")))
		{
			current_search_path.clear();
			current_search_path = std::filesystem::path("../../assets");
			if (std::filesystem::exists(current_search_path))
			{
				current_search_path /= "fonts";
				if (std::filesystem::exists(current_search_path))
				{
					font_resource_path = current_search_path;
				}
			}
		}
		else if (std::filesystem::exists(std::filesystem::path("../../../assets")))
		{
			current_search_path.clear();
			current_search_path = std::filesystem::path("../../../assets");
			if (std::filesystem::exists(current_search_path))
			{
				current_search_path /= "fonts";
				if (std::filesystem::exists(current_search_path))
				{
					font_resource_path = current_search_path;
				}
			}
		}
		else
		{
			current_search_path.clear();
			current_search_path = std::filesystem::path("../assets");
			if (std::filesystem::exists(current_search_path))
			{
				current_search_path /= "fonts";
				if (std::filesystem::exists(current_search_path))
				{
					font_resource_path = current_search_path;
				}
			}
		}
		// .............................


		return true;
	}

	void FontManager::Shutdown()
	{
		if (!m_fonts.empty())
		{
			for (Font& font : m_fonts)
			{
				if (font.m_id == INVALID_ID)
					continue;
				TRC_DEBUG("Unloaded Font name:{}, RefCount : {}", font.GetFontName(), font.m_refCount);
				FontFunc::DestroyFont(&font);
			}
			m_fonts.clear();
		}

	}

	bool FontManager::LoadDefaults()
	{
		return true;
	}

	Ref<Font> FontManager::LoadFont(const std::string& name)
	{
		return LoadFont_((font_resource_path / name).string());
	}

	Ref<Font> FontManager::LoadFont_(const std::string& path)
	{
		std::filesystem::path p(path);
		std::string name = p.filename().string();
		Ref<Font> result;
		Font* _font = nullptr;
		result = GetFont(name);
		if (result)
		{
			TRC_WARN("{} font has been loaded", name);
			return result;
		}

		uint32_t i = 0;
		for (Font& font : m_fonts)
		{
			if (font.m_id == INVALID_ID)
			{
				std::string file_path = path;
				if (!FontFunc::LoadAndInitializeFont(file_path, &font))
				{
					TRC_ERROR("Failed to load to font, path->{}", file_path);
					return result;
				}
				font.SetFontName(name);
				font.m_id = i;
				hashTable.Set(name, i);
				_font = &font;
				_font->m_path = p;
				break;
			}

			i++;
		}

		result = { _font, BIND_RENDER_COMMAND_FN(FontManager::UnloadFont) };
		return result;
	}

	Ref<Font> FontManager::GetFont(const std::string& name)
	{
		Ref<Font> result;
		Font* _font = nullptr;
		uint32_t hash = hashTable.Get(name);
		if (hash == INVALID_ID)
		{
			TRC_WARN("{} font has not been loaded", name);
			return result;
		}
		_font = &m_fonts[hash];
		if (_font->m_id == INVALID_ID)
		{
			TRC_WARN("{} font has been destroyed", name);
			hash = INVALID_ID;
			return result;
		}
		result = { _font, BIND_RENDER_COMMAND_FN(FontManager::UnloadFont) };
		return result;
	}

	void FontManager::UnloadFont(Font* font)
	{
		// TODO: Destroy unreferenced font
	}

	Ref<Font> FontManager::LoadFont_Runtime(UUID id)
	{
		std::string name = GetNameFromUUID(id);
		Ref<Font> result;

		auto it = m_assetMap.find(id);
		if (it == m_assetMap.end())
		{
			TRC_WARN("{} is not available in the build", id);
			return result;
		}

		Font* _font = nullptr;
		result = GetFont(name);
		if (result)
		{
			TRC_WARN("{} font has been loaded", name);
			return result;
		}

		uint32_t i = 0;
		for (Font& font : m_fonts)
		{
			if (font.m_id == INVALID_ID)
			{
				std::string bin_dir;
				FindDirectory(AppSettings::exe_path, "Data/trfnt.trbin", bin_dir);
				FileStream stream(bin_dir, FileMode::READ);

				stream.SetPosition(it->second.offset);
				int file_size = 0;
				stream.Read<int>(file_size);
				char* data = new char[file_size];//TODO: Use Custom Allocator
				stream.Read(data, file_size);

				if (!FontFunc::LoadAndInitializeFont(name, &font, data, file_size))
				{
					TRC_ERROR("Failed to load to font, name->{}", name);
					delete[] data;//TODO: Use Custom Allocator
					return result;
				}

				delete[] data;//TODO: Use Custom Allocator
				font.SetFontName(name);
				font.m_id = i;
				hashTable.Set(name, i);
				_font = &font;
				break;
			}

			i++;
		}

		result = { _font, BIND_RENDER_COMMAND_FN(FontManager::UnloadFont) };
		return result;
	}

	FontManager* FontManager::get_instance()
	{
		static FontManager* s_instance = new FontManager;
		return s_instance;
	}

}