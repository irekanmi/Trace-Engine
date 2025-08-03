#pragma once

#include "reflection/TypeRegistry.h"

#include "glm/glm.hpp"

namespace trace {

    class EmissionVolume 
    {

    public:
        EmissionVolume() {};
        virtual ~EmissionVolume() {};

        // Get a random point within the volume
        virtual glm::vec3 GetRandomPoint() = 0;

        // Get a random point on the surface of the volume
        virtual glm::vec3 GetRandomSurfacePoint() = 0;

        // Get a random direction for emission (used for surface emission)
        virtual glm::vec3 GetRandomDirection() = 0;
    private:
    protected:
        GET_TYPE_ID;

    };


    class PointVolume : public EmissionVolume 
    {


    public:
        PointVolume() {};
        PointVolume(glm::vec3 pos) : m_position(pos) {}

        glm::vec3 GetRandomPoint() override { return m_position; }

        glm::vec3 GetRandomSurfacePoint() override {
            return m_position;
        }

        glm::vec3 GetRandomDirection() override;

    private:
        glm::vec3 m_position;

    protected:
        ACCESS_CLASS_MEMBERS(PointVolume);
        GET_TYPE_ID;

    };


    class SphereVolume : public EmissionVolume 
    {

    public:
        SphereVolume() {}
        SphereVolume(const glm::vec3& c, float r)
            : m_center(c), m_radius(r) {}

        glm::vec3 GetRandomPoint() override {
            // Random point inside sphere
            glm::vec3 dir = GetRandomDirection();
            float r = m_radius * pow(rand() / (float)RAND_MAX, 1.0f / 3.0f);
            return m_center + dir * r;
        }

        glm::vec3 GetRandomSurfacePoint() override {
            // Random point on sphere surface
            return m_center + GetRandomDirection() * m_radius;
        }

        glm::vec3 GetRandomDirection() override;

    private:
        glm::vec3 m_center;
        float m_radius;

    protected:
        ACCESS_CLASS_MEMBERS(SphereVolume);
        GET_TYPE_ID;
        
    };



    class BoxVolume : public EmissionVolume 
    {

    public:
        BoxVolume() {}
        BoxVolume(const glm::vec3& c,
            const glm::vec3& size,
            const glm::mat3& rotation)
            : m_center(c), m_extents(size * 0.5f) {
            m_axis[0] = rotation[0];
            m_axis[1] = rotation[1];
            m_axis[2] = rotation[2];
        }

        glm::vec3 GetRandomPoint() override;

        glm::vec3 GetRandomSurfacePoint() override;

        glm::vec3 GetRandomDirection() override;

    private:
        glm::vec3 m_center;
        glm::vec3 m_extents; // Half sizes
        glm::vec3 m_axis[3]; // Orientation axes

    protected:
        ACCESS_CLASS_MEMBERS(BoxVolume);
        GET_TYPE_ID;
    };

    class CircleVolume : public EmissionVolume {

    public:
        CircleVolume() {};
        CircleVolume(const glm::vec3& c,
            const glm::vec3& n,
            float r)
            : m_center(c), m_normal(glm::normalize(n)), m_radius(r) {}

        glm::vec3 GetRandomPoint() override;

        glm::vec3 GetRandomSurfacePoint() override;

        glm::vec3 GetRandomDirection() override;

    private:
        glm::vec3 get_tangent();

    private:
        glm::vec3 m_center;
        glm::vec3 m_normal;
        float m_radius;
    protected:
        ACCESS_CLASS_MEMBERS(CircleVolume);
        GET_TYPE_ID;
    };


}
