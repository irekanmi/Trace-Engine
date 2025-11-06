#pragma once

#include "reflection/TypeRegistry.h"
#include "particle_effects/ParticleData.h"
#include "core/Utils.h"

#include "glm/glm.hpp"

namespace trace {

	class ParticleGeneratorInstance;

	class ParticleUpdate : public ParticleBase
	{

	public:
		virtual void UpdateParticle(ParticleGeneratorInstance* particle_generator, uint32_t particle_index, float deltaTime) = 0;

	private:
	protected:
		GET_TYPE_ID;

	};

    class CustomParticleUpdate : public ParticleUpdate
    {

    public:
        CustomParticleUpdate() {};
        ~CustomParticleUpdate() {};

        virtual void UpdateParticle(ParticleGeneratorInstance* particle_generator, uint32_t particle_index, float deltaTime) override;

        UUID GetNodeID() { return m_nodeID; }
        void SetNodeID(UUID node_id) { m_nodeID = node_id; }

        StringID GetName() { return m_name; }
        void SetName(StringID name) { m_name = name; }

    private:
        UUID m_nodeID;
        StringID m_name;

    protected:
        ACCESS_CLASS_MEMBERS(CustomParticleUpdate);
        GET_TYPE_ID;

    };

    class GravityUpdate : public ParticleUpdate 
    {

    public:
        GravityUpdate(const glm::vec3& gravity = glm::vec3(0.0f, -9.8f, 0.0f))
            : m_gravity(gravity) {}

        virtual void UpdateParticle(ParticleGeneratorInstance* particle_generator, uint32_t particle_index, float deltaTime) override;

        glm::vec3 GetGravity() { return m_gravity; }

        void SetGravity(glm::vec3 gravity) { m_gravity = gravity; }

    private:
        glm::vec3 m_gravity;

    protected:
        ACCESS_CLASS_MEMBERS(GravityUpdate);
        GET_TYPE_ID;
    };
    
    class DragUpdate : public ParticleUpdate 
    {

    public:
        DragUpdate(float coefficient = 0.1f)
            : m_dragCoefficient(coefficient) {}

        virtual void UpdateParticle(ParticleGeneratorInstance* particle_generator, uint32_t particle_index, float deltaTime) override;

        float GetDragCoefficient() { return m_dragCoefficient; }

        void SetDragCoefficient(float drag_coefficient) { m_dragCoefficient = drag_coefficient; }

    private:
        float m_dragCoefficient;

    protected:
        ACCESS_CLASS_MEMBERS(DragUpdate);
        GET_TYPE_ID;
    };

    class WindUpdate : public ParticleUpdate 
    {

    public:
        WindUpdate(const glm::vec3& force = glm::vec3(5.0f, 0.0f, 0.0f),
            float turbulence = 0.5f)
            : m_windForce(force), m_turbulence(turbulence) {}

        virtual void UpdateParticle(ParticleGeneratorInstance* particle_generator, uint32_t particle_index, float deltaTime) override;

        glm::vec3 GetWindForce() { return m_windForce; }
        float GetTurbulence() { return m_turbulence; }

        void SetWindForce(glm::vec3 wind_force) { m_windForce = wind_force; }
        void SetTurbulence(float turbulence) { m_turbulence = turbulence; }

    private:
        glm::vec3 m_windForce;
        float m_turbulence;

    protected:
        ACCESS_CLASS_MEMBERS(WindUpdate);
        GET_TYPE_ID;

    };
    
    class VelocityUpdate : public ParticleUpdate 
    {

    public:
        VelocityUpdate() {}

        virtual void UpdateParticle(ParticleGeneratorInstance* particle_generator, uint32_t particle_index, float deltaTime) override;


    private:

    protected:
        ACCESS_CLASS_MEMBERS(VelocityUpdate);
        GET_TYPE_ID;

    };

}
