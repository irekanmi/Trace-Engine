#pragma once

#include "core/Core.h"
#include "core/Enums.h"
#include <string>

#include "glm/glm.hpp"


namespace trace {

	class Font;

	// Font Loader Initialiasation
	typedef bool (*__LoadAndInitializeFont)(const std::string&, Font*);
	typedef bool (*__DestroyFont)(Font*);

	// String Processing 
	typedef bool (*__ComputeTextString)(Font*, const std::string& text, std::vector<glm::vec4>&, uint32_t, std::vector<glm::vec4>&, glm::mat4&, float, uint32_t&);

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
		static bool ComputeTextString(Font* font, const std::string& text, std::vector<glm::vec4>& positions, uint32_t pos_index, std::vector<glm::vec4>& tex_coords, glm::mat4& transform, float tex_index, uint32_t& count);

	private:
		static __LoadAndInitializeFont _loadAndInitializeFont;
		static __DestroyFont _destroyFont;
		static __ComputeTextString _computeTextString;

		friend FontFuncLoader;
	protected:
	};


}
