#include "pch.h"

#include "PerspectiveCamera.h"
#include "glm/gtc/matrix_transform.hpp"
#include "core/input/Input.h"

namespace trace {



	PerspectiveCamera::PerspectiveCamera()
	{
		m_type = CameraType::PERSPECTIVE;
	}

	PerspectiveCamera::PerspectiveCamera(glm::vec3 position, glm::vec3 look_dir, glm::vec3 up_dir, float aspect_ratio, float fov, float z_near, float z_far)
	{
		m_type = CameraType::PERSPECTIVE;

		m_position = position;
		m_lookDirection = look_dir;
		m_upDirection = up_dir;
		m_aspectRatio = aspect_ratio;
		m_fov = fov;
		m_zNear = z_near;
		m_zFar = z_far;

		m_projection = glm::perspective(m_fov, m_aspectRatio, m_zNear, m_zFar);

	}

	PerspectiveCamera::~PerspectiveCamera()
	{
	}

	glm::vec3 PerspectiveCamera::GetPosition()
	{
		return m_position;
	}

	glm::vec3 PerspectiveCamera::GetLookDir()
	{
		return m_lookDirection;
	}

	glm::vec3 PerspectiveCamera::GetUpDir()
	{
		return m_upDirection;
	}

	glm::mat4 PerspectiveCamera::GetViewMatrix()
	{
		glm::mat4 result = glm::lookAt(m_position, m_lookDirection + m_position, m_upDirection);

		return result;
	}

	glm::mat4 PerspectiveCamera::GetProjectionMatix()
	{
		return m_projection;
	}

	float PerspectiveCamera::GetFov()
	{
		return m_fov;
	}

	float PerspectiveCamera::GetNear()
	{
		return m_zNear;
	}

	float PerspectiveCamera::GetFar()
	{
		return m_zFar;
	}

	float PerspectiveCamera::GetAspectRatio()
	{
		return m_aspectRatio;
	}

	void PerspectiveCamera::Update(float deltaTime)
	{
		if (InputSystem::get_instance()->GetKeyState(Keys::KEY_W) == KeyState::KEY_HELD)
		{
			m_position += m_lookDirection;
		}
		if (InputSystem::get_instance()->GetKeyState(Keys::KEY_S) == KeyState::KEY_HELD)
		{
			m_position -= m_lookDirection;
		}

		if (InputSystem::get_instance()->GetKeyState(Keys::KEY_D) == KeyState::KEY_HELD)
		{
			glm::vec3 value = glm::cross(m_lookDirection, m_upDirection);
			m_position += value;
		}
		if (InputSystem::get_instance()->GetKeyState(Keys::KEY_A) == KeyState::KEY_HELD)
		{
			glm::vec3 value = glm::cross(m_lookDirection, m_upDirection);
			m_position -= value;
		}
	}

	void PerspectiveCamera::SetPosition(glm::vec3 position)
	{
		m_position = position;
	}

	void PerspectiveCamera::SetLookDir(glm::vec3 look_dir)
	{
		m_lookDirection = look_dir;
	}

	void PerspectiveCamera::SetUpDir(glm::vec3 up_dir)
	{
		m_upDirection = up_dir;
	}

	void PerspectiveCamera::SetFov(float fov)
	{
		m_fov = fov;

		m_projection = glm::perspective(m_fov, m_aspectRatio, m_zNear, m_zFar);
	}

	void PerspectiveCamera::SetNear(float z_near)
	{
		m_zNear = z_near;

		m_projection = glm::perspective(m_fov, m_aspectRatio, m_zNear, m_zFar);
	}

	void PerspectiveCamera::SetFar(float z_far)
	{
		m_zFar = z_far;

		m_projection = glm::perspective(m_fov, m_aspectRatio, m_zNear, m_zFar);
	}

	void PerspectiveCamera::SetAspectRatio(float aspect_ratio)
	{
		m_aspectRatio = aspect_ratio;

		m_projection = glm::perspective(m_fov, m_aspectRatio, m_zNear, m_zFar);
	}

}