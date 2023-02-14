#pragma once

#include "core/io/Logging.h"
#include "Ref.h"
#include "TextureManager.h"

using Texture_Ref = Ref<trace::GTexture>;

namespace trace {


	class ResourceSystem
	{

	public:
		ResourceSystem();
		~ResourceSystem();

		bool Init();
		void ShutDown();

		Texture_Ref GetTexture(const std::string& name);
		void ReleaseTexture(const std::string& name);

		static ResourceSystem* s_instance;
		static ResourceSystem* get_instance();

	private:
		TextureManager m_textureManager;

	protected:

	};

}

