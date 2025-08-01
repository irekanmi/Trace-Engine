#pragma once

#include "particle_effects/ParticleData.h"
#include "reflection/TypeRegistry.h"

namespace trace {

	class ParticleGeneratorInstance;

	class ParticleInitializer
	{

	public:

		virtual void Init(ParticleGeneratorInstance* particle_generator) = 0;
		virtual void InitParticle(ParticleGeneratorInstance* particle_generator, uint32_t particle_index) = 0;

	private:
	protected:
		GET_TYPE_ID;

	};

}
