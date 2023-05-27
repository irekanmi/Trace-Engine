#pragma once

#include "core/Core.h"
#include "HashTable.h"
#include "render/GTexture.h"
#include <string>
#include "Ref.h"

using Texture_Ref = Ref<trace::GTexture>;

namespace trace {

	//TODO: Add more parameter for more info
	struct TextureHash
	{
		uint32_t _id = INVALID_ID;
	};

	class TRACE_API TextureManager
	{

	public:
		TextureManager();
		~TextureManager();

		bool Init(uint32_t maxTextureUnits);
		void ShutDown();

		GTexture* GetTexture(const std::string& name);
		Texture_Ref GetDefault(const std::string& name);
		bool LoadTexture(const std::string& name);
		bool LoadTexture(const std::string& name, TextureDesc desc);
		bool LoadTexture(const std::vector<std::string>& filenames, TextureDesc desc, const std::string& name);
		void UnloadTexture(GTexture* texture);
		void ReleaseTexture(const std::string& name);

		bool LoadDefaultTextures();

		static TextureManager* get_instance();

	private:
		void UnloadDefaults(GTexture* texture);

		static TextureManager* s_instance;

	private:
		HashTable<TextureHash> m_hashTable;
		std::vector<GTexture> m_textures;
		uint32_t m_textureUnits;
		uint32_t m_textureTypeSize;
		Texture_Ref default_diffuse_map;
		Texture_Ref default_specular_map;
		Texture_Ref default_normal_map;
		GTexture default_diffuse_texture;
		GTexture default_specular_texture;
		GTexture default_normal_texture;

	protected:

	};

}
