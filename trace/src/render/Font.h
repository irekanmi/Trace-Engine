#pragma once

#include "core/Core.h"
#include "core/Enums.h"

#include "resource/Ref.h"
#include "resource/Resource.h"

namespace trace {

	class GTexture;

	class TRACE_API Font : public Resource
	{

	public:

		Ref<GTexture> GetAtlas();
		void* GetInternal();
		std::string& GetFontName();
		void SetAtlas(Ref<GTexture> atlas);
		void SetInternal(void* value);
		void SetFontName(const std::string& name);


	private:
		Ref<GTexture> m_atlas;
		std::string m_fontName;
		void* m_internal = nullptr;

	protected:

	};

}
