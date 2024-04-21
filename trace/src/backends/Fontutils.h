#pragma once

#include "core/Core.h"
#include "core/Enums.h"
#include <string>

#include "glm/glm.hpp"


namespace trace {

	class Font;
	struct TextVertex;

	// Font Loader Initialiasation
	typedef bool (*__LoadAndInitializeFont)(const std::string&, Font*);
	typedef bool (*__LoadAndInitializeFont_Data)(const std::string&,Font*, char*, uint32_t);
	typedef bool (*__DestroyFont)(Font*);

	// String Processing 
	typedef bool (*__ComputeTextString)(Font*, const std::string& text, std::vector<glm::vec4>&, uint32_t, std::vector<glm::vec4>&, glm::mat4&, float, uint32_t&);
	typedef bool (*__ComputeTextVertex)(Font*, const std::string& text, std::vector<TextVertex>&, glm::mat4&, glm::vec3&);

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
		static bool LoadAndInitializeFont(const std::string& name, Font* font, char* data, uint32_t size);
		static bool DestroyFont(Font* font);
		static bool ComputeTextString(Font* font, const std::string& text, std::vector<glm::vec4>& positions, uint32_t pos_index, std::vector<glm::vec4>& tex_coords, glm::mat4& _transform, float tex_index, uint32_t& count);
		static bool ComputeTextVertex(Font* font, const std::string& text, std::vector<TextVertex>& text_vertices, glm::mat4& _transform, glm::vec3& color);

	private:
		static __LoadAndInitializeFont _loadAndInitializeFont;
		static __LoadAndInitializeFont_Data _loadAndInitializeFont_Data;
		static __DestroyFont _destroyFont;
		static __ComputeTextString _computeTextString;
		static __ComputeTextVertex _computeTextVertex;

		friend FontFuncLoader;
	protected:
	};


}
