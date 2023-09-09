#pragma once

#include "core/Core.h"
#include "core/Enums.h"
#include <string>

namespace trace {

	class Font;

	// Font Loader Initialiasation
	typedef bool (*__LoadAndInitializeFont)(const std::string&, Font*);
	typedef bool (*__DestroyFont)(Font*);

	class FontFuncLoader
	{
	public:
		static 	bool Load_MSDF_Func();
	private:
	protected:
	};

	class FontFunc
	{
	public:

		static bool LoadAndInitializeFont(const std::string& name, Font* font);
		static bool DestroyFont(Font* font);

	private:
		static __LoadAndInitializeFont _loadAndInitializeFont;
		static __DestroyFont _destroyFont;

		friend FontFuncLoader;
	protected:
	};


}
