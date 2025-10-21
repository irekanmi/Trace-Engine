#include "pch.h"

#include "particle_effects/EmissionVolume.h"
#include "core/Utils.h"

#include "glm/common.hpp"
#include <type_traits>

namespace trace {


	glm::vec3 PointVolume::GetRandomDirection()
	{
        // Uniform random direction
        float theta = 2.0f * glm::pi<float>() * RandomFloat();
        float phi = acos(1.0f - 2.0f * RandomFloat());
        return glm::vec3(
            sin(phi) * cos(theta),
            sin(phi) * sin(theta),
            cos(phi)
        );
	}

    glm::vec3 SphereVolume::GetRandomDirection()
    {
        // Same as PointVolume
        float theta = 2.0f * glm::pi<float>() * RandomFloat();
        float phi = acos(1.0f - 2.0f * RandomFloat());
        return glm::vec3(
            sin(phi) * cos(theta),
            sin(phi) * sin(theta),
            cos(phi)
        );
    }

    BoxVolume::BoxVolume()
    {
        m_center = glm::vec3(0.0f);
        m_extents = glm::vec3(0.5f); // Half sizes
        glm::vec3 axis[3] = {
            glm::vec3(1.0f, 0.0f, 0.0f),
            glm::vec3(0.0f, 1.0f, 0.0f),
            glm::vec3(0.0f, 0.0f, 1.0f)
        }; // Orientation axes

        m_axis[0] = axis[0];
        m_axis[1] = axis[1];
        m_axis[2] = axis[2];

    }

    glm::vec3 BoxVolume::GetRandomPoint()
    {
        // Random point inside box
        float x = (RandomFloat() * 2.0f - 1.0f) * m_extents.x;
        float y = (RandomFloat() * 2.0f - 1.0f) * m_extents.y;
        float z = (RandomFloat() * 2.0f - 1.0f) * m_extents.z;

        return m_center + m_axis[0] * x + m_axis[1] * y + m_axis[2] * z;
    }

    glm::vec3 BoxVolume::GetRandomSurfacePoint()
    {
        // Random point on box surface
        int face = int(RandomFloat() * 600.0f) % 6;
        float u = RandomFloat() * 2.0f - 1.0f;
        float v = RandomFloat() * 2.0f - 1.0f;

        glm::vec3 point;
        switch (face) 
        {
        case 0: point = glm::vec3(m_extents.x, u * m_extents.y, v * m_extents.z); break;
        case 1: point = glm::vec3(-m_extents.x, u * m_extents.y, v * m_extents.z); break;
        case 2: point = glm::vec3(u * m_extents.x, m_extents.y, v * m_extents.z); break;
        case 3: point = glm::vec3(u * m_extents.x, -m_extents.y, v * m_extents.z); break;
        case 4: point = glm::vec3(u * m_extents.x, v * m_extents.y, m_extents.z); break;
        case 5: point = glm::vec3(u * m_extents.x, v * m_extents.y, -m_extents.z); break;
        }

        return m_center + m_axis[0] * point.x + m_axis[1] * point.y + m_axis[2] * point.z;
    }

    glm::vec3 BoxVolume::GetRandomDirection()
    {
        // Direction normal to random face
        int face = int(RandomFloat() * 600.0f) % 6;
        glm::vec3 normal;
        switch (face) 
        {
        case 0: normal = m_axis[0]; break;
        case 1: normal = -m_axis[0]; break;
        case 2: normal = m_axis[1]; break;
        case 3: normal = -m_axis[1]; break;
        case 4: normal = m_axis[2]; break;
        case 5: normal = -m_axis[2]; break;
        }
        return normal;
    }

    void BoxVolume::SetAxis(glm::vec3* axis)
    {
        m_axis[0] = axis[0];
        m_axis[1] = axis[1];
        m_axis[2] = axis[2];
    }

    glm::vec3 CircleVolume::GetRandomPoint()
    {
        // Random point inside circle
        glm::vec3 tangent = get_tangent();
        glm::vec3 bitangent = glm::cross(m_normal, tangent);

        float r = m_radius * sqrt(RandomFloat());
        float theta = 2.0f * glm::pi<float>() * (RandomFloat());

        return m_center + (tangent * cos(theta) + bitangent * sin(theta)) * r;
    }

    glm::vec3 CircleVolume::GetRandomSurfacePoint()
    {
        // Edge of the circle
        glm::vec3 tangent = get_tangent();
        glm::vec3 bitangent = glm::cross(m_normal, tangent);

        float theta = 2.0f * glm::pi<float>() * RandomFloat();
        return m_center + (tangent * cos(theta) + bitangent * sin(theta)) * m_radius;
    }

    glm::vec3 CircleVolume::GetRandomDirection()
    {
        // Either m_normal or radial direction
        int nrm = int(RandomFloat() * 600.0f) % 6;
        if (nrm % 2 == 0) {
            return m_normal;
        }
        else {
            glm::vec3 tangent = get_tangent();
            glm::vec3 bitangent = glm::cross(m_normal, tangent);
            float theta = 2.0f * glm::pi<float>() * RandomFloat();
            return tangent * cos(theta) + bitangent * sin(theta);
        }
    }

    glm::vec3 CircleVolume::get_tangent()
    {
        // Find a vector perpendicular to normal
        glm::vec3 tangent;
        if (fabs(m_normal.x) > fabs(m_normal.y)) {
            tangent = glm::vec3(m_normal.z, 0.0f, -m_normal.x);
        }
        else {
            tangent = glm::vec3(0.0f, -m_normal.z, m_normal.y);
        }
        return glm::normalize(tangent);
    }

}