#include "pch.h"

#include "particle_effects/ParticleInitialize.h"
#include "particle_effects/ParticleGenerator.h"
#include "core/Utils.h"

#include "glm/glm.hpp"

namespace trace {



	void VelocityInitializer::Init(ParticleGeneratorInstance* particle_generator)
	{
	}

	void VelocityInitializer::InitParticle(ParticleGeneratorInstance* particle_generator, uint32_t particle_index)
	{
		ParticleData& particle_data = particle_generator->GetParticlesData();

		glm::vec4 velocity = particle_data.velocities[particle_index];

		velocity = glm::vec4(
			m_minVelocity.x + (m_maxVelocity.x - m_minVelocity.x) * RandomFloat(),
			m_minVelocity.y + (m_maxVelocity.y - m_minVelocity.y) * RandomFloat(),
			m_minVelocity.z + (m_maxVelocity.z - m_minVelocity.z) * RandomFloat(),
			00.0f
		);

		particle_data.velocities[particle_index] = velocity;
	}

	void LifetimeInitializer::Init(ParticleGeneratorInstance* particle_generator)
	{
	}

	void LifetimeInitializer::InitParticle(ParticleGeneratorInstance* particle_generator, uint32_t particle_index)
	{
		ParticleData& particle_data = particle_generator->GetParticlesData();

		float lifetime = particle_data.lifetime[particle_index];

		lifetime = m_min + (m_max - m_min) * RandomFloat();
		
		particle_data.lifetime[particle_index] = lifetime;
	}

}