#pragma once

#include "resource/Ref.h"
#include "project/Project.h"
#include <string>


namespace trace {

	class ProjectSerializer
	{

	public:

		static bool Serialize(Ref<Project> project, const std::string& file_path);
		static Ref<Project> Deserialize(const std::string& file_path);

	private:

	protected:

	};

	

}
