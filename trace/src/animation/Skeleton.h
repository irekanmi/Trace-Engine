#pragma once

#include "resource/Resource.h"
#include "animation/Bone.h"
#include "scene/UUID.h"

#include <string>
#include <vector>

namespace trace {

	class Scene;

	class Skeleton : public Resource
	{

	public:
		bool Create(const std::string& name, const std::string& root_node, std::vector<Bone>& bones);
		virtual void Destroy() override;

		std::string& GetSkeletonName() { return m_name; }
		std::vector<Bone>& GetBones() { return m_bones; }
		std::vector<UUID> GetEntites() { return m_boneEntites; }
		int32_t GetIndex(UUID idx);
		std::string& GetRootNode() { return m_rootNode; }
		StringID& GetRootNodeID() { return m_rootNodeID; }

		int32_t GetBoneIndex(const std::string& bone_name);
		int32_t GetBoneIndex(StringID bone_name);

		void SetAsRuntime(Scene* scene, UUID id);
		void GetGlobalPose(Scene* scene, std::vector<glm::mat4>& out_pose, UUID id);
		void SetRootNode(std::string& root_node);


	private:
		std::string m_name;
		std::vector<Bone> m_bones;
		std::vector<UUID> m_boneEntites;
		std::string m_rootNode = "";
		StringID m_rootNodeID;

		void get_bone_entites(Scene* scene, std::vector<UUID>& bone_entites, UUID id);

	protected:

	};

}
