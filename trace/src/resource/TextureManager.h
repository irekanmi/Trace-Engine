#pragma once

#include "core/Core.h"
#include "HashTable.h"
#include "render/GTexture.h"
#include "Ref.h"
#include "scene/UUID.h"
#include "serialize/AssetsInfo.h"


#include <string>
#include <filesystem>

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

		Ref<GTexture> GetTexture(const std::string& name);
		Texture_Ref GetDefault(const std::string& name);
		Ref<GTexture> CreateTexture(const std::string& name, TextureDesc desc);
		Ref<GTexture> LoadTexture(const std::string& name);
		Ref<GTexture> LoadTexture_(const std::string& path);
		Ref<GTexture> LoadTexture(const std::string& name, TextureDesc desc);
		Ref<GTexture> LoadTexture_(const std::string& path, TextureDesc desc);
		Ref<GTexture> LoadTexture(const std::vector<std::string>& filenames, TextureDesc desc, const std::string& name);
		void UnloadTexture(Resource* res);
		void ReleaseTexture(const std::string& name);
		void SetAssetMap(std::unordered_map<UUID, AssetHeader> map)
		{
			m_assetMap = map;
		}
		Ref<GTexture> LoadTexture_Runtime(UUID id);

		bool LoadDefaultTextures();

		static TextureManager* get_instance();

	private:
		void UnloadDefaults(GTexture* texture);


	private:
		HashTable<TextureHash> m_hashTable;
		//std::unordered_map<UUID, uint32_t> m_uuidMap; // NOTE: Used to hold resource runtime handle, which allow resources to be queried using there UUID
		std::vector<GTexture> m_textures;
		uint32_t m_textureUnits;
		uint32_t m_textureTypeSize;
		std::unordered_map<UUID, AssetHeader> m_assetMap;

		Texture_Ref default_diffuse_map;
		Texture_Ref default_specular_map;
		Texture_Ref default_normal_map;
		Texture_Ref black_texture;
		Texture_Ref transparent_texture;
		GTexture default_diffuse_texture;
		GTexture default_specular_texture;
		GTexture default_normal_texture;
		std::filesystem::path texture_resource_path;

	protected:

	};

}
