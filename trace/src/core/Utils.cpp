#include "pch.h"

#include "Utils.h"
#include "core/Coretypes.h"
#include "debug/Debugger.h"


#include "glm/gtx/matrix_decompose.hpp"
#include <filesystem>

namespace trace {



	bool FindDirectory(const std::string& from, const std::string& dir, std::string& result)
	{
		std::filesystem::path current_dir = std::filesystem::path(from);


		while (current_dir != "")
		{
			if (std::filesystem::exists(current_dir / dir))
			{
				result = (current_dir / dir).generic_string();
				return true;
			}

			current_dir = current_dir.parent_path();
		}

		return false;
	}

	void DecomposeMatrix(glm::mat4 matrix, glm::vec3& translation, glm::vec3& rotation, glm::vec3& scale)
	{

		using namespace glm;
		using T = float;

		mat4 LocalMatrix(matrix);

		// Normalize the matrix.
		if (epsilonEqual(LocalMatrix[3][3], static_cast<T>(0), epsilon<T>()))
			return;

		for (length_t i = 0; i < 4; ++i)
			for (length_t j = 0; j < 4; ++j)
				LocalMatrix[i][j] /= LocalMatrix[3][3];

		// First, isolate perspective.  This is the messiest.
		if (
			epsilonNotEqual(LocalMatrix[0][3], static_cast<T>(0), epsilon<T>()) ||
			epsilonNotEqual(LocalMatrix[1][3], static_cast<T>(0), epsilon<T>()) ||
			epsilonNotEqual(LocalMatrix[2][3], static_cast<T>(0), epsilon<T>()))
		{
			// Clear the perspective partition
			LocalMatrix[0][3] = LocalMatrix[1][3] = LocalMatrix[2][3] = static_cast<T>(0);
			LocalMatrix[3][3] = static_cast<T>(1);
		}

		// Next take care of translation (easy).
		translation = vec3(LocalMatrix[3]);
		LocalMatrix[3] = vec4(0, 0, 0, LocalMatrix[3].w);

		vec3 Row[3];// , Pdum3;

		// Now get scale and shear.
		for (length_t i = 0; i < 3; ++i)
			for (length_t j = 0; j < 3; ++j)
				Row[i][j] = LocalMatrix[i][j];

		// Compute X scale factor and normalize first row.
		scale.x = length(Row[0]);// v3Length(Row[0]);
		Row[0] = detail::scale(Row[0], static_cast<T>(1));
		// Now, compute Y scale and normalize 2nd row.
		scale.y = length(Row[1]);
		Row[1] = detail::scale(Row[1], static_cast<T>(1));
		scale.z = length(Row[2]);
		Row[2] = detail::scale(Row[2], static_cast<T>(1));

		 rotation.y = asin(-Row[0][2]);
		 if (cos(rotation.y) != 0) {
		     rotation.x = atan2(Row[1][2], Row[2][2]);
		     rotation.z = atan2(Row[0][1], Row[0][0]);
		 } else {
		     rotation.x = atan2(-Row[2][0], Row[1][1]);
		     rotation.z = 0;
		 }


	}

	std::vector<std::string> SplitString(const std::string& str, char token)
	{
		std::stringstream ss(str);

		std::vector<std::string> data;
		std::string t;

		while (std::getline(ss, t, token))
		{
			if (t.empty()) continue;
			data.push_back(t);
		}

		return data;
	}

	glm::vec4 colorFromUint32(uint32_t color)
	{
		glm::vec4 result;

		float a = float(color >> 24) / 255.0f;
		float b = float((color >> 16) & 0x000000FF) / 255.0f;
		float g = float((color >> 8) & 0x000000FF) / 255.0f;
		float r = float((color >> 0) & 0x000000FF) / 255.0f;

		result.r = r;
		result.g = g;
		result.b = b;
		result.a = a;

		return result;
	}

	uint32_t colorVec4ToUint(glm::vec4 color)
	{
		uint32_t result = TRC_COL32_WHITE;

		uint32_t r = (uint32_t)(color.r * 255.0f);
		uint32_t g = (uint32_t)(color.g * 255.0f);
		uint32_t b = (uint32_t)(color.b * 255.0f);
		uint32_t a = (uint32_t)(color.a * 255.0f);

		result = TRC_COL32(r, g, b, a);
		return result;
	}

	StringID GetStringID(const std::string& data)
	{
		StringID id;
		id.value = hash_string(data);
		// NOTE: Only to be used for the editor
		Debugger::get_instance()->SetString(id, data);
		return id;
	}

	std::string& GetStringFromID(StringID string_id)
	{
		// NOTE: Only to be used for the editor
		 return Debugger::get_instance()->GetString(string_id);
	}

	uint64_t hash_string(const std::string& str)
	{
		std::uint64_t hash_value = 0xcbf29ce484222325ULL;
		constexpr std::uint64_t prime = 0x100000001b3ULL;
		for (char c : str)
		{
			hash_value ^= static_cast<std::uint64_t>(c);
			hash_value *= prime;
		}
		return hash_value;
	}

}