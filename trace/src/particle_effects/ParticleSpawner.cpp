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
		uint32_t num_particles = uint32_t(m_rate * deltaTime);
		ParticleEffectInstance* effect = gen_instance->GetEffectInstance();
		Scene* scene = effect->GetScene();
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
				particle_data.positions[index] = position;
				particle_data.scale[index] = glm::vec4(1.0f);
				particle_data.color[index] = glm::vec4(1.0f);
				particle_data.lifetime[index] = 0.75f;

				gen_instance->initialize_particle(index);
			}
			
		}
	}

}