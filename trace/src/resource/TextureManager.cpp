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

	GTexture* TextureManager::GetTexture(const std::string& name)
	{

		TextureHash& _hash = m_hashTable.Get_Ref(name);

		if (_hash._id != INVALID_ID)
		{
			return &m_textures[_hash._id];
		}
		else
		{

			if (LoadTexture(name))
			{
				return &m_textures[_hash._id];
			}

		}

		//TODO: replace with the default texture
		TRC_ERROR("Please enter a vaild texture");
		return nullptr;
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

	bool TextureManager::LoadTexture(const std::string& name)
	{
		if (m_hashTable.Get(name)._id != INVALID_ID)
		{
			TRC_WARN("Texture has already being loaded {}", name);
			return true;
		}
		int _width, _height, _channels;
		unsigned char* pixel_data = nullptr;

		stbi_set_flip_vertically_on_load(true);
		pixel_data = stbi_load((texture_resource_path / name).string().c_str(), &_width, &_height, &_channels, STBI_rgb_alpha);
		if (!pixel_data)
		{
			TRC_ERROR("Unable to load texture {}: Error=> {}", name.c_str(), stbi_failure_reason());
			stbi__err(0, 0);
			return false;
		}




		TextureDesc texture_desc;
		texture_desc.m_addressModeU = texture_desc.m_addressModeW = texture_desc.m_addressModeV = AddressMode::REPEAT;
		texture_desc.m_channels = 4;
		texture_desc.m_format = Format::R8G8B8A8_SRBG;
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
				tex.m_id = i;
				TextureHash _hash;
				_hash._id = i;
				m_hashTable.Set(name, _hash);
				return true;
			}
	
			i++;
		}


		return true;
	}

	bool TextureManager::LoadTexture(const std::string& name, TextureDesc desc)
	{

		if (m_hashTable.Get(name)._id != INVALID_ID)
		{
			TRC_WARN("Texture has already being loaded {}", name);
			return true;
		}

		int _width, _height, _channels;
		unsigned char* pixel_data = nullptr;

		stbi_set_flip_vertically_on_load(true);
		pixel_data = stbi_load((texture_resource_path / name).string().c_str(), &_width, &_height, &_channels, STBI_rgb_alpha);
		if (!pixel_data)
		{
			TRC_ERROR("Unable to load texture {}: Error=> {}", name.c_str(), stbi_failure_reason());
			stbi__err(0, 0);
			return false;
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
				tex.m_id = i;
				TextureHash _hash;
				_hash._id = i;
				m_hashTable.Set(name, _hash);
				return true;
			}
			i++;
		}


		return true;
	}

	bool TextureManager::LoadTexture(const std::vector<std::string>& filenames, TextureDesc desc,const std::string& name)
	{
		if (m_hashTable.Get(name)._id != INVALID_ID)
		{
			TRC_WARN("Texture has already being loaded {}", name);
			return true;
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
				return false;
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
				tex.m_id = i;
				TextureHash _hash;
				_hash._id = i;
				m_hashTable.Set(name, _hash);
				return true;
			}
			i++;
		}

		return true;
	}

	void TextureManager::UnloadTexture(GTexture* texture)
	{
		if (texture->m_refCount > 0)
		{
			TRC_WARN("Can't release a texture that is in use");
			return;
		}

		//TODO: maybe the texture should be freed immediatly it is loaded to the GPU
		for(uint32_t i = 0; i < texture->GetTextureDescription().m_numLayers; i++)
			stbi_image_free(texture->GetTextureDescription().m_data[i]);

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
						pixel[_idx + 0] = 0;
						pixel[_idx + 1] = 0;
					}
				}
				else
				{
					if (!(coloumn % 2))
					{
						pixel[_idx + 0] = 0;
						pixel[_idx + 1] = 0;
					}
				}
			}
		}

		TextureDesc texture_desc;
		texture_desc.m_addressModeU = texture_desc.m_addressModeW = texture_desc.m_addressModeV = AddressMode::REPEAT;
		texture_desc.m_channels = 4;
		texture_desc.m_format = Format::R8G8B8A8_SRBG;
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
		default_diffuse_map = { &default_diffuse_texture, BIND_RESOURCE_UNLOAD_FN(TextureManager::UnloadDefaults, this) };


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
		default_specular_map = { &default_specular_texture, BIND_RESOURCE_UNLOAD_FN(TextureManager::UnloadDefaults, this) };

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
		default_normal_map = { &default_normal_texture, BIND_RESOURCE_UNLOAD_FN(TextureManager::UnloadDefaults, this) };

		
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



		//TODO: maybe the texture should be freed immediatly it is loaded to the GPU
		delete[] texture->GetTextureDescription().m_data[0];
		RenderFunc::DestroyTexture(texture);
		texture->~GTexture();
	}

}
