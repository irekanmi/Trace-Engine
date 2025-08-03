#pragma once

#include "reflection/TypeRegistry.h"
#include "particle_effects/EmissionVolume.h"

namespace trace {

	class ParticleGeneratorInstance;

	class ParticleSpawner
	{

	public:

		virtual ~ParticleSpawner();

		virtual void Run(ParticleGeneratorInstance* gen_instance, float deltaTime) {};
		EmissionVolume* GetEmissionVolume() { return m_emissionVolume; }
		void SetEmissionVolume(EmissionVolume* emission_volume) { m_emissionVolume = emission_volume; }

	private:

	protected:
		EmissionVolume* m_emissionVolume = nullptr;
		ACCESS_CLASS_MEMBERS(ParticleSpawner);
		GET_TYPE_ID;

	};


	class RateSpawner : public ParticleSpawner
	{

	public:
		virtual void Run(ParticleGeneratorInstance* gen_instance, float deltaTime) override;

		float GetSpawnRate() { return m_rate; }
		void SetSpawnRate(float rate) { m_rate = rate; }

	private:
		float m_rate = 100.0f;

	protected:
		ACCESS_CLASS_MEMBERS(RateSpawner);
		GET_TYPE_ID;

	};


}
