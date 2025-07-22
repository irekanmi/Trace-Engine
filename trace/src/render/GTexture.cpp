#include "pch.h"

#include "GTexture.h"
#include "external_utils.h"
#include "core/Coretypes.h"
#include "resource/GenericAssetManager.h"
#include "backends/Renderutils.h"
#include "backends/UIutils.h"

#include <filesystem>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace trace {

	bool GTexture::Create(const std::string& path)
	{
		std::filesystem::path p(path);
		std::string name = p.filename().string();
		int _width, _height, _channels;
		unsigned char* pixel_data = nullptr;

		stbi_set_flip_vertically_on_load(true);
		pixel_data = stbi_load(p.string().c_str(), &_width, &_height, &_channels, STBI_rgb_alpha);
		if (!pixel_data)
		{
			TRC_ERROR("Unable to load texture {}: Error=> {}", name.c_str(), stbi_failure_reason());
			stbi__err(0, 0);
			return false;
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

		RenderFunc::CreateTexture(this, texture_desc);
		stbi_image_free(pixel_data);

		UIFunc::CreateTextureHandle(this);

		return true;
	}

	bool GTexture::Create(const std::string& path, TextureDesc desc)
	{
		std::filesystem::path p(path);
		std::string name = p.filename().string();

		int _width, _height, _channels;
		unsigned char* pixel_data = nullptr;

		stbi_set_flip_vertically_on_load(true);
		pixel_data = stbi_load(p.string().c_str(), &_width, &_height, &_channels, STBI_rgb_alpha);
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


		RenderFunc::CreateTexture(this, desc);
		stbi_image_free(pixel_data);

		UIFunc::CreateTextureHandle(this);

		return true;
	}

	bool GTexture::Create(TextureDesc desc)
	{
		TRC_ASSERT(desc.m_width != 0 || desc.m_height != 0, "width or height of a texture can not zero(0)");

		RenderFunc::CreateTexture(this, desc);
		delete[] desc.m_data[0];// TODO: Use custom allocator

		UIFunc::CreateTextureHandle(this);

		return true;
	}

	void GTexture::Destroy()
	{
		UIFunc::DestroyTextureHandle(this);
		RenderFunc::DestroyTexture(this);
	}

	TextureDesc& GTexture::GetTextureDescription()
	{
		return m_desc;
	}

	Ref<GTexture> GTexture::Deserialize(UUID id)
	{
		Ref<GTexture> result;

		if (AppSettings::is_editor)
		{
			result = LoadTexture(id);
		}
		else
		{
			result = GenericAssetManager::get_instance()->Load_Runtime<GTexture>(id);
		}

		return result;
	}

	Ref<GTexture> GTexture::Deserialize(DataStream* stream)
	{
		std::string name;
		stream->Read(name);
		TextureDesc desc;
		stream->Read<uint32_t>(desc.m_width);
		stream->Read<uint32_t>(desc.m_height);
		stream->Read<uint32_t>(desc.m_mipLevels);


		stream->Read<Format>(desc.m_format);

		stream->Read<BindFlag>(desc.m_flag);

		stream->Read<UsageFlag>(desc.m_usage);

		stream->Read<uint32_t>(desc.m_channels);
		stream->Read<uint32_t>(desc.m_numLayers);

		stream->Read<ImageType>(desc.m_image_type);

		stream->Read<AddressMode>(desc.m_addressModeU);
		stream->Read<AddressMode>(desc.m_addressModeV);
		stream->Read<AddressMode>(desc.m_addressModeW);

		stream->Read<FilterMode>(desc.m_minFilterMode);
		stream->Read<FilterMode>(desc.m_magFilterMode);

		stream->Read<AttachmentType>(desc.m_attachmentType);
		int tex_size = desc.m_width * desc.m_height * getFmtSize(desc.m_format);
		unsigned char* data = new unsigned char[tex_size];// TODO: Use custom allocator
		stream->Read(data, tex_size);
		std::vector<unsigned char*> _d;
		_d.push_back(data);
		desc.m_data = std::move(_d);

		Ref<GTexture> result = GenericAssetManager::get_instance()->CreateAssetHandle<GTexture>(name, desc);

		return result;
	}


}