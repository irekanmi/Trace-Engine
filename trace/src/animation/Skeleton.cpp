#include "pch.h"

#include "animation/Skeleton.h"
#include "core/io/Logging.h"
#include "scene/Entity.h"
#include "scene/Scene.h"
#include "serialize/AnimationsSerializer.h"
#include "core/Utils.h"

namespace trace {
	bool Skeleton::Create(const std::string& name, const std::string& root_node, std::vector<Bone>& bones)
	{
		if (!m_name.empty())
		{
			TRC_WARN("Skeleton has already been created. Actual name: {}, New name: {}", m_name, name);
			return false;
		}

		m_name = name;
		m_bones = bones;


		m_rootNode = root_node;
		m_rootNodeID = STR_ID(root_node);

		return true;
	}
	void Skeleton::Destroy()
	{
	}
	Bone* Skeleton::GetBone(uint32_t bone_index)
	{
		if (m_bones.empty())
		{
			return nullptr;
		}
		if (bone_index > m_bones.size())
		{
			return nullptr;
		}

		return &m_bones[bone_index];
	}
	int32_t Skeleton::GetBoneIndex(const std::string& bone_name)
	{
		return GetBoneIndex(STR_ID(bone_name));
	}
	int32_t Skeleton::GetBoneIndex(StringID bone_name)
	{
		int32_t i = 0;
		for (Bone& bone : m_bones)
		{

			if (bone.GetStringID() == bone_name)
			{
				return i;
			}

			i++;
		}
		return -1;
	}

	void Skeleton::SetRootNode(std::string& root_node)
	{
		if (root_node.empty())
		{
			return;
		}

		m_rootNode = root_node;
		m_rootNodeID = STR_ID(root_node);

	}

	Ref<Skeleton> Skeleton::Deserialize(const std::string& file_path)
	{
		return AnimationsSerializer::DeserializeSkeleton(file_path);
	}


	void SkeletonInstance::GetGlobalPose(std::vector<glm::mat4>& out_pose, UUID id)
	{
		std::vector<Bone>& bones = m_skeleton->GetBones();

		Scene* scene = m_scene;
		out_pose.clear();
		out_pose.resize(bones.size());

		if (!m_boneEntites.empty())
		{
			for (uint32_t i = 0; i < bones.size(); i++)
			{
				Bone& bone = bones[i];
				Entity obj = scene->GetEntity(m_boneEntites[i]);
				if (!obj)
				{
					continue;
				}
				glm::mat4 final_transform = obj.GetComponent<HierachyComponent>().transform;
				out_pose[i] = final_transform * bone.GetBoneOffset();
			}
		}
		else
		{
			std::vector<UUID> bone_entites;
			get_bone_entites(scene, bone_entites, id);

			for (uint32_t i = 0; i < bones.size(); i++)
			{
				Bone& bone = bones[i];
				Entity obj = scene->GetEntity(bone_entites[i]);
				if (!obj)
				{
					continue;
				}
				glm::mat4 final_transform = obj.GetComponent<HierachyComponent>().transform;
				out_pose[i] = final_transform * bone.GetBoneOffset();
			}
		}
	}
	int32_t SkeletonInstance::GetIndex(UUID idx)
	{
		int32_t i = 0;
		for (UUID& id : m_boneEntites)
		{
			if (id == idx)
			{
				return i;
			}

			i++;
		}
		return -1;
	}
	void SkeletonInstance::get_bone_entites(Scene* scene, std::vector<UUID>& bone_entites, UUID id)
	{

		Entity obj = scene->GetEntity(id);
		if (!obj)
		{
			return;
		}

		std::vector<Bone>& bones = m_skeleton->GetBones();
		
		Bone& root_bone = bones[0];

		Entity root_entity = scene->GetChildEntityByName(obj, root_bone.GetStringID());

		if (root_entity)// found root bone in child
		{
			bone_entites.resize(bones.size());
			bone_entites[0] = root_entity.GetID();
			for (uint32_t i = 1; i < bones.size(); i++)
			{
				Bone& bone = bones[i];

				Entity entity = scene->GetChildEntityByName(root_entity, bone.GetStringID());
				bone_entites[i] = entity.GetID();
			}
		}
		else // try and find in parent nodes
		{
			root_entity = scene->FindEnityInHierachy(obj, root_bone.GetStringID());
			if (!root_entity)
			{
				return;
			}

			bone_entites.resize(bones.size());
			bone_entites[0] = root_entity.GetID();
			for (uint32_t i = 1; i < bones.size(); i++)
			{
				Bone& bone = bones[i];

				Entity entity = scene->GetChildEntityByName(root_entity, bone.GetStringID());
				bone_entites[i] = entity.GetID();
			}
		}
		
	}


	bool SkeletonInstance::CreateInstance(Ref<Skeleton> skeleton, Scene* scene, UUID id)
	{
		if (!m_skeleton && !skeleton)
		{
			TRC_ERROR("Invalid skeleton handle");
			return false;
		}

		if (!scene)
		{
			TRC_ERROR("Invalid scene handle");
			return false;
		}

		m_skeleton = m_skeleton ? m_skeleton : skeleton;
		m_scene = scene;

		get_bone_entites(scene, m_boneEntites, id);
		
		return true;
	}
}