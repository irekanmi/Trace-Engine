#pragma once

#include "resource/Ref.h"
#include "render/GPipeline.h"
#include <string>


namespace trace {

	class PipelineSerializer
	{

	public:

		static bool Serialize(Ref<GPipeline> pipeline, const std::string& file_path);
		static Ref<GPipeline> Deserialize(const std::string& file_path);

	private:

	protected:

	};
}
