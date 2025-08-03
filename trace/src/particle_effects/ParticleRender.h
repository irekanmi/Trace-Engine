#pragma once

#include "reflection/TypeRegistry.h"

#include "glm/glm.hpp"

namespace trace {
	
	class ParticleGeneratorInstance;
	class Camera;

	class ParticleRender
	{

	public:

		virtual void Render(ParticleGeneratorInstance* gen_instance, Camera* camera, glm::mat4 transform) = 0;

	private:
	protected:
		GET_TYPE_ID;

	};
	



}
