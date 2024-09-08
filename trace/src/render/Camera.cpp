#include "pch.h"

#include "Camera.h"
#include "glm/gtc/matrix_transform.hpp"
#include "core/input/Input.h"

namespace trace {

	Camera::Camera(CameraType cam_type)
	{
		m_type = cam_type;
	}


	Camera::~Camera()
	{
	}

	glm::vec3 Camera::GetPosition()
	{
		return m_position;
	}

	glm::vec3 Camera::GetLookDir()
	{
		return m_lookDirection;
	}

	glm::vec3 Camera::GetUpDir()
	{
		return m_upDirection;
	}

	glm::mat4 Camera::GetViewMatrix()
	{
		if (is_dirty[1])
		{
			m_view = glm::lookAt(m_position, m_lookDirection + m_position, m_upDirection);
			is_dirty[1] = false;
		}

		return m_view;
	}

	glm::mat4 Camera::GetProjectionMatix()
	{
		if (is_dirty[0])
		{
			if (m_type == CameraType::PERSPECTIVE)
			{
				m_projection = glm::perspective(glm::radians(m_fov), m_aspectRatio, m_zNear, m_zFar);
				m_projection[1][1] *= -1.0f; //FIX: Added due to vulkan viewport issuses
			}
			else if (m_type == CameraType::ORTHOGRAPHIC)
			{
				float left = -m_orthographicSize * m_aspectRatio;
				float right = m_orthographicSize * m_aspectRatio;
				float top = m_orthographicSize;
				float bottom = -m_orthographicSize;

				m_projection = glm::ortho(left, right, bottom, top, m_zNear, m_zFar);
				m_projection[1][1] *= -1.0f; //FIX: Added due to vulkan viewport issuses
			}
			is_dirty[0] = false;
		}
		return m_projection;
	}

	float Camera::GetFov()
	{
		return m_fov;
	}

	float Camera::GetNear()
	{
		return m_zNear;
	}

	float Camera::GetFar()
	{
		return m_zFar;
	}

	float Camera::GetAspectRatio()
	{
		return m_aspectRatio;
	}

	void Camera::Update(float deltaTime)
	{
		float move_speed = 75.0f;
		float rotate_speed = 50.0f;

		if (InputSystem::get_instance()->GetKeyState(Keys::KEY_W) == KeyState::KEY_HELD)
		{
			m_position += m_lookDirection * move_speed * deltaTime;
			is_dirty[1] = true;

		}
		if (InputSystem::get_instance()->GetKeyState(Keys::KEY_S) == KeyState::KEY_HELD)
		{
			m_position -= m_lookDirection * move_speed * deltaTime;
			is_dirty[1] = true;

		}


		if (InputSystem::get_instance()->GetKeyState(Keys::KEY_D) == KeyState::KEY_HELD)
		{
			m_position += m_rightDirection * move_speed * deltaTime;
			is_dirty[1] = true;

		}
		if (InputSystem::get_instance()->GetKeyState(Keys::KEY_A) == KeyState::KEY_HELD)
		{
			m_position -= m_rightDirection * move_speed * deltaTime;
			is_dirty[1] = true;

		}

		if (InputSystem::get_instance()->GetKeyState(Keys::KEY_Q) == KeyState::KEY_HELD)
		{
			m_position += m_upDirection * move_speed * deltaTime;
			is_dirty[1] = true;

		}
		if (InputSystem::get_instance()->GetKeyState(Keys::KEY_E) == KeyState::KEY_HELD)
		{
			m_position -= m_upDirection * move_speed * deltaTime;
			is_dirty[1] = true;

		}


		if (InputSystem::get_instance()->GetKeyState(Keys::KEY_UP) == KeyState::KEY_HELD)
		{

			float _rot = 1.0f * rotate_speed * deltaTime;
			glm::mat4 _rotation = glm::identity<glm::mat4>();
			_rotation = glm::rotate(_rotation, glm::radians(_rot), m_rightDirection);

			m_lookDirection = _rotation * glm::vec4(m_lookDirection, 0.0f);
			m_lookDirection = glm::normalize(m_lookDirection);
			m_upDirection = _rotation * glm::vec4(m_upDirection, 0.0f);
			m_upDirection = glm::normalize(m_upDirection);
			is_dirty[1] = true;

		}
		if (InputSystem::get_instance()->GetKeyState(Keys::KEY_DOWN) == KeyState::KEY_HELD)
		{

			float _rot = -1.0f * rotate_speed * deltaTime;
			glm::mat4 _rotation = glm::identity<glm::mat4>();
			_rotation = glm::rotate(_rotation, glm::radians(_rot), m_rightDirection);

			m_lookDirection = _rotation * glm::vec4(m_lookDirection, 0.0f);
			m_lookDirection = glm::normalize(m_lookDirection);
			m_upDirection = _rotation * glm::vec4(m_upDirection, 0.0f);
			m_upDirection = glm::normalize(m_upDirection);
			is_dirty[1] = true;

		}
		if (InputSystem::get_instance()->GetKeyState(Keys::KEY_RIGHT) == KeyState::KEY_HELD)
		{

			float _rot = -1.0f * rotate_speed * deltaTime;
			glm::mat4 _rotation = glm::identity<glm::mat4>();
			_rotation = glm::rotate(_rotation, glm::radians(_rot), glm::vec3(0.0f, 1.0f, 0.0f));

			m_lookDirection = _rotation * glm::vec4(m_lookDirection, 0.0f);
			m_lookDirection = glm::normalize(m_lookDirection);
			m_upDirection = _rotation * glm::vec4(m_upDirection, 0.0f);
			m_upDirection = glm::normalize(m_upDirection);
			m_rightDirection = glm::normalize(glm::cross(m_lookDirection, m_upDirection));
			is_dirty[1] = true;

		}
		if (InputSystem::get_instance()->GetKeyState(Keys::KEY_LEFT) == KeyState::KEY_HELD)
		{

			float _rot = 1.0f * rotate_speed * deltaTime;
			glm::mat4 _rotation = glm::identity<glm::mat4>();
			_rotation = glm::rotate(_rotation, glm::radians(_rot), glm::vec3(0.0f, 1.0f, 0.0f));

			m_lookDirection = _rotation * glm::vec4(m_lookDirection, 0.0f);
			m_lookDirection = glm::normalize(m_lookDirection);
			m_upDirection = _rotation * glm::vec4(m_upDirection, 0.0f);
			m_upDirection = glm::normalize(m_upDirection);
			m_rightDirection = glm::normalize(glm::cross(m_lookDirection, m_upDirection));
			is_dirty[1] = true;

		}


	}

	void Camera::SetPosition(glm::vec3 position)
	{
		m_position = position;
		is_dirty[1] = true;
	}

	void Camera::SetLookDir(glm::vec3 look_dir)
	{
		m_lookDirection = look_dir;

		m_rightDirection = glm::normalize(glm::cross(m_lookDirection, m_upDirection));
		is_dirty[1] = true;

	}

	void Camera::SetUpDir(glm::vec3 up_dir)
	{
		m_upDirection = up_dir;

		m_rightDirection = glm::normalize(glm::cross(m_lookDirection, m_upDirection));
		is_dirty[1] = true;

	}

	void Camera::SetFov(float fov)
	{
		m_fov = fov;
		is_dirty[0] = true;

	}

	void Camera::SetNear(float z_near)
	{
		m_zNear = z_near;

		is_dirty[0] = true;

	}

	void Camera::SetFar(float z_far)
	{
		m_zFar = z_far;

		is_dirty[0] = true;

	}

	void Camera::SetAspectRatio(float aspect_ratio)
	{
		m_aspectRatio = aspect_ratio;

		is_dirty[0] = true;

	}

}