#pragma once

#include "particle_effects/ParticleData.h"
#include "reflection/TypeRegistry.h"

namespace trace {

	class ParticleGeneratorInstance;

	class ParticleInitializer : public ParticleBase
	{

	public:

		virtual void Init(ParticleGeneratorInstance* particle_generator) = 0;
		virtual void InitParticle(ParticleGeneratorInstance* particle_generator, uint32_t particle_index) = 0;

	private:
	protected:
		GET_TYPE_ID;

	};

    class VelocityInitializer : public ParticleInitializer
    {

    public:
		VelocityInitializer() {}
		
		VelocityInitializer(const glm::vec3& minVel, const glm::vec3& maxVel)
            : m_minVelocity(minVel), m_maxVelocity(maxVel) {}

		virtual void Init(ParticleGeneratorInstance* particle_generator) override;
		virtual void InitParticle(ParticleGeneratorInstance* particle_generator, uint32_t particle_index) override;

		glm::vec3 GetMinVelocity() { return m_minVelocity; }
		glm::vec3 GetMaxVelocity() { return m_maxVelocity; }

		void SetMinVelocity(glm::vec3 min_velocity) { m_minVelocity = min_velocity; }
		void SetMaxVelocity(glm::vec3 max_velocity) { m_maxVelocity = max_velocity; }

    private:
        glm::vec3 m_minVelocity;
        glm::vec3 m_maxVelocity;

    protected:
		ACCESS_CLASS_MEMBERS(VelocityInitializer);
		GET_TYPE_ID;
    };

	class LifetimeInitializer : public ParticleInitializer
	{

	public:
		LifetimeInitializer() {}

		virtual void Init(ParticleGeneratorInstance* particle_generator) override;
		virtual void InitParticle(ParticleGeneratorInstance* particle_generator, uint32_t particle_index) override;

		float GetMin() { return m_min; }
		float GetMax() { return m_max; }

		void SetMin(float min) { m_min = min; }
		void SetMax(float max) { m_max = max; }

	private:
		float m_min;
		float m_max;

	protected:
		ACCESS_CLASS_MEMBERS(LifetimeInitializer);
		GET_TYPE_ID;
	};

	class CustomParticleInitializer : public ParticleInitializer
	{

	public:
		CustomParticleInitializer() {};
		~CustomParticleInitializer() {};

		virtual void Init(ParticleGeneratorInstance* particle_generator) override;
		virtual void InitParticle(ParticleGeneratorInstance* particle_generator, uint32_t particle_index) override;

		UUID GetNodeID() { return m_nodeID; }
		void SetNodeID(UUID node_id) { m_nodeID = node_id; }

	private:
		UUID m_nodeID;

	protected:
		ACCESS_CLASS_MEMBERS(CustomParticleInitializer);
		GET_TYPE_ID;

	};

}
