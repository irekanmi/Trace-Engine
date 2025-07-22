#include "pch.h"

#include "SkyBox.h"
#include "resource/DefaultAssetsManager.h"

namespace trace{



	SkyBox::SkyBox()
	{
	}

	SkyBox::SkyBox(Texture_Ref cube_map)
	{
		//Cube = DefaultAssetsManager::Cube;
		CubeMap = cube_map;
	}

	SkyBox::~SkyBox()
	{
	}

}