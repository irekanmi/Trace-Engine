#include "pch.h"

#include "SkyBox.h"
#include "resource/ResourceSystem.h"

namespace trace{



	SkyBox::SkyBox()
	{
	}

	SkyBox::SkyBox(Texture_Ref cube_map)
	{
		Cube = ResourceSystem::get_instance()->GetDefaultMesh("Cube");
		CubeMap = cube_map;
	}

	SkyBox::~SkyBox()
	{
	}

}