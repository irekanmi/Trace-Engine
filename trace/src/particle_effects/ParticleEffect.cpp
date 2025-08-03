#include "pch.h"

#include "particle_effects/ParticleEffect.h"
#include "core/io/Logging.h"
#include "serialize/GenericSerializer.h"
#include "resource/GenericAssetManager.h"
#include "render/Camera.h"
#include "scene/Entity.h"
#include "scene/Scene.h"

namespace trace {



	bool ParticleEffect::Create()
	{
		return true;
	}

	void ParticleEffect::Destroy()
	{
		for (auto& particle_gen : m_generators)
		{
			particle_gen.free();
		}
	}

	Ref<ParticleEffect> ParticleEffect::Deserialize(UUID id)
	{
		Ref<ParticleEffect> result;

		if (AppSettings::is_editor)
		{
			std::string file_path = GetPathFromUUID(id).string();
			if (!file_path.empty())
			{
				result = GenericSerializer::Deserialize<ParticleEffect>(file_path);
			}
		}
		else
		{
			return GenericAssetManager::get_instance()->Load_Runtime<ParticleEffect>(id);
		}

		return result;
	}

	Ref<ParticleEffect> ParticleEffect::Deserialize(DataStream* stream)
	{
		return GenericSerializer::Deserialize<ParticleEffect>(stream);
	}

	bool ParticleEffectInstance::CreateInstance(Ref<ParticleEffect> particle_effect, UUID entityID, Scene* scene)
	{
		if (!particle_effect)
		{
			TRC_ERROR("Invalid particle effect handle, EntityID: {}, Function: {}", entityID, __FUNCTION__);
			return false;
		}

		m_particleEffect = particle_effect;
		m_ownerID = entityID;
		m_scene = scene;

		//Initialize Generators
		for (Ref<ParticleGenerator>& generator : m_particleEffect->GetGenerators())
		{
			ParticleGeneratorInstance gen_instance;
			if (gen_instance.CreateInstance(generator))
			{
				m_generatorInstance.emplace_back(gen_instance);
			}
		}

		return true;
	}

	void ParticleEffectInstance::Start()
	{
		m_running = true;

		//Start Generators
		for (ParticleGeneratorInstance& gen_instance : m_generatorInstance)
		{
			gen_instance.Start(this);
		}
	}

	void ParticleEffectInstance::Update(float deltaTime)
	{
		if (!m_running)
		{
			return;
		}

		if (m_elaspedTime >= m_particleEffect->GetLifeTime())
		{
			Stop();
		}

		//Update Generators
		for (ParticleGeneratorInstance& gen_instance : m_generatorInstance)
		{
			gen_instance.Update(this, deltaTime);
		}



		m_elaspedTime += deltaTime;
	}

	void ParticleEffectInstance::Stop()
	{
		m_running = false;

		//Stop Generators
		for (ParticleGeneratorInstance& gen_instance : m_generatorInstance)
		{
			gen_instance.Stop(this);
		}
	}

	void ParticleEffectInstance::Render(Camera* camera)
	{
		//Render Generators
		glm::mat4 transform = m_scene->GetEntityWorldTransform(m_scene->GetEntity(m_ownerID)).GetLocalMatrix();
		for (ParticleGeneratorInstance& gen_instance : m_generatorInstance)
		{
			gen_instance.Render(this, camera, transform);
		}
	}

}