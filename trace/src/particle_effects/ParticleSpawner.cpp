#include "pch.h"

#include "particle_effects/ParticleSpawner.h"
#include "particle_effects/ParticleGenerator.h"
#include "particle_effects/ParticleEffect.h"
#include "scene/Entity.h"
#include "scene/Scene.h"


namespace trace {



	ParticleSpawner::~ParticleSpawner()
	{
		delete m_emissionVolume;//TODO: Use custom allocator
	}

	void RateSpawner::Run(ParticleGeneratorInstance* gen_instance, float deltaTime)
	{

		m_accumulator += deltaTime;
		float rate_inv = 1.0f / m_rate;
		uint32_t num_particles = 0;

		if (m_accumulator >= rate_inv)
		{
			num_particles = uint32_t(m_accumulator / rate_inv);
			m_accumulator = 0.0f;
		}
		else
		{
			return;
		}

		ParticleEffectInstance* effect = gen_instance->GetEffectInstance();

		if (!effect)
		{
			return;
		}

		Scene* scene = effect->GetScene();
		if (!scene)
		{
			return;
		}
		UUID owner_id = effect->GetOwnerID();		
		glm::vec3 entity_pos(0.0f);

		// NOTE: Enable if particle should be spawned in world_position
		//entity_pos = scene->GetEntityWorldPosition(scene->GetEntity(owner_id));

		for (uint32_t i = 0; i < num_particles; i++)
		{
			int32_t index = gen_instance->emit_particle();

			if (index >= 0)
			{
				ParticleData& particle_data = gen_instance->GetParticlesData();

				glm::vec4 position(m_emissionVolume->GetRandomPoint(), 0.0f);

				position += glm::vec4(entity_pos, 0.0f);
				position.w = 0.75f;
				particle_data.positions[index] = position;
				particle_data.scale[index] = glm::vec4(1.0f);
				particle_data.color[index] = glm::vec4(1.0f);
				particle_data.rotation[index] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

				gen_instance->initialize_particle(index);
			}
			
		}
	}

}