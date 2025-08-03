#pragma once


#include "scene/UUID.h"
#include "resource/Ref.h"
#include "particle_effects/ParticleSpawner.h"
#include "particle_effects/ParticleInitialize.h"
#include "particle_effects/ParticleUpdate.h"
#include "particle_effects/ParticleRender.h"
#include "serialize/DataStream.h"
#include "particle_effects/ParticleData.h"
#include "reflection/TypeRegistry.h"

#include "glm/glm.hpp"

namespace trace {

	
	class ParticleEffectInstance;
	class Camera;

	class ParticleGenerator : public Resource
	{

	public:

		bool Create();
		virtual void Destroy() override;

		uint32_t GetCapacity() { return m_capacity; }
		void SetCapacity(uint32_t capacity) { m_capacity = capacity; }

		ParticleSpawner* GetSpawner() { return m_spawner; }
		void SetSpawner(ParticleSpawner* spawner) { m_spawner = spawner; }

		std::vector<ParticleInitializer*>& GetInitializers() { return m_initializers; }
		void SetInitializers(std::vector<ParticleInitializer*>& initializers) { m_initializers = initializers; }
		
		std::vector<ParticleUpdate*>& GetUpdates() { return m_updates; }
		void SetUpdates(std::vector<ParticleUpdate*>& updates) { m_updates = updates; }
		
		std::vector<ParticleRender*>& GetRenderers() { return m_renderers; }
		void SetRenderers(std::vector<ParticleRender*>& renderers) { m_renderers = renderers; }


		static Ref<ParticleGenerator> Deserialize(UUID id);
		static Ref<ParticleGenerator> Deserialize(DataStream* stream);

	private:
		ParticleSpawner* m_spawner = nullptr;
		std::vector<ParticleInitializer*> m_initializers;
		std::vector<ParticleUpdate*> m_updates;
		std::vector<ParticleRender*> m_renderers;
		uint32_t m_capacity;

	protected:
		ACCESS_CLASS_MEMBERS(ParticleGenerator);

	};

	class ParticleGeneratorInstance
	{

	public:

		bool CreateInstance(Ref<ParticleGenerator> generator);

		void Start(ParticleEffectInstance* effect_instance);
		void Update(ParticleEffectInstance* effect_instance, float deltaTime);
		void Stop(ParticleEffectInstance* effect_instance);
		void Render(ParticleEffectInstance* effect_instance, Camera* camera, glm::mat4 transform);

		int32_t emit_particle();
		void initialize_particle(uint32_t index);
		void kill_particle(uint32_t index);

		Ref<ParticleGenerator> GetGenerator() { return m_gen; }
		void SetGenerator(Ref<ParticleGenerator> generator) { m_gen = generator; }

		ParticleData& GetParticlesData() { return m_particleData; }
		float GetElaspedTime() { return m_elaspedTime; }

		uint32_t GetNumAlive() { return m_numAlive; }

	private:
		ParticleData m_particleData;
		uint32_t m_numAlive = 0;
		Ref<ParticleGenerator> m_gen;
		float m_elaspedTime = 0.0f;

	protected:
		ACCESS_CLASS_MEMBERS(ParticleGeneratorInstance);

	};

}
