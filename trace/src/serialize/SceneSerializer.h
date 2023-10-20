#pragma once

#include "scene/Scene.h"
#include "resource/Ref.h"
#include <string>


namespace trace {

	class SceneSerializer
	{

	public:

		static bool Serialize(Ref<Scene> scene, const std::string& file_path);
		static Ref<Scene> Deserialize( const std::string& file_path);

	private:
		
	protected:

	};

}
