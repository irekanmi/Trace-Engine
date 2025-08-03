#include "pch.h"

#include "particle_effects/ParticleGenerator.h"
#include "particle_effects/ParticleEffect.h"
#include "core/io/Logging.h"
#include "serialize/GenericSerializer.h"
#include "resource/GenericAssetManager.h"
#include "render/Camera.h"

namespace trace {



	bool ParticleGenerator::Create()
	{
		return true;
	}

	void ParticleGenerator::Destroy()
	{
		//TODO: Use Custom Allocator
		delete m_spawner;
		m_spawner = nullptr;
		for (auto& i : m_initializers)
		{
			delete i;
			i = nullptr;
		}

		for (auto& i : m_updates)
		{
			delete i;
			i = nullptr;
		}
		
		for (auto& i : m_renderers)
		{
			delete i;
			i = nullptr;
		}

	}

	Ref<ParticleGenerator> ParticleGenerator::Deserialize(UUID id)
	{
		Ref<ParticleGenerator> result;

		if (AppSettings::is_editor)
		{
			std::string file_path = GetPathFromUUID(id).string();
			if (!file_path.empty())
			{
				result = GenericSerializer::Deserialize<ParticleGenerator>(file_path);
			}
		}
		else
		{
			return GenericAssetManager::get_instance()->Load_Runtime<ParticleGenerator>(id);
		}

		return result;
	}

	Ref<ParticleGenerator> ParticleGenerator::Deserialize(DataStream* stream)
	{
		return GenericSerializer::Deserialize<ParticleGenerator>(stream);
	}

	bool ParticleGeneratorInstance::CreateInstance(Ref<ParticleGenerator> generator)
	{
		if (!generator)
		{
			TRC_ERROR("Invalid Particle Generator Handle, Function: {}", __FUNCTION__);
			return false;
		}

		m_gen = generator;

		

		return true;
	}

	void ParticleGeneratorInstance::Start(ParticleEffectInstance* effect_instance)
	{
		for (ParticleInitializer*& init : m_gen->GetInitializers())
		{
			init->Init(this);
		}

		m_particleData.positions.resize(m_gen->GetCapacity());
		m_particleData.color.resize(m_gen->GetCapacity());
		m_particleData.velocities.resize(m_gen->GetCapacity());
		m_particleData.scale.resize(m_gen->GetCapacity());
		m_particleData.lifetime.resize(m_gen->GetCapacity());

		for (auto& i : m_particleData.custom_data)
		{
			i.second.resize(m_gen->GetCapacity());
		}

		m_elaspedTime = 0.0f;
	}

	void ParticleGeneratorInstance::Update(ParticleEffectInstance* effect_instance, float deltaTime)
	{

		for (uint32_t index = 0; index < m_numAlive; index++)
		{
			float& life_time = m_particleData.lifetime[index];
			if (life_time <= 0.0f)
			{
				kill_particle(index);
			}
			else
			{
				life_time -= deltaTime;
			}
		}

		m_gen->GetSpawner()->Run(this, deltaTime);

		for (ParticleUpdate*& update : m_gen->GetUpdates())
		{
			for (uint32_t index = 0; index < m_numAlive; index++)
			{
				update->UpdateParticle(this, index, deltaTime);
			}
		}

	}

	void ParticleGeneratorInstance::Stop(ParticleEffectInstance* effect_instance)
	{
		m_numAlive = 0;
		m_elaspedTime = 0.0f;
	}

	void ParticleGeneratorInstance::Render(ParticleEffectInstance* effect_instance, Camera* camera, glm::mat4 transform)
	{
		for (ParticleRender*& render : m_gen->GetRenderers())
		{
			render->Render(this, camera, transform);
		}
	}

	int32_t ParticleGeneratorInstance::emit_particle()
	{
		if (m_numAlive < m_gen->GetCapacity())
		{
			uint32_t index = m_numAlive++;

			return index;
		}

		return -1;
	}

	void ParticleGeneratorInstance::initialize_particle(uint32_t index)
	{
		for (ParticleInitializer*& init : m_gen->GetInitializers())
		{
			init->InitParticle(this, index);
		}
	}
	
	void ParticleGeneratorInstance::kill_particle(uint32_t index)
	{
		uint32_t dst_index = m_numAlive - 1;

		std::swap(m_particleData.positions[index], m_particleData.positions[dst_index]);
		std::swap(m_particleData.color[index], m_particleData.color[dst_index]);
		std::swap(m_particleData.velocities[index], m_particleData.velocities[dst_index]);
		std::swap(m_particleData.lifetime[index], m_particleData.lifetime[dst_index]);

		for (auto& i : m_particleData.custom_data)
		{
			std::swap(i.second[index], i.second[dst_index]);
		}

		--m_numAlive;
	}

}