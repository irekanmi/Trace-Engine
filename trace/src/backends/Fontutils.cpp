#include "pch.h"

#include "Fontutils.h"
#include "core/io/Logging.h"
#include "render/Font.h"
#include "render/Graphics.h"
#include "render/GTexture.h"

#define FONT_FUNC_IS_VALID(function)							 \
	if(!function)                                                \
	{                                                            \
		TRC_ERROR(                                               \
	"{} is not available, please check for any errors"           \
		, #function);                                            \
		return false;                                            \
	}

// MSDF -----------------------------------
bool __MSDF_LoadAndInitializeFont(const std::string & name, trace::Font * font);
bool __MSDF_DestroyFont(trace::Font * font);
// ----------------------------------------



namespace trace {

	bool FontFuncLoader::Load_MSDF_Func()
	{
		FontFunc::_loadAndInitializeFont = __MSDF_LoadAndInitializeFont;
		FontFunc::_destroyFont = __MSDF_DestroyFont;
		return true;
	}

	__LoadAndInitializeFont FontFunc::_loadAndInitializeFont = nullptr;
	__DestroyFont FontFunc::_destroyFont = nullptr;	

	bool FontFunc::LoadAndInitializeFont(const std::string& name, Font* font)
	{
		FONT_FUNC_IS_VALID(_loadAndInitializeFont);
		return _loadAndInitializeFont(name, font);
	}

	bool FontFunc::DestroyFont(Font* font)
	{
		FONT_FUNC_IS_VALID(_destroyFont);
		return _destroyFont(font);
	}

}



// MSDF-------------------------------------------------------------

#include "msdf-atlas-gen/msdf-atlas-gen.h"
#include "msdfgen.h"
#include "resource/ResourceSystem.h"

struct MSDF_Handle
{
	MSDF_Handle() : fontGeometry(&glyphs) {}
	std::vector<msdf_atlas::GlyphGeometry> glyphs;
	msdf_atlas::FontGeometry fontGeometry;
};

bool submitAtlasBitmapAndLayout(msdf_atlas::BitmapAtlasStorage<msdfgen::byte, 3> bit_map_storage, std::vector<msdf_atlas::GlyphGeometry> glyphs, const std::string& name, trace::Font* font)
{
	msdfgen::BitmapConstRef<msdfgen::byte, 3>& bitmap_ref = (msdfgen::BitmapConstRef<msdfgen::byte, 3>)bit_map_storage;

	//NOTE: Temporary fix my latop does not support trace::Format::R8G8B8_UNORM in vulkan and msdf generated is of three channels //////////////////////
	unsigned char* map_data = new unsigned char[bitmap_ref.width * bitmap_ref.height * 4];
	unsigned char* bitmap_data = (unsigned char*)bitmap_ref.pixels;
	memset(map_data, 0, bitmap_ref.width * bitmap_ref.height * 4);
	for (uint32_t row = 0; row < bitmap_ref.height; row++)
	{
		for (uint32_t coloumn = 0; coloumn < bitmap_ref.width; coloumn++)
		{
			uint32_t index = (row * bitmap_ref.height) + coloumn;
			uint32_t _idx = index * 4;
			uint32_t _idx2 = index * 3;
			map_data[_idx + 0] = bitmap_data[_idx2 + 0];
			map_data[_idx + 1] = bitmap_data[_idx2 + 1];
			map_data[_idx + 2] = bitmap_data[_idx2 + 2];
			map_data[_idx + 3] = 255;
		}
	}
	//////////////////////////////////////////////////////////

	trace::TextureDesc tex_desc = {};
	tex_desc.m_addressModeU = tex_desc.m_addressModeV = tex_desc.m_addressModeW = trace::AddressMode::REPEAT;
	tex_desc.m_attachmentType = trace::AttachmentType::COLOR;
	tex_desc.m_channels = 4;
	tex_desc.m_data.push_back(map_data);// TODO: Check if MSDF-ATLAS-GEN Free memory allocated for atlas data;
	tex_desc.m_flag = trace::BindFlag::SHADER_RESOURCE_BIT;
	tex_desc.m_format = trace::Format::R8G8B8A8_UNORM;
	tex_desc.m_height = bitmap_ref.height;
	tex_desc.m_width = bitmap_ref.width;
	tex_desc.m_image_type = trace::ImageType::IMAGE_2D;
	tex_desc.m_magFilterMode = tex_desc.m_minFilterMode = trace::FilterMode::LINEAR;
	tex_desc.m_mipLevels = 1;
	tex_desc.m_numLayers = 1;
	tex_desc.m_usage = trace::UsageFlag::DEFAULT;

	font->SetAtlas(trace::ResourceSystem::get_instance()->CreateTexture(name, tex_desc));
	TRC_ASSERT(font->GetAtlas().is_valid(), "Texture Creation Failed");

	bool result = msdfgen::saveBmp(bit_map_storage, "output2.bmp");
	return result;
}

bool __MSDF_LoadAndInitializeFont(const std::string& name, trace::Font* _font)
{
	if (_font->GetInternal())
	{
		TRC_WARN("{} has already been loaded or _font passed in is invalid, _font->{}  : {}", name, _font->GetInternal(), __FUNCTION__);
		return false;
	}

	if (name.empty())
	{
		TRC_ERROR("Can't process an empty string, {}", __FUNCTION__);
		return false;
	}

	MSDF_Handle* _internal = new MSDF_Handle();
	_font->SetInternal(_internal);

	bool success = false;
	if (msdfgen::FreetypeHandle* ft = msdfgen::initializeFreetype())
	{
		if (msdfgen::FontHandle* font = msdfgen::loadFont(ft, name.c_str()))
		{
			// Load a set of character glyphs:
			// The second argument can be ignored unless you mix different font sizes in one atlas.
			// In the last argument, you can specify a charset other than ASCII.
			// To load specific glyph indices, use loadGlyphs instead.
			int loaded = _internal->fontGeometry.loadCharset(font, 1.0, msdf_atlas::Charset::ASCII);
			TRC_INFO("Loaded {} glyphs from {}", loaded, name);
			// Apply MSDF edge coloring. See edge-coloring.h for other coloring strategies.
			const double maxCornerAngle = 3.0;
			for (msdf_atlas::GlyphGeometry& glyph : _internal->glyphs)
				glyph.edgeColoring(&msdfgen::edgeColoringInkTrap, maxCornerAngle, 0);
			// TightAtlasPacker class computes the layout of the atlas.
			msdf_atlas::TightAtlasPacker packer;
			// Set atlas parameters:
			// setDimensions or setDimensionsConstraint to find the best value
			packer.setDimensionsConstraint(msdf_atlas::TightAtlasPacker::DimensionsConstraint::POWER_OF_TWO_SQUARE);
			// setScale for a fixed size or setMinimumScale to use the largest that fits
			//packer.setMinimumScale(24.0);
			packer.setScale(64.0f);
			// setPixelRange or setUnitRange
			packer.setPixelRange(2.0);
			packer.setMiterLimit(1.0);
			// Compute atlas layout - pack glyphs
			packer.pack(_internal->glyphs.data(), _internal->glyphs.size());
			// Get final atlas dimensions
			int width = 0, height = 0;
			packer.getDimensions(width, height);
			// The ImmediateAtlasGenerator class facilitates the generation of the atlas bitmap.
			msdf_atlas::ImmediateAtlasGenerator<
				float, // pixel type of buffer for individual glyphs depends on generator function
				3, // number of atlas color channels
				&msdf_atlas::msdfGenerator, // function to generate bitmaps for individual glyphs
				msdf_atlas::BitmapAtlasStorage<byte, 3> // class that stores the atlas bitmap
				// For example, a custom atlas storage class that stores it in VRAM can be used.
			> generator(width, height);
			// GeneratorAttributes can be modified to change the generator's default settings.
			msdf_atlas::GeneratorAttributes attributes;
			attributes.scanlinePass = true;
			attributes.config.overlapSupport = true;

			generator.setAttributes(attributes);
			generator.setThreadCount(4);
			// Generate atlas bitmap
			generator.generate(_internal->glyphs.data(), _internal->glyphs.size());
			// The atlas bitmap can now be retrieved via atlasStorage as a BitmapConstRef.
			// The glyphs array (or fontGeometry) contains positioning data for typesetting text.
			success = submitAtlasBitmapAndLayout(generator.atlasStorage(), _internal->glyphs, _font->GetFontName(), _font);
			// Cleanup
			msdfgen::destroyFont(font);

		}
		msdfgen::deinitializeFreetype(ft);
	}


	return success;
}

bool __MSDF_DestroyFont(trace::Font* font)
{

	if (!font->GetInternal())
	{
		TRC_ERROR("Handle passed in is invalid : {}", __FUNCTION__);
		return false;
	}

	void* _internal = font->GetInternal();
	delete _internal;
	_internal = nullptr;
	font->SetInternal(nullptr);

	return true;
}

// -------------------------------------------------------------------------------- 
