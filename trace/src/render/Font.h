#pragma once

#include "core/Core.h"
#include "core/Enums.h"
#include "resource/Ref.h"
#include "resource/Resource.h"
#include "scene/UUID.h"
#include "serialize/DataStream.h"

namespace trace {

	class GTexture;

	class TRACE_API Font : public Resource
	{

	public:

		bool Create(const std::string& path);
		bool Create(const std::string& name, DataStream* stream);
		virtual void Destroy() override;

		virtual ~Font() {};

		Ref<GTexture> GetAtlas();
		void* GetInternal();
		std::string& GetFontName();
		void SetAtlas(Ref<GTexture> atlas);
		void SetInternal(void* value);
		void SetFontName(const std::string& name);

		static Ref<Font> Deserialize(UUID id);
		static Ref<Font> Deserialize(DataStream* stream);
	private:
		Ref<GTexture> m_atlas;
		std::string m_fontName;
		void* m_internal = nullptr;

	protected:

	};

}
