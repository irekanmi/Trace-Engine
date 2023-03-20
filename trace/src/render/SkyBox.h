#pragma once

#include "core/Core.h"
#include "core/Enums.h"
#include "resource/Resource.h"
#include "Mesh.h"


namespace trace {

	class GTexture;

	class TRACE_API SkyBox
	{

	public:
		SkyBox();
		SkyBox(Texture_Ref cube_map);
		~SkyBox();

		Texture_Ref GetCubeMap_ref() { return CubeMap; }
		GTexture* GetCubeMap() { return CubeMap.get(); }
		Mesh* GetCube() { return Cube.get(); }
		
		void SetCubeMap(Texture_Ref cube_map) { CubeMap = cube_map; }


	private:
		Texture_Ref CubeMap;
		Ref<Mesh> Cube;

	protected:


	};

}
