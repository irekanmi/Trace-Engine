#include "pch.h"

#include "particle_effects/ParticleSpawner.h"
#include "particle_effects/ParticleGenerator.h"


namespace trace {



	ParticleSpawner::~ParticleSpawner()
	{
		delete m_emissionVolume;//TODO: Use custom allocator
	}

	void RateSpawner::Run(ParticleGeneratorInstance* gen_instance, float deltaTime)
	{
		uint32_t num_particles = uint32_t(m_rate * deltaTime);

		for (uint32_t i = 0; i < num_particles; i++)
		{
			int32_t index = gen_instance->emit_particle();

			if (index >= 0)
			{
				ParticleData& particle_data = gen_instance->GetParticlesData();

				glm::vec4 position(m_emissionVolume->GetRandomPoint(), 0.0f);
				particle_data.positions[index] = position;
				particle_data.scale[index] = glm::vec4(1.0f);
				particle_data.color[index] = glm::vec4(1.0f);
				particle_data.lifetime[index] = 0.75f;

				gen_instance->initialize_particle(index);
			}
			
		}
	}

}