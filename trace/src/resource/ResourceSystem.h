#pragma once

#include "core/io/Logging.h"
#include "Ref.h"
#include "render/Graphics.h"




namespace trace {

	class TextureManager;
	class ModelManager;
	class MeshManager;
	class PipelineManager;
	class MaterialManager;
	class ShaderManager;
	class Mesh;
	class GTexture;
	class GPipeline;
	class GShader;
	class MaterialInstance;
	struct Material;
	using Texture_Ref = Ref<GTexture>;

	class ResourceSystem
	{

	public:
		ResourceSystem();
		~ResourceSystem();

		bool Init();
		void ShutDown();

		// Textures
		Texture_Ref GetDefaultTexture(const std::string& name);
		void CreateTexture(const std::string& name, TextureDesc desc);
		Texture_Ref LoadTexture(const std::string& name, TextureDesc desc);
		Texture_Ref LoadTexture(const std::vector<std::string>& filenames, TextureDesc desc, const std::string& name);
		Texture_Ref LoadTexture(const std::string& name);
		Texture_Ref GetTexture(const std::string& name);
		void ReleaseTexture(const std::string& name);

		// Meshes
		Ref<Mesh> LoadMesh(const std::string& name);
		Ref<Mesh> GetDefaultMesh(const std::string& name);
		Ref<Mesh> GetMesh(const std::string& name);
		

		//Pipelines
		bool CreatePipeline(PipelineStateDesc desc, const std::string& name, bool auto_fill = true);
		Ref<GPipeline> GetPipeline(const std::string& name);
		Ref<GPipeline> GetDefaultPipeline(const std::string& name);

		//Materials
		bool CreateMaterial(const std::string& name, Material material, Ref<GPipeline> pipeline);
		Ref<MaterialInstance> GetMaterial(const std::string& name);
		bool LoadDefaults();

		// Shaders
		Ref<GShader> CreateShader(const std::string& name, ShaderStage shader_stage);
		Ref<GShader> GetShader(const std::string& name);
		std::string GetShaderResourcePath();

		static ResourceSystem* get_instance();

	private:
		static ResourceSystem* s_instance;

	private:
		TextureManager* m_textureManager;
		ModelManager* m_modelManager;
		MeshManager* m_meshManager;
		PipelineManager* m_pipelineManager;
		MaterialManager* m_materialManager;
		ShaderManager* m_shaderManager;

	protected:

	};

}

