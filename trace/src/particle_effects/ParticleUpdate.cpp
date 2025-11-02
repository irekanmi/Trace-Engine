#include "pch.h"

#include "particle_effects/ParticleUpdate.h"
#include "particle_effects/ParticleGenerator.h"
#include "core/Utils.h"


namespace trace {



	void GravityUpdate::UpdateParticle(ParticleGeneratorInstance* particle_generator, uint32_t particle_index, float deltaTime)
	{
		ParticleData& particle_data = particle_generator->GetParticlesData();

		glm::vec4 velocity = particle_data.velocities[particle_index];

		velocity += glm::vec4(m_gravity, 0.0f) * deltaTime;

		particle_data.velocities[particle_index] = velocity;

	}

	void DragUpdate::UpdateParticle(ParticleGeneratorInstance* particle_generator, uint32_t particle_index, float deltaTime)
	{
		ParticleData& particle_data = particle_generator->GetParticlesData();

		glm::vec4 velocity = particle_data.velocities[particle_index];

		velocity *= (1.0f - m_dragCoefficient * deltaTime);

		particle_data.velocities[particle_index] = velocity;
	}

	void WindUpdate::UpdateParticle(ParticleGeneratorInstance* particle_generator, uint32_t particle_index, float deltaTime)
	{


		ParticleData& particle_data = particle_generator->GetParticlesData();

		glm::vec4 velocity = particle_data.velocities[particle_index];


		// Add base wind force
		velocity += glm::vec4(m_windForce, 0.0f) * deltaTime;

		// Add some turbulence
		if (m_turbulence > 0.0f) {
			glm::vec4 randomForce(
				(RandomFloat() - 0.5f) * m_turbulence,
				(RandomFloat() - 0.5f) * m_turbulence,
				(RandomFloat() - 0.5f) * m_turbulence,
				0.0f
			);
			velocity += randomForce * deltaTime;
		}

		particle_data.velocities[particle_index] = velocity;
	}

	void VelocityUpdate::UpdateParticle(ParticleGeneratorInstance* particle_generator, uint32_t particle_index, float deltaTime)
	{
		ParticleData& particle_data = particle_generator->GetParticlesData();

		glm::vec4 velocity = particle_data.velocities[particle_index];
		glm::vec4 position = particle_data.positions[particle_index];

		position.x += velocity.x * deltaTime;
		position.y += velocity.y * deltaTime;
		position.z += velocity.z * deltaTime;

		particle_data.positions[particle_index] = position;
	}

}

