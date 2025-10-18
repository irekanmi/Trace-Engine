#pragma once

#include "scene/UUID.h"
#include "reflection/TypeRegistry.h"

#include "glm/glm.hpp"
#include <vector>
#include <unordered_map>

namespace trace {


	struct ParticleData
	{
		std::vector<glm::vec4> positions;
		std::vector<glm::vec4> color;
		std::vector<glm::vec4> velocities;
		std::vector<glm::vec4> scale;
		std::vector<float> lifetime;
		std::unordered_map<UUID, std::vector<glm::vec4>> custom_data;
	};

	class ParticleBase
	{
	public:
	private:
	protected:

		GET_TYPE_ID;
	};

}
