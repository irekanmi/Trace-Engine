#pragma once

#include "reflection/TypeRegistry.h"

namespace trace {

	class ParticleGeneratorInstance;

	class ParticleUpdate
	{

	public:
		virtual void UpdateParticle(ParticleGeneratorInstance* particle_generator, uint32_t particle_index) = 0;

	private:
	protected:
		GET_TYPE_ID;

	};

}
