#pragma once

#include "core/Core.h"
#include "core/Enums.h"
#include "glm/glm.hpp"

namespace trace {

	enum class CameraType
	{
		NONE,
		PERSPECTIVE,
		ORTHOGRAPHIC
	};


	class TRACE_API Camera
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
		void Update(float deltaTime);


		void SetPosition(glm::vec3 position);
		void SetLookDir(glm::vec3 look_dir);
		void SetUpDir(glm::vec3 up_dir);

		void SetFov(float fov);
		void SetNear(float z_near);
		void SetFar(float z_far);
		void SetAspectRatio(float aspect_ratio);

	private:
		glm::mat4 m_projection;
		glm::mat4 m_view;

		float m_fov;
		float m_zNear;
		float m_zFar;
		float m_aspectRatio;

		glm::vec3 m_position;
		glm::vec3 m_lookDirection;
		glm::vec3 m_upDirection;
		glm::vec3 m_rightDirection;

		bool is_dirty[2] = {0};

		CameraType m_type = CameraType::NONE;


	protected:


	};

}
