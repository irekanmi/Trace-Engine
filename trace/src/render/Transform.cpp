#include "pch.h"

#include "Transform.h"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/quaternion.hpp"
#include "glm/gtx/euler_angles.hpp"
#include "core/Utils.h"





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

	glm::vec3 Transform::GetForward()
	{
		glm::mat4 rot = glm::toMat4(m_rotation);
		glm::vec3 result = rot * glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
		return glm::normalize(result);
	}

	glm::vec3 Transform::GetRight()
	{
		glm::mat4 rot = glm::toMat4(m_rotation);
		glm::vec3 result = rot * glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
		return glm::normalize(result);
	}

	glm::vec3 Transform::GetUp()
	{
		glm::mat4 rot = glm::toMat4(m_rotation);
		glm::vec3 result = rot * glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
		return glm::normalize(result);
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

	void Transform::RotateBy(glm::vec3 euler)
	{
		glm::vec3 rot = GetRotationEuler() + euler;
		Rotate(rot);
	}

	void Transform::Scale(float value)
	{
		m_scale += glm::vec3(value);
		m_dirty = true;
	}

	void Transform::Scale(glm::vec3 value)
	{
		m_scale += (value);
		m_dirty = true;
	}

	Transform Transform::CombineTransform(Transform& parent, Transform& child)
	{
		Transform result;
		result.SetScale(parent.GetScale() * child.GetScale());
		result.SetRotation(parent.GetRotation() * child.GetRotation());

		glm::vec3 child_pos = child.GetPosition();
		child_pos *= parent.GetScale();
		child_pos = glm::rotate(parent.GetRotation(), child_pos);
		child_pos += parent.GetPosition();

		result.SetPosition(child_pos);

		return result;
	}

	Transform Transform::CombineTransform_Direction(Transform& parent, Transform& child)
	{
		Transform result;
		result.SetScale(parent.GetScale() * child.GetScale());
		result.SetRotation(parent.GetRotation() * child.GetRotation());

		glm::vec3 child_pos = child.GetPosition();
		child_pos *= parent.GetScale();
		child_pos = glm::rotate(parent.GetRotation(), child_pos);

		result.SetPosition(child_pos);

		return result;
	}

	Transform Transform::Identity()
	{
		return Transform();
	}

	Transform Transform::Blend(Transform& source, Transform& target, float blend_weight)
	{
		Transform out_pose;

		glm::vec3 pos_a = source.GetPosition();
		glm::vec3 pos_b = target.GetPosition();

		glm::vec3 pos_res(0.0f);
		pos_res.x = lerp(pos_a.x, pos_b.x, blend_weight);
		pos_res.y = lerp(pos_a.y, pos_b.y, blend_weight);
		pos_res.z = lerp(pos_a.z, pos_b.z, blend_weight);

		out_pose.SetPosition(pos_res);

		glm::quat rot_a = source.GetRotation();
		glm::quat rot_b = target.GetRotation();

		glm::quat rot_res;
		rot_res = glm::slerp(rot_a, rot_b, blend_weight);

		out_pose.SetRotation(rot_res);

		glm::vec3 scale_a = source.GetScale();
		glm::vec3 scale_b = target.GetScale();

		glm::vec3 scale_res(0.0f);
		scale_res.x = lerp(scale_a.x, scale_b.x, blend_weight);
		scale_res.y = lerp(scale_a.y, scale_b.y, blend_weight);
		scale_res.z = lerp(scale_a.z, scale_b.z, blend_weight);

		out_pose.SetScale(scale_res);

		return out_pose;
	}

	void Transform::ApplyRootMotion(Transform& pose, Transform& root_motion_delta)
	{
		Transform model_space_delta = Transform::CombineTransform_Direction(pose, root_motion_delta);
		pose.Translate(model_space_delta.GetPosition());
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