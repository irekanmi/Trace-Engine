#pragma once

#include "scene/UUID.h"
#include "reflection/TypeRegistry.h"

#include "glm/glm.hpp"
#include <vector>
#include <unordered_map>

namespace trace {


	struct ParticleData
	{
		std::vector<glm::vec4> positions;//NOTE: Particle lifetime is stored in the w component of the vector
		std::vector<glm::vec4> color;
		std::vector<glm::vec4> velocities;
		std::vector<glm::vec4> scale;
		std::vector<glm::vec4> rotation;
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
