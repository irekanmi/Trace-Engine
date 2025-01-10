#pragma once

#include "animation/Bone.h"
#include "scene/UUID.h"
#include "resource/Resource.h"
#include "resource/Ref.h"

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
		std::string& GetRootNode() { return m_rootNode; }
		StringID& GetRootNodeID() { return m_rootNodeID; }
		Bone* GetBone(uint32_t bone_index);

		int32_t GetBoneIndex(const std::string& bone_name);
		int32_t GetBoneIndex(StringID bone_name);

		void SetRootNode(std::string& root_node);

	public:
		static Ref<Skeleton> Deserialize(const std::string& file_path);

	private:
		std::string m_name;
		std::vector<Bone> m_bones;
		std::string m_rootNode = "";
		StringID m_rootNodeID;

		

	protected:

	};

	class SkeletonInstance
	{

	public:

		bool CreateInstance(Ref<Skeleton> skeleton, Scene* scene, UUID id);
		void GetGlobalPose(std::vector<glm::mat4>& out_pose, UUID id);
		int32_t GetIndex(UUID idx);
		std::vector<UUID>& GetEntites() { return m_boneEntites; }
		Ref<Skeleton> GetSkeleton() { return m_skeleton; }
		void SetSkeleton(Ref<Skeleton> skeleton) { m_skeleton = skeleton; }
		Scene* GetScene() { return m_scene; }

	private:
		void get_bone_entites(Scene* scene, std::vector<UUID>& bone_entites, UUID id);

		std::vector<UUID> m_boneEntites;
		Ref<Skeleton> m_skeleton;
		Scene* m_scene = nullptr;
	protected:

	};

}
