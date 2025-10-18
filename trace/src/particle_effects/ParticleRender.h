#pragma once

#include "reflection/TypeRegistry.h"
#include "particle_effects/ParticleData.h"

#include "glm/glm.hpp"

namespace trace {
	
	class ParticleGeneratorInstance;
	class Camera;

	class ParticleRender : public ParticleBase
	{

	public:

		virtual void Render(ParticleGeneratorInstance* gen_instance, Camera* camera, glm::mat4 transform, int32_t render_graph_index = 0) = 0;

	private:
	protected:
		GET_TYPE_ID;

	};
	



}
