#pragma once

#include "core/Enums.h"
#include "core/Core.h"
#include "resource/ResourceSystem.h"

namespace trace {

	struct Material
	{
		glm::vec4 m_diffuseColor;
		float m_shininess;
		Texture_Ref m_albedoMap;
		Texture_Ref m_specularMap;
		Texture_Ref m_normalMap;
	};
}
