#pragma once

#include "reflection/TypeRegistry.h"

namespace trace {

	class ParticleGeneratorInstance;

	class ParticleSpawner
	{

	public:

		virtual void Run(ParticleGeneratorInstance* gen_instance) = 0;

	private:
	protected:
		GET_TYPE_ID;

	};

}
