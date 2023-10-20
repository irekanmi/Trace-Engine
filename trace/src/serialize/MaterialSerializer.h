#pragma once

#include "resource/Ref.h"
#include "render/Material.h"
#include <string>


namespace trace {

	class MaterialSerializer
	{

	public:

		static bool Serialize(Ref<MaterialInstance> material, const std::string& file_path);
		static Ref<MaterialInstance> Deserialize(const std::string& file_path);

	private:

	protected:

	};
}
