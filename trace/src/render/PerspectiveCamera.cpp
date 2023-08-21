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
		yaw = -90.0f;
		pitch = 0.0f;

		//m_upDirection = glm::normalize(glm::cross(m_lookDirection, glm::vec3(0.0f, 1.0f, 0.0f)));
		m_rightDirection = glm::normalize(glm::cross(m_lookDirection, m_upDirection));

		m_projection = glm::perspective(glm::radians(m_fov), m_aspectRatio, m_zNear, m_zFar);
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
		float move_speed = 75.0f;
		float rotate_speed = 50.0f;

		if (InputSystem::get_instance()->GetKeyState(Keys::KEY_W) == KeyState::KEY_HELD)
		{
			m_position += m_lookDirection * move_speed * deltaTime;
		}
		if (InputSystem::get_instance()->GetKeyState(Keys::KEY_S) == KeyState::KEY_HELD)
		{
			m_position -= m_lookDirection * move_speed * deltaTime;
		}
		

		if (InputSystem::get_instance()->GetKeyState(Keys::KEY_D) == KeyState::KEY_HELD)
		{
			m_position += m_rightDirection * move_speed * deltaTime;
		}
		if (InputSystem::get_instance()->GetKeyState(Keys::KEY_A) == KeyState::KEY_HELD)
		{
			m_position -= m_rightDirection * move_speed * deltaTime;
		}
		
		if (InputSystem::get_instance()->GetKeyState(Keys::KEY_Q) == KeyState::KEY_HELD)
		{
			m_position += m_upDirection * move_speed * deltaTime;
		}
		if (InputSystem::get_instance()->GetKeyState(Keys::KEY_E) == KeyState::KEY_HELD)
		{
			m_position -= m_upDirection * move_speed * deltaTime;
		}


		if (InputSystem::get_instance()->GetKeyState(Keys::KEY_UP) == KeyState::KEY_HELD)
		{
			pitch += rotate_speed * deltaTime;

			float _rot = 1.0f * rotate_speed * deltaTime;
			glm::mat4 _rotation = glm::identity<glm::mat4>();
			_rotation = glm::rotate(_rotation, glm::radians(_rot), m_rightDirection);

			m_lookDirection = _rotation * glm::vec4(m_lookDirection, 0.0f);
			m_lookDirection = glm::normalize(m_lookDirection);
			m_upDirection = _rotation * glm::vec4(m_upDirection, 0.0f);
			m_upDirection = glm::normalize(m_upDirection);

			recompute();
		}
		if (InputSystem::get_instance()->GetKeyState(Keys::KEY_DOWN) == KeyState::KEY_HELD)
		{
			pitch -= rotate_speed * deltaTime;

			float _rot = -1.0f * rotate_speed * deltaTime;
			glm::mat4 _rotation = glm::identity<glm::mat4>();
			_rotation = glm::rotate(_rotation, glm::radians(_rot), m_rightDirection);

			m_lookDirection = _rotation * glm::vec4(m_lookDirection, 0.0f);
			m_lookDirection = glm::normalize(m_lookDirection);
			m_upDirection = _rotation * glm::vec4(m_upDirection, 0.0f);
			m_upDirection = glm::normalize(m_upDirection);

			recompute();
		}
		if (InputSystem::get_instance()->GetKeyState(Keys::KEY_RIGHT) == KeyState::KEY_HELD)
		{
			yaw += rotate_speed * deltaTime;

			float _rot = -1.0f * rotate_speed * deltaTime;
			glm::mat4 _rotation = glm::identity<glm::mat4>();
			_rotation = glm::rotate(_rotation, glm::radians(_rot), glm::vec3(0.0f, 1.0f, 0.0f));

			m_lookDirection = _rotation * glm::vec4(m_lookDirection, 0.0f);
			m_lookDirection = glm::normalize(m_lookDirection);
			m_upDirection = _rotation * glm::vec4(m_upDirection, 0.0f);
			m_upDirection = glm::normalize(m_upDirection);
			m_rightDirection = glm::normalize(glm::cross(m_lookDirection, m_upDirection));

			recompute();
		}
		if (InputSystem::get_instance()->GetKeyState(Keys::KEY_LEFT) == KeyState::KEY_HELD)
		{
			yaw -= rotate_speed * deltaTime;

			float _rot = 1.0f * rotate_speed * deltaTime;
			glm::mat4 _rotation = glm::identity<glm::mat4>();
			_rotation = glm::rotate(_rotation, glm::radians(_rot), glm::vec3(0.0f, 1.0f, 0.0f));

			m_lookDirection = _rotation * glm::vec4(m_lookDirection, 0.0f);
			m_lookDirection = glm::normalize(m_lookDirection);
			m_upDirection = _rotation * glm::vec4(m_upDirection, 0.0f);
			m_upDirection = glm::normalize(m_upDirection);
			m_rightDirection = glm::normalize(glm::cross(m_lookDirection, m_upDirection));
			recompute();
		}

		if (pitch > 89.0f)
			pitch = 89.0f;
		else if (pitch < -89.0f)
			pitch = -89.0f;

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

		m_projection = glm::perspective(glm::radians(m_fov), m_aspectRatio, m_zNear, m_zFar);
	}

	void PerspectiveCamera::SetNear(float z_near)
	{
		m_zNear = z_near;

		m_projection = glm::perspective(glm::radians(m_fov), m_aspectRatio, m_zNear, m_zFar);
	}

	void PerspectiveCamera::SetFar(float z_far)
	{
		m_zFar = z_far;

		m_projection = glm::perspective(glm::radians(m_fov), m_aspectRatio, m_zNear, m_zFar);
	}

	void PerspectiveCamera::SetAspectRatio(float aspect_ratio)
	{
		m_aspectRatio = aspect_ratio;

		m_projection = glm::perspective(glm::radians(m_fov), m_aspectRatio, m_zNear, m_zFar);
	}

	void PerspectiveCamera::recompute()
	{
		/*m_lookDirection.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		m_lookDirection.y = sin(glm::radians(pitch));
		m_lookDirection.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
		
		m_lookDirection = glm::normalize(m_lookDirection);

		m_rightDirection = glm::normalize(glm::cross(m_lookDirection, glm::vec3(0.0f, 1.0f, 0.0f)));
		m_upDirection = glm::normalize(glm::cross( m_rightDirection, m_lookDirection));*/


		

	}

}