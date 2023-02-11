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
		virtual ~Camera() {};

		CameraType GetCameraType() { return m_type; }
		virtual glm::vec3 GetPosition() = 0;
		virtual glm::vec3 GetLookDir() = 0;
		virtual glm::vec3 GetUpDir() = 0;
		virtual glm::mat4 GetViewMatrix() = 0;
		virtual glm::mat4 GetProjectionMatix() = 0;

		

		// TODO: Speculating maybe it should not be in the base class
		virtual float GetFov() = 0;
		virtual float GetNear() = 0;
		virtual float GetFar() = 0;
		virtual float GetAspectRatio() = 0;
		virtual void Update(float deltaTime) = 0;


		virtual void SetPosition(glm::vec3 position) = 0;
		virtual void SetLookDir(glm::vec3 look_dir) = 0;
		virtual void SetUpDir(glm::vec3 up_dir) = 0;

		// TODO: Speculating maybe it should not be in the base class
		virtual void SetFov(float fov) = 0;
		virtual void SetNear(float z_near) = 0;
		virtual void SetFar(float z_far) = 0;
		virtual void SetAspectRatio(float aspect_ratio) = 0;

	private:

	protected:
		CameraType m_type = CameraType::NONE;


	};

}
