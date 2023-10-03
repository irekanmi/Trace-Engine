#include "pch.h"
#include "TextureManager.h"
#include "render/Renderutils.h"
#include <new>
#include "core/Platform.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace trace {

	TextureManager* TextureManager::s_instance = nullptr;



	TextureManager::TextureManager()
	{
	}

	TextureManager::~TextureManager()
	{

	}

	bool TextureManager::Init(uint32_t maxTextureUnits)
	{
		m_textureUnits = maxTextureUnits;
		m_hashTable.Init(maxTextureUnits);
		
		m_textures.resize(m_textureUnits);

		for (uint32_t i = 0; i < m_textureUnits; i++)
		{
			m_textures[i].m_id = INVALID_ID;
		}

		TextureHash _hash;
		_hash._id = INVALID_ID;
		m_hashTable.Fill(_hash);

		// Trying to determine directory where textures are located..............
		std::filesystem::path current_search_path("./assets");

		if (std::filesystem::exists(current_search_path))
		{
			current_search_path /= "textures";
			if (std::filesystem::exists(current_search_path))
			{
				texture_resource_path = current_search_path;
			}
		}
		else if (std::filesystem::exists(std::filesystem::path("../../assets")))
		{
			current_search_path.clear();
			current_search_path = std::filesystem::path("../../assets");
			if (std::filesystem::exists(current_search_path))
			{
				current_search_path /= "textures";
				if (std::filesystem::exists(current_search_path))
				{
					texture_resource_path = current_search_path;
				}
			}
		}
		else if (std::filesystem::exists(std::filesystem::path("../../../assets")))
		{
			current_search_path.clear();
			current_search_path = std::filesystem::path("../../../assets");
			if (std::filesystem::exists(current_search_path))
			{
				current_search_path /= "textures";
				if (std::filesystem::exists(current_search_path))
				{
					texture_resource_path = current_search_path;
				}
			}
		}
		else
		{
			current_search_path.clear();
			current_search_path = std::filesystem::path("../assets");
			if (std::filesystem::exists(current_search_path))
			{
				current_search_path /= "textures";
				if (std::filesystem::exists(current_search_path))
				{
					texture_resource_path = current_search_path;
				}
			}
		}
		// .............................

		return true;
	}

	void TextureManager::ShutDown()
	{
		default_diffuse_map.~Ref();
		default_specular_map.~Ref();
		default_normal_map.~Ref();
		if (!m_textures.empty())
		{
			for (GTexture& tex : m_textures)
			{
				if (tex.m_id == INVALID_ID)
					continue;
				TRC_DEBUG("Unloaded texture  id:{}", tex.m_id);
				RenderFunc::DestroyTexture(&tex);
				tex.~GTexture();
			}
			m_textures.clear();
		}

	}

	Ref<GTexture> TextureManager::GetTexture(const std::string& name)
	{
		Ref<GTexture> result;
		GTexture* _tex = nullptr;
		TextureHash& _hash = m_hashTable.Get_Ref(name);

		if (_hash._id == INVALID_ID)
		{
			//TODO: replace with the default texture
			TRC_ERROR("Please enter a vaild texture");
			return result;
		}

		_tex = &m_textures[_hash._id];
		if (_tex->m_id == INVALID_ID)
		{
			TRC_WARN("{} texture has been destroyed", name);
			_hash._id = INVALID_ID;
			return result;
		}
		result = { _tex, BIND_RENDER_COMMAND_FN(TextureManager::UnloadTexture) };
		return result;
	}

	Texture_Ref TextureManager::GetDefault(const std::string& name)
	{
		if (m_hashTable.Hash(name) == m_hashTable.Hash("albedo_map"))
		{
			return default_diffuse_map;
		}
		else if (m_hashTable.Hash(name) == m_hashTable.Hash("specular_map"))
		{
			return default_specular_map;
		}
		else if (m_hashTable.Hash(name) == m_hashTable.Hash("normal_map"))
		{
			return default_normal_map;
		}

		return Texture_Ref();
	}

	Ref<GTexture> TextureManager::CreateTexture(const std::string& name, TextureDesc desc)
	{
		Ref<GTexture> result;
		GTexture* _tex = nullptr;
		if (m_hashTable.Get(name)._id != INVALID_ID)
		{
			TRC_WARN("Texture has already being loaded {}", name);
			return GetTexture(name);
		}

		TRC_ASSERT(desc.m_width != 0 || desc.m_height != 0, "width or height of a texture can not zero(0)");

		uint32_t i = 0;
		for (GTexture& tex : m_textures)
		{
			if (tex.m_id == INVALID_ID)
			{
				RenderFunc::CreateTexture(&tex, desc);
				delete[] desc.m_data[0];// TODO: Use custom allocator
				tex.m_id = i;
				TextureHash _hash;
				_hash._id = i;
				_tex = &m_textures[_hash._id];
				m_hashTable.Set(name, _hash);
				break;
			}
			i++;
		}

		result = { _tex, BIND_RENDER_COMMAND_FN(TextureManager::UnloadTexture) };
		return result;
	}

	Ref<GTexture> TextureManager::LoadTexture(const std::string& name)
	{
		return LoadTexture_((texture_resource_path / name).string());
	}

	Ref<GTexture> TextureManager::LoadTexture_(const std::string& path)
	{
		Ref<GTexture> result;
		GTexture* _tex = nullptr;
		std::filesystem::path p(path);
		std::string name = p.filename().string();
		result = GetTexture(name);
		if (result)
		{
			TRC_WARN("Texture has already being loaded {}", name);
			return result;
		}
		int _width, _height, _channels;
		unsigned char* pixel_data = nullptr;

		stbi_set_flip_vertically_on_load(true);
		pixel_data = stbi_load(p.string().c_str(), &_width, &_height, &_channels, STBI_rgb_alpha);
		if (!pixel_data)
		{
			TRC_ERROR("Unable to load texture {}: Error=> {}", name.c_str(), stbi_failure_reason());
			stbi__err(0, 0);
			return result;
		}

		TextureDesc texture_desc;
		texture_desc.m_addressModeU = texture_desc.m_addressModeW = texture_desc.m_addressModeV = AddressMode::REPEAT;
		texture_desc.m_channels = 4;
		texture_desc.m_format = Format::R8G8B8A8_UNORM;
		texture_desc.m_minFilterMode = texture_desc.m_magFilterMode = FilterMode::LINEAR;
		texture_desc.m_flag = BindFlag::SHADER_RESOURCE_BIT;
		texture_desc.m_usage = UsageFlag::DEFAULT;
		texture_desc.m_image_type = ImageType::IMAGE_2D;
		texture_desc.m_mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(_width, _height))) / 2) + 1;
		texture_desc.m_width = (uint32_t)_width;
		texture_desc.m_height = (uint32_t)_height;
		texture_desc.m_data.push_back(pixel_data);
		texture_desc.m_numLayers = 1;


		uint32_t i = 0;
		for (GTexture& tex : m_textures)
		{
			if (tex.m_id == INVALID_ID)
			{
				RenderFunc::CreateTexture(&tex, texture_desc);
				stbi_image_free(pixel_data);
				tex.m_id = i;
				TextureHash _hash;
				_hash._id = i;
				tex.m_path = p;
				_tex = &m_textures[_hash._id];
				m_hashTable.Set(name, _hash);
				break;
			}

			i++;
		}
		result = { _tex, BIND_RENDER_COMMAND_FN(TextureManager::UnloadTexture) };
		return result;
	}

	Ref<GTexture> TextureManager::LoadTexture(const std::string& name, TextureDesc desc)
	{
		return LoadTexture_((texture_resource_path / name).string(), desc);
	}

	Ref<GTexture> TextureManager::LoadTexture_(const std::string& path, TextureDesc desc)
	{
		Ref<GTexture> result;
		GTexture* _tex = nullptr;
		std::filesystem::path p(path);
		std::string name = p.filename().string();
		result = GetTexture(name);
		if (result)
		{
			TRC_WARN("Texture has already being loaded {}", name);
			return result;
		}

		int _width, _height, _channels;
		unsigned char* pixel_data = nullptr;

		stbi_set_flip_vertically_on_load(true);
		pixel_data = stbi_load(p.string().c_str(), &_width, &_height, &_channels, STBI_rgb_alpha);
		if (!pixel_data)
		{
			TRC_ERROR("Unable to load texture {}: Error=> {}", name.c_str(), stbi_failure_reason());
			stbi__err(0, 0);
			return result;
		}

		desc.m_width = _width;
		desc.m_height = _height;
		desc.m_channels = 4;
		desc.m_data.push_back(pixel_data);
		desc.m_numLayers = 1;
		desc.m_image_type = ImageType::IMAGE_2D;
		desc.m_mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(_width, _height))) / 2) + 1;

		uint32_t i = 0;
		for (GTexture& tex : m_textures)
		{
			if (tex.m_id == INVALID_ID)
			{
				RenderFunc::CreateTexture(&tex, desc);
				stbi_image_free(pixel_data);
				tex.m_id = i;
				TextureHash _hash;
				_hash._id = i;
				tex.m_path = p;
				_tex = &m_textures[_hash._id];
				m_hashTable.Set(name, _hash);
				break;
			}
			i++;
		}

		result = { _tex, BIND_RENDER_COMMAND_FN(TextureManager::UnloadTexture) };
		return result;
	}

	Ref<GTexture> TextureManager::LoadTexture(const std::vector<std::string>& filenames, TextureDesc desc,const std::string& name)
	{
		Ref<GTexture> result;
		GTexture* _tex = nullptr;
		if (m_hashTable.Get(name)._id != INVALID_ID)
		{
			TRC_WARN("Texture has already being loaded {}", name);
			return GetTexture(name);
		}
		desc.m_numLayers = static_cast<uint32_t>(filenames.size());

		for (uint32_t i = 0; i < desc.m_numLayers; i++)
		{
			int _width, _height, _channels;
			unsigned char* pixel_data = nullptr;

			stbi_set_flip_vertically_on_load(false);
			pixel_data = stbi_load((texture_resource_path / filenames[i]).string().c_str(), &_width, &_height, &_channels, STBI_rgb_alpha);
			if (!pixel_data)
			{
				TRC_ERROR("Unable to load texture {}: Error=> {}", name.c_str(), stbi_failure_reason());
				stbi__err(0, 0);
				return result;
			}

			desc.m_width = _width;
			desc.m_height = _height;
			desc.m_channels = 4;
			desc.m_data.push_back(pixel_data);

			
		}
		stbi_set_flip_vertically_on_load(true);

		uint32_t i = 0;
		for (GTexture& tex : m_textures)
		{
			if (tex.m_id == INVALID_ID)
			{
				RenderFunc::CreateTexture(&tex, desc);
				for (unsigned char*& i : desc.m_data)
				{
					stbi_image_free(i);
					i = nullptr;
				}
				tex.m_id = i;
				TextureHash _hash;
				_hash._id = i;
				_tex = &m_textures[_hash._id];
				m_hashTable.Set(name, _hash);
				break;
			}
			i++;
		}
		result = { _tex, BIND_RENDER_COMMAND_FN(TextureManager::UnloadTexture) };
		return result;
	}

	void TextureManager::UnloadTexture(GTexture* texture)
	{
		if (texture->m_refCount > 0)
		{
			TRC_WARN("Can't release a texture that is in use");
			return;
		}

		RenderFunc::DestroyTexture(texture);
		texture->~GTexture();
		texture->m_id = INVALID_ID;
	}

	void TextureManager::ReleaseTexture(const std::string& name)
	{
		TextureHash& _hash = m_hashTable.Get_Ref(name);

		if (_hash._id != INVALID_ID)
		{
			GTexture* value = &m_textures[_hash._id];
			UnloadTexture(value);
			return;
		}

		TRC_WARN("Can't release a texture that hasn't been loaded please ensure texture is loaded");
		return;
	}

	bool TextureManager::LoadDefaultTextures()
	{
		uint32_t dimension = 256;
		uint32_t channels = 4;
			
		unsigned char* pixel = new unsigned char[dimension * dimension * channels];
		memset(pixel, 255, dimension * dimension * channels);
		for (uint32_t row = 0; row < dimension; row++)
		{
			for (uint32_t coloumn = 0; coloumn < dimension; coloumn++)
			{
				uint32_t index = (row * dimension) + coloumn;
				uint32_t _idx = index * channels;
				if (row % 2)
				{
					if (coloumn % 2)
					{
						pixel[_idx + 0] = 255;//0;
						pixel[_idx + 1] = 255;//0;
					}
				}
				else
				{
					if (!(coloumn % 2))
					{
						pixel[_idx + 0] = 255;//0;
						pixel[_idx + 1] = 255;//0;
					}
				}
			}
		}

		TextureDesc texture_desc;
		texture_desc.m_addressModeU = texture_desc.m_addressModeW = texture_desc.m_addressModeV = AddressMode::REPEAT;
		texture_desc.m_channels = 4;
		texture_desc.m_format = Format::R8G8B8A8_UNORM;
		texture_desc.m_minFilterMode = texture_desc.m_magFilterMode = FilterMode::LINEAR;
		texture_desc.m_flag = BindFlag::SHADER_RESOURCE_BIT;
		texture_desc.m_usage = UsageFlag::DEFAULT;
		texture_desc.m_width = (uint32_t)dimension;
		texture_desc.m_height = (uint32_t)dimension;
		texture_desc.m_image_type = ImageType::IMAGE_2D;
		texture_desc.m_data.push_back(pixel);
		texture_desc.m_numLayers = 1;
		texture_desc.m_mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(dimension, dimension))) / 2) + 1;

		// Default Diffuse texture
		RenderFunc::CreateTexture(&default_diffuse_texture, texture_desc);
		default_diffuse_texture.m_path = "albedo_map";
		default_diffuse_map = { &default_diffuse_texture, BIND_RESOURCE_UNLOAD_FN(TextureManager::UnloadDefaults, this) };
		delete[] pixel;

		dimension = 16;
		channels = 4;

		

		pixel = new unsigned char[dimension * dimension * channels];
		memset(pixel, 0, dimension * dimension * channels);
		texture_desc.m_width = (uint32_t)dimension;
		texture_desc.m_height = (uint32_t)dimension;
		texture_desc.m_channels = 4;
		texture_desc.m_data[0] = (pixel);
		texture_desc.m_mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(dimension, dimension))) / 2) + 1;


		for (uint32_t row = 0; row < dimension; row++)
		{
			for (uint32_t coloumn = 0; coloumn < dimension; coloumn++)
			{
				uint32_t index = (row * dimension) + coloumn;
				uint32_t _idx = index * channels;
				pixel[_idx + 0] = 255;
				pixel[_idx + 1] = 255;
				pixel[_idx + 2] = 255;
				pixel[_idx + 3] = 255;
			}
		}
		
		// Default specular
		RenderFunc::CreateTexture(&default_specular_texture, texture_desc);
		default_specular_texture.m_path = "specular_map";
		default_specular_map = { &default_specular_texture, BIND_RESOURCE_UNLOAD_FN(TextureManager::UnloadDefaults, this) };
		delete[] pixel;

		dimension = 16;
		channels = 4;



		pixel = new unsigned char[dimension * dimension * channels];
		memset(pixel, 0, dimension * dimension * channels);
		texture_desc.m_width = (uint32_t)dimension;
		texture_desc.m_height = (uint32_t)dimension;
		texture_desc.m_channels = 4;
		texture_desc.m_data[0] = pixel;
		texture_desc.m_format = Format::R8G8B8A8_UNORM;
		texture_desc.m_mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(dimension, dimension))) / 2) + 1;


		for (uint32_t row = 0; row < dimension; row++)
		{
			for (uint32_t coloumn = 0; coloumn < dimension; coloumn++)
			{
				uint32_t index = (row * dimension) + coloumn;
				uint32_t _idx = index * channels;
				pixel[_idx + 0] = 128;
				pixel[_idx + 1] = 128;
				pixel[_idx + 2] = 255;
				pixel[_idx + 3] = 255;
			}
		}

		// Default normal
		RenderFunc::CreateTexture(&default_normal_texture, texture_desc);
		default_normal_texture.m_path = "normal_map";
		default_normal_map = { &default_normal_texture, BIND_RESOURCE_UNLOAD_FN(TextureManager::UnloadDefaults, this) };
		delete[] pixel;

		
		return true;
	}

	TextureManager* TextureManager::get_instance()
	{

		if (!s_instance)
		{
			s_instance = new TextureManager();
		}
		return s_instance;
	}

	void TextureManager::UnloadDefaults(GTexture* texture)
	{
		if (texture->m_refCount > 0)
		{
			TRC_WARN("Can't release a texture that is in use");
			return;
		}

		RenderFunc::DestroyTexture(texture);
		texture->~GTexture();
	}

}
