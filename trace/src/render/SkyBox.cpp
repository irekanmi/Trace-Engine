#include "pch.h"

#include "SkyBox.h"
#include "resource/MeshManager.h"

namespace trace{



	SkyBox::SkyBox()
	{
	}

	SkyBox::SkyBox(Texture_Ref cube_map)
	{
		Cube = MeshManager::get_instance()->GetDefault("Cube");
		CubeMap = cube_map;
	}

	SkyBox::~SkyBox()
	{
	}

}