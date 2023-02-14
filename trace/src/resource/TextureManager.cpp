#include "pch.h"
#include "TextureManager.h"
#include "render/Renderer.h"
#include "core/platform/Vulkan/VulkanTexture.h"
#include <new>
#include "core/Platform.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace trace {





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

		switch (Renderer::s_api)
		{
		case RenderAPI::Vulkan:
		{
			m_textureTypeSize = sizeof(VulkanTexture);

			//TODO: Use a custom allocator for allocation
			m_textures = new char[m_textureTypeSize * maxTextureUnits];

			VulkanTexture* textures = (VulkanTexture*)m_textures;

			for (uint32_t i = 0; i < m_textureUnits; i++)
			{
				textures[i].m_id = INVAILD_ID;
			}

			break;
		}
		default:
		{
			TRC_ASSERT(false, "Render API Texture not suppoted");
			return false;
			break;
		}
		}

			

		TextureHash _hash;
		_hash._id = INVAILD_ID;
		m_hashTable.Fill(_hash);
		return true;
	}

	void TextureManager::ShutDown()
	{
		if (m_textures)
		{
			//TODO: Use a custom allocator for allocation
			delete[] m_textures;

		}

	}

	GTexture* TextureManager::GetTexture(const std::string& name)
	{

		TextureHash& _hash = m_hashTable.Get_Ref(name);

		if (_hash._id != INVAILD_ID)
		{
			GTexture* value = (GTexture*)(m_textures + (m_textureTypeSize * _hash._id));
			return value;
		}
		else
		{

			if (LoadTexture(name))
			{
				GTexture* value = (GTexture*)(m_textures + (m_textureTypeSize * _hash._id));
				return value;
			}

		}

		//TODO: replace with the default texture
		TRC_ERROR("Please enter a vaild texture");
		return nullptr;
	}

	bool TextureManager::LoadTexture(const std::string& name)
	{
		int _width, _height, _channels;
		unsigned char* pixel_data = nullptr;

		TRC_TRACE("Texture load begin %s", name.c_str());
		stbi_set_flip_vertically_on_load(true);
		pixel_data = stbi_load(("../assets/textures/" + name).c_str(), &_width, &_height, &_channels, STBI_rgb_alpha);
		TRC_TRACE("Texture loaded %s", name.c_str());


		TextureDesc texture_desc;
		texture_desc.m_addressModeU = texture_desc.m_addressModeW = texture_desc.m_addressModeV = AddressMode::REPEAT;
		texture_desc.m_channels = 4;
		//texture_desc.m_data = pixel;
		texture_desc.m_format = Format::R8G8B8A8_SRBG;
		texture_desc.m_minFilterMode = texture_desc.m_magFilterMode = FilterMode::LINEAR;

		switch (Renderer::s_api)
		{
		case RenderAPI::Vulkan:
		{

			//if (stbi_failure_reason())
			//{
			//	TRC_ERROR("Unable to load texture %s: Error=> %s", name.c_str(), stbi_failure_reason());
			//	stbi__err(0, 0);
			//	return false;
			//}

			texture_desc.m_width = (uint32_t)_width;
			texture_desc.m_height = (uint32_t)_height;
			texture_desc.m_data = pixel_data;

			VulkanTexture* textures = (VulkanTexture*)m_textures;

			for (uint32_t i = 0; i < m_textureUnits; i++)
			{
				if (textures[i].m_id == INVAILD_ID)
				{
					VulkanTexture* texture = textures + i;
					new(texture) VulkanTexture(texture_desc);
					TRC_TRACE("Texture constructed %s", name.c_str());
					texture->m_id = i;
					TextureHash _hash;
					_hash._id = i;
					m_hashTable.Set(name, _hash);
					return true;
					break;
				}
			}

			TRC_WARN("Failed to find a vaild slot for the texture %s", name.c_str());
			return false;

			break;
		}
		default:
		{
			TRC_ASSERT(false, "Render API Texture not suppoted");
			return false;
			break;
		}
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

		GTexture* value = (GTexture*)(m_textures + (m_textureTypeSize * texture->m_id));
		value->m_id = INVAILD_ID;

		//TODO: maybe the texture should be freed immediatly it is loaded to the GPU
		stbi_image_free(texture->GetTextureDescription().m_data);
		texture->~GTexture();
	}

	void TextureManager::ReleaseTexture(const std::string& name)
	{
		TextureHash& _hash = m_hashTable.Get_Ref(name);

		if (_hash._id != INVAILD_ID)
		{
			GTexture* value = (GTexture*)(m_textures + (m_textureTypeSize * _hash._id));
			UnloadTexture(value);
			return;
		}

		TRC_WARN("Can't release a texture that hasn't been loaded please ensure texture is loaded");
		return;
	}

}
