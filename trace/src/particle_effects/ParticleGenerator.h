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
#include "node_system/GenericGraph.h"
#include "core/Utils.h"

#include "glm/glm.hpp"

namespace trace {

	
	class ParticleEffectInstance;
	class Camera;

	class ParticleGenerator : public GenericGraph, public Resource
	{

	public:

		bool Create();
		bool Create(void* ptr);
		virtual void Destroy() override;

		virtual ~ParticleGenerator() {}

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

		UUID GetEffectRoot() { return m_effectRoot; }
		std::vector<StringID>& GetCustomData() { return m_customDataNames; }

		static Ref<ParticleGenerator> Deserialize(UUID id);
		static Ref<ParticleGenerator> Deserialize(DataStream* stream);

	private:
		ParticleSpawner* m_spawner = nullptr;
		std::vector<ParticleInitializer*> m_initializers;
		std::vector<ParticleUpdate*> m_updates;
		std::vector<ParticleRender*> m_renderers;
		uint32_t m_capacity;
		UUID m_effectRoot = 0;
		std::vector<StringID> m_customDataNames;


	protected:
		virtual void CreateParameter(const std::string& param_name, GenericValueType type) override;

		ACCESS_CLASS_MEMBERS(ParticleGenerator);
		GET_TYPE_ID;

	};

	class ParticleGeneratorInstance : public GenericGraphInstance
	{

	public:

		~ParticleGeneratorInstance() 
		{
		}

		bool CreateInstance(Ref<ParticleGenerator> generator);
		virtual void DestroyInstance() override;

		void Start(ParticleEffectInstance* effect_instance);
		void Update(ParticleEffectInstance* effect_instance, float deltaTime);
		void Stop(ParticleEffectInstance* effect_instance);
		void Render(ParticleEffectInstance* effect_instance, Camera* camera, glm::mat4 transform, int32_t render_graph_index = 0);
		ParticleEffectInstance* GetEffectInstance() { return m_effectInstance; }

		int32_t emit_particle();
		void initialize_particle(uint32_t index);
		void kill_particle(uint32_t index);

		Ref<ParticleGenerator> GetGenerator() { return m_gen; }
		void SetGenerator(Ref<ParticleGenerator> generator) { m_gen = generator; }

		ParticleData& GetParticlesData() { return m_particleData; }
		float GetElaspedTime() { return m_elaspedTime; }
		bool GetParticleAttribute(StringID attr_name, glm::vec4& out_data, uint32_t particle_index);
		bool SetParticleAttribute(StringID attr_name, glm::vec4& data, uint32_t particle_index);

		uint32_t GetNumAlive() { return m_numAlive; }

	private:
		ParticleData m_particleData;
		uint32_t m_numAlive = 0;
		Ref<ParticleGenerator> m_gen;
		float m_elaspedTime = 0.0f;
		ParticleEffectInstance* m_effectInstance;

	protected:
		ACCESS_CLASS_MEMBERS(ParticleGeneratorInstance);


		// Inherited via GenericGraphInstance
		virtual void set_parameter_data(const std::string& param_name, void* data, uint32_t size) override;

	};

}
