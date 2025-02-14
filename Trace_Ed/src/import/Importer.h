#pragma once

#include "render/GTexture.h"
#include "resource/Ref.h"
#include "render/Model.h"
#include "render/SkinnedModel.h"
#include "animation/Skeleton.h"

#include <string>
#include <vector>
#include <unordered_map>
#include "assimp/Importer.hpp"
#include "assimp/scene.h"

namespace trace {

	class Importer
	{

	public:

		bool ImportMeshFile(const std::string& file_path);
		Ref<GTexture> LoadTexture(const std::string& file_path);
		Ref<Model> LoadModel(const std::string& file_path);
		Ref<SkinnedModel> LoadSkinnedModel(const std::string& file_path);
		std::unordered_map<std::string, std::vector<Ref<Animation::Skeleton>>>& GetImportedSkeletons() { return m_loadedSkeleton; }

		std::unordered_map<std::string,const aiScene*>& GetLoadedImports() { return m_loadedFiles; }
		
	private:
		std::unordered_map<std::string, const aiScene*> m_loadedFiles;
		std::unordered_map<std::string, std::vector<Ref<Animation::Skeleton>>> m_loadedSkeleton;
		Assimp::Importer m_importer;

	protected:

	};

}