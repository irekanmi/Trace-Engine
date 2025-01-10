#pragma once

#include "core/Core.h"
#include "core/Enums.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/quaternion.hpp"

namespace trace {

	class TRACE_API Transform
	{

	public:
		Transform();
		Transform(glm::vec3 position);
		Transform(glm::vec3 position, glm::quat rotation);
		Transform(glm::vec3 position, glm::quat rotation, glm::vec3 scale);
		Transform(glm::quat rotation);
		Transform(glm::quat rotation, glm::vec3 scale);
		Transform(float scale_x, float scale_y, float scale_z);
		~Transform();

		// Returns value of the position
		glm::vec3 GetPosition();
		// Returns value of the scale
		glm::vec3 GetScale();
		// Returns value of the rotation
		glm::quat GetRotation();
		// Return the local space matrix
		glm::mat4 GetLocalMatrix();
		// Return the rotation in euler angle
		glm::vec3 GetRotationEuler();

		glm::vec3 GetForward();
		glm::vec3 GetRight();
		glm::vec3 GetUp();

		// Overrides current position
		void SetPosition(glm::vec3 position);
		// Overrides current scale
		void SetScale(glm::vec3 scale);
		// Overrides current rotation
		void SetRotation(glm::quat rotation);
		// Overrides current rotation
		void SetRotationEuler(glm::vec3 rotation);

		void Translate(glm::vec3 value);
		void Rotate(float value, glm::vec3 direction);
		void Rotate(glm::vec3 euler);// NOTE: Used to set the rotation
		void RotateBy(glm::vec3 euler);// NOTE: Used to rotate by certain amount
		void Scale(float value);
		void Scale(glm::vec3 value);

		bool IsDirty() { return m_dirty; }
		void SetDirty(bool dirty) { m_dirty = dirty; }

		static Transform CombineTransform(Transform& parent, Transform& child);
		static Transform CombineTransform_Direction(Transform& parent, Transform& child);
		static Transform Identity();

	private:
		void recalculate_local_matrix();

	private:
		glm::mat4 m_model = glm::identity<glm::mat4>();
		glm::quat m_rotation = glm::identity<glm::quat>();
		glm::vec3 m_position = glm::vec3(0.0f);
		glm::vec3 m_scale = glm::vec3(0.0f);
		bool m_dirty = true;

	protected:

	};

}
