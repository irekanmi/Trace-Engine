#pragma once

#include "Camera.h"

namespace trace {

	class TRACE_API PerspectiveCamera : public Camera
	{

	public:
		PerspectiveCamera();
		PerspectiveCamera(glm::vec3 position, glm::vec3 look_dir, glm::vec3 up_dir, float aspect_ratio, float fov, float z_near, float z_far);
		~PerspectiveCamera();

		virtual glm::vec3 GetPosition() override;
		virtual glm::vec3 GetLookDir() override;
		virtual glm::vec3 GetUpDir() override;
		virtual glm::mat4 GetViewMatrix() override;
		virtual glm::mat4 GetProjectionMatix() override;

		virtual float GetFov() override;
		virtual float GetNear() override;
		virtual float GetFar() override;
		virtual float GetAspectRatio() override;
		virtual void Update(float deltaTime) override;


		virtual void SetPosition(glm::vec3 position) override;
		virtual void SetLookDir(glm::vec3 look_dir) override;
		virtual void SetUpDir(glm::vec3 up_dir) override;

		virtual void SetFov(float fov) override;
		virtual void SetNear(float z_near) override;
		virtual void SetFar(float z_far) override;
		virtual void SetAspectRatio(float aspect_ratio) override;

	private:
		glm::vec3 m_position;
		glm::vec3 m_lookDirection;
		glm::vec3 m_upDirection;

		float m_fov;
		float m_zNear;
		float m_zFar;
		float m_aspectRatio;

		glm::mat4 m_projection;


	protected:

	};

}
