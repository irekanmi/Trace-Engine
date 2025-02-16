#pragma once

#include "reflection/TypeRegistry.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

namespace trace {

	enum class CameraType
	{
		NONE = -1,
		PERSPECTIVE,
		ORTHOGRAPHIC
	};


	class Camera
	{

	public:
		Camera() {};
		Camera(CameraType cam_type);
		~Camera();

		CameraType GetCameraType() { return m_type; }
		glm::vec3 GetPosition();
		glm::vec3 GetLookDir();
		glm::vec3 GetUpDir();
		glm::mat4 GetViewMatrix();
		glm::mat4 GetProjectionMatix();

		

		float GetFov();
		float GetNear();
		float GetFar();
		float GetAspectRatio();
		float GetOrthographicSize() { return m_orthographicSize; }
		void Update(float deltaTime);


		void SetPosition(glm::vec3 position);
		void SetLookDir(glm::vec3 look_dir);
		void SetUpDir(glm::vec3 up_dir);

		void SetFov(float fov);
		void SetNear(float z_near);
		void SetFar(float z_far);
		void SetAspectRatio(float aspect_ratio);
		void SetCameraType(CameraType type) { m_type = type; }

		void SetOrthographicSize(float ortho_size) { m_orthographicSize = ortho_size; is_dirty[0] = true; }

	private:
		glm::mat4 m_projection = glm::identity<glm::mat4>();
		glm::mat4 m_view = glm::identity<glm::mat4>();

		float m_fov = 45.0f;
		float m_zNear = 0.1f;
		float m_zFar = 1000.0f;
		float m_aspectRatio = (16.0f / 9.0f);

		float m_orthographicSize = 10.0f;
		


		glm::vec3 m_position = glm::vec3(0.0f, .0f, 1.0f);
		glm::vec3 m_lookDirection = glm::vec3(0.0f, .0f, -1.0f);
		glm::vec3 m_upDirection = glm::vec3(0.0f, 1.0f, 0.0f);
		glm::vec3 m_rightDirection = glm::vec3(-1.0f, .0f, 0.0f);

		bool is_dirty[2] = {0};

		CameraType m_type = CameraType::PERSPECTIVE;


	protected:
		ACCESS_CLASS_MEMBERS(Camera);

	};

}
