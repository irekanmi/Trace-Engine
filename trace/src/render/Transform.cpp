#include "pch.h"

#include "Transform.h"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/quaternion.hpp"
#include "glm/gtx/euler_angles.hpp"





namespace trace {



	Transform::Transform()
	{
		m_position = glm::vec3(0.0f);
		m_rotation = glm::identity<glm::quat>();
		m_scale = glm::vec3(1.0f);
		m_model = glm::identity<glm::mat4>();
		m_dirty = true;
	}

	Transform::Transform(glm::vec3 position)
	{
		m_position = position;
		m_rotation = glm::identity<glm::quat>();
		m_scale = glm::vec3(1.0f);
		m_model = glm::identity<glm::mat4>();
		m_dirty = true;
	}

	Transform::Transform(float scale_x, float scale_y, float scale_z)
	{
		m_position = glm::vec3(0.0f);
		m_rotation = glm::identity<glm::quat>();
		m_scale.x = scale_x, m_scale.y = scale_y, m_scale.z = scale_z;
		m_model = glm::identity<glm::mat4>();
		m_dirty = true;
	}

	Transform::~Transform()
	{
	}

	glm::vec3 Transform::GetPosition()
	{
		return m_position;
	}

	glm::vec3 Transform::GetScale()
	{
		return m_scale;
	}

	glm::quat Transform::GetRotation()
	{
		return m_rotation;
	}

	glm::mat4 Transform::GetLocalMatrix()
	{
		if (m_dirty)
		{
			recalculate_local_matrix();
			m_dirty = false;
		}

		return m_model;
	}

	glm::vec3 Transform::GetRotationEuler()
	{
		return glm::degrees(glm::eulerAngles(m_rotation));
	}

	void Transform::SetPosition(glm::vec3 position)
	{
		m_position = position;
		m_dirty = true;
	}

	void Transform::SetScale(glm::vec3 scale)
	{
		m_scale = scale;
		m_dirty = true;
	}

	void Transform::SetRotation(glm::quat rotation)
	{
		m_rotation = rotation;
		m_dirty = true;
	}

	void Transform::SetRotationEuler(glm::vec3 rotation)
	{
		m_rotation = glm::quat(glm::radians(rotation));
		m_dirty = true;
	}

	void Transform::Translate(glm::vec3 value)
	{
		m_position += value;
		m_dirty = true;
	}

	// value - is expressed in degrees
	void Transform::Rotate(float value, glm::vec3 direction)
	{
		m_rotation = glm::rotate(m_rotation, glm::radians(value), direction);
		m_dirty = true;
	}

	void Transform::Rotate(glm::vec3 euler)
	{
		m_rotation = glm::quat(glm::radians(euler));
		m_dirty = true;
	}

	void Transform::Scale(float value)
	{
		m_scale += glm::vec3(value);
		m_dirty = true;
	}

	void Transform::recalculate_local_matrix()
	{
		m_model = glm::identity<glm::mat4>();
		m_model = glm::translate(m_model, m_position);
		m_model *= glm::toMat4(m_rotation);
		m_model = glm::scale(m_model, m_scale);
	}

	Transform::Transform(glm::vec3 position, glm::quat rotation)
	{
		m_position = position;
		m_rotation = rotation;
		m_scale = glm::vec3(1.0f);
		m_model = glm::identity<glm::mat4>();
		m_dirty = true;
	}

	Transform::Transform(glm::vec3 position, glm::quat rotation, glm::vec3 scale)
	{
		m_position = position;
		m_rotation = rotation;
		m_scale = scale;
		m_model = glm::identity<glm::mat4>();
		m_dirty = true;
	}

	Transform::Transform(glm::quat rotation)
	{
		m_position = glm::vec3(0.0f);
		m_rotation = rotation;
		m_scale = glm::vec3(1.0f);
		m_model = glm::identity<glm::mat4>();
		m_dirty = true;
	}

	Transform::Transform(glm::quat rotation, glm::vec3 scale)
	{
		m_position = glm::vec3(0.0f);
		m_rotation = rotation;
		m_scale = scale;
		m_model = glm::identity<glm::mat4>();
		m_dirty = true;
	}



}