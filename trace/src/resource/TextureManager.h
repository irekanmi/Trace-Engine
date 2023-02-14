#pragma once

#include "core/Core.h"
#include "HashTable.h"
#include "render/GTexture.h"
#include <string>

namespace trace {

	//TODO: Add more parameter for more info
	struct TextureHash
	{
		uint32_t _id = INVAILD_ID;
	};

	class TRACE_API TextureManager
	{

	public:
		TextureManager();
		~TextureManager();

		bool Init(uint32_t maxTextureUnits);
		void ShutDown();

		GTexture* GetTexture(const std::string& name);
		bool LoadTexture(const std::string& name);
		void UnloadTexture(GTexture* texture);
		void ReleaseTexture(const std::string& name);

	private:
		HashTable<TextureHash> m_hashTable;
		char* m_textures = nullptr;
		uint32_t m_textureUnits;
		uint32_t m_textureTypeSize;

	protected:

	};

}
