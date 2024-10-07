#include "pch.h"

#include "animation/Skeleton.h"
#include "core/io/Logging.h"
#include "scene/Entity.h"
#include "scene/Scene.h"

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
		m_rootNodeID = std::hash<std::string>{}(root_node);

		return true;
	}
	void Skeleton::Destroy()
	{
	}
	int32_t Skeleton::GetIndex(UUID idx)
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
	int32_t Skeleton::GetBoneIndex(const std::string& bone_name)
	{
		return GetBoneIndex(std::hash<std::string>{}(bone_name));
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
		return -2;
	}
	void Skeleton::SetAsRuntime(Scene* scene, UUID id)
	{
		get_bone_entites(scene, m_boneEntites, id);

	}
	void Skeleton::GetGlobalPose(Scene* scene, std::vector<glm::mat4>& out_pose, UUID id)
	{
		out_pose.clear();
		out_pose.resize(m_bones.size());

		if (!m_boneEntites.empty())
		{
			for (uint32_t i = 0; i < m_bones.size(); i++)
			{
				Bone& bone = m_bones[i];
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

			for (uint32_t i = 0; i < m_bones.size(); i++)
			{
				Bone& bone = m_bones[i];
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
	void Skeleton::SetRootNode(std::string& root_node)
	{
		if (root_node.empty())
		{
			return;
		}

		m_rootNode = root_node;
		m_rootNodeID = std::hash<std::string>{}(root_node);

	}
	void Skeleton::get_bone_entites(Scene* scene, std::vector<UUID>& bone_entites, UUID id)
	{
		Entity obj = scene->GetEntity(id);
		if (!obj)
		{
			return;
		}

		Entity root_node = scene->GetParentByName(obj, m_rootNodeID);

		if (!root_node)
		{
			return;
		}

		bone_entites.resize(m_bones.size());

		for (uint32_t i = 0; i < m_bones.size(); i++)
		{
			Bone& bone = m_bones[i];

			Entity entity = scene->GetChildEntityByName(root_node, bone.GetStringID());
			bone_entites[i] = entity.GetID();
		}
	}
}