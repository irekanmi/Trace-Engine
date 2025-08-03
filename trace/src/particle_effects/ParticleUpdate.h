#pragma once

#include "reflection/TypeRegistry.h"

#include "glm/glm.hpp"

namespace trace {

	class ParticleGeneratorInstance;

	class ParticleUpdate
	{

	public:
		virtual void UpdateParticle(ParticleGeneratorInstance* particle_generator, uint32_t particle_index, float deltaTime) = 0;

	private:
	protected:
		GET_TYPE_ID;

	};

    class GravityUpdate : public ParticleUpdate 
    {

    public:
        GravityUpdate(const glm::vec3& gravity = glm::vec3(0.0f, -9.8f, 0.0f))
            : m_gravity(gravity) {}

        virtual void UpdateParticle(ParticleGeneratorInstance* particle_generator, uint32_t particle_index, float deltaTime) override;

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
