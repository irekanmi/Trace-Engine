#pragma once


#include "resource/Ref.h"
#include "scene/UUID.h"
#include "particle_effects/ParticleGenerator.h"
#include "serialize/DataStream.h"
#include "reflection/TypeRegistry.h"

#include <vector>

namespace trace {

	class Scene;

	class ParticleEffect : public Resource
	{

	public:

		bool Create();
		virtual void Destroy() override;

		float GetLifeTime() { return m_lifeTime; }
		void SetLifeTime(float life_time) { m_lifeTime = life_time; }

		std::vector<Ref<ParticleGenerator>>& GetGenerators() { return m_generators; }
		void SetGenerators(std::vector<Ref<ParticleGenerator>>& generators) { m_generators = generators; }

		static Ref<ParticleEffect> Deserialize(UUID id);
		static Ref<ParticleEffect> Deserialize(DataStream* stream);

	private:
		float m_lifeTime = 0.0f;
		std::vector<Ref<ParticleGenerator>> m_generators;

	protected:
		ACCESS_CLASS_MEMBERS(ParticleEffect);

	};

	
	class ParticleEffectInstance
	{

	public:

		bool CreateInstance(Ref<ParticleEffect> particle_effect, UUID entityID, Scene* scene);

		Ref<ParticleEffect> GetParticleEffect() { return m_particleEffect; }
		void SetParticleEffect(Ref<ParticleEffect> particle_effect) { m_particleEffect = particle_effect; }

		void Start();
		void Update(float deltaTime);
		void Stop();

	private:
		float m_elaspedTime = 0.0f;
		Ref<ParticleEffect> m_particleEffect;
		UUID m_ownerID = 0;
		Scene* m_scene = nullptr;
		std::vector<ParticleGeneratorInstance> m_generatorInstance;
		bool m_running = false;

	protected:
		ACCESS_CLASS_MEMBERS(ParticleEffectInstance);

	};


}
