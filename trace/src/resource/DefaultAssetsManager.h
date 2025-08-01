#pragma once

#include "resource/Ref.h"
#include "render/Material.h"
#include "render/GPipeline.h"
#include "render/GTexture.h"
#include "render/GShader.h"
#include "render/Model.h"

#include <string>

namespace trace {

	class DefaultAssetsManager
	{

	public:

		static bool LoadAssets();
		static bool LoadAssets_Runtime();
		static void LoadAssetsPath();
		static void ReleaseAssets();
		static void BuildDefaults();

	public:
		static Ref<MaterialInstance> default_material;
		static Ref<Model> Cube;
		static Ref<Model> Sphere;
		static Ref<Model> Plane;
		static Ref<GPipeline> skybox_pipeline;
		static Ref<GPipeline> light_pipeline;
		static Ref<GPipeline> gbuffer_pipeline;
		static Ref<GPipeline> skinned_gbuffer_pipeline;
		static Ref<GPipeline> text_batch_pipeline;
		static Ref<GPipeline> text_pipeline;
		static Ref<GPipeline> quad_pipeline;
		static Ref<GPipeline> particle_billboard_pipeline;
		static Ref<GPipeline> particle_velocity_aligned_pipeline;
		static Ref<GPipeline> debug_line_pipeline;
		static Texture_Ref default_diffuse_map;
		static Texture_Ref default_specular_map;
		static Texture_Ref default_normal_map;
		static Texture_Ref black_texture;
		static Texture_Ref transparent_texture;

		//NOTE: Used only with the editor
		static std::string assets_path;

	protected:

	};
}
