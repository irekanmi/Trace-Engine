#include "pch.h"

#include "animation/AnimationPose.h"
#include "core/io/Logging.h"
#include "core/Application.h"

#include "scene/Scene.h"
#include "scene/Entity.h"

namespace trace::Animation {
	bool Pose::Init(SkeletonInstance* skeleton_instance)
	{

		if (!skeleton_instance)
		{
			TRC_ERROR("Invalid Skeleton Instance. Function: {}", __FUNCTION__);
			return false;
		}

		if (!skeleton_instance->GetSkeleton())
		{
			TRC_ERROR("Invalid Skeleton Handle. Function: {}", __FUNCTION__);
			return false;
		}

		m_skeleton = skeleton_instance;

		std::vector<UUID>& entities = skeleton_instance->GetEntites();
		m_localPose.clear();
		m_localPose.resize(entities.size());

		return true;
	}
	bool Pose::Init(Ref<Skeleton> skeleton)
	{
		if (!skeleton)
		{
			TRC_ERROR("Invalid Skeleton Handle. Function: {}", __FUNCTION__);
			return false;
		}

		m_localPose.clear();
		m_localPose.resize(skeleton->GetBones().size());

		return true;
	}
	std::vector<Transform>& Pose::GetGlobalPose()
	{
		std::vector<UUID>& entities = m_skeleton->GetEntites();
		Scene* scene = m_skeleton->GetScene();

		if ( m_updateID == Application::get_instance()->GetUpdateID() || m_globalPose.size() == entities.size())
		{
			return m_globalPose;
		}

		m_globalPose.clear();
		m_globalPose.resize(entities.size());

		uint32_t i = 0;
		for (UUID& uuid : entities)
		{
			Entity object = scene->GetEntity(uuid);
			if (!object)
			{
				continue;
			}
			object.GetComponent<TransformComponent>()._transform = m_localPose[i];
			m_globalPose[i] = scene->GetEntityGlobalPose(object, true);

			i++;
		}

		m_updateID = Application::get_instance()->GetUpdateID();

		return m_globalPose;
	}
	void Pose::SetEntityLocalPose()
	{
		std::vector<UUID>& entities = m_skeleton->GetEntites();
		Scene* scene = m_skeleton->GetScene();

		uint32_t i = 0;
		for (UUID& uuid : entities)
		{
			Entity object = scene->GetEntity(uuid);
			if (!object)
			{
				continue;
			}
			object.GetComponent<TransformComponent>()._transform.SetPosition(m_localPose[i].GetPosition());
			object.GetComponent<TransformComponent>()._transform.SetRotation(m_localPose[i].GetRotation());
			object.GetComponent<TransformComponent>()._transform.SetScale(m_localPose[i].GetScale());

			i++;
		}

	}
	glm::mat4 Pose::GetBoneGlobalPose(int32_t bone_index)
	{
		if (!m_skeleton)
		{
			TRC_ERROR("Invalid skeleton instance, Function: {}", __FUNCTION__);
			return glm::mat4(1.0f);
		}

		Ref<Skeleton> skeleton = m_skeleton->GetSkeleton();

		return GetBoneGlobalPose(skeleton, bone_index);
	}
	glm::mat4 Pose::GetBoneGlobalPose(Ref<Skeleton> skeleton, int32_t bone_index)
	{
		if (m_globalPose.empty())
		{
			m_globalPose.resize(skeleton->GetBones().size());
		}

		if (bone_index < 0 || bone_index > skeleton->GetBones().size())
		{
			TRC_ERROR("Invalid bone index, Function: {}", __FUNCTION__);
			return glm::mat4(1.0f);
		}

		Bone& bone = skeleton->GetBones()[bone_index];
		if (bone.GetParentIndex() != -1)
		{
			glm::mat4 result = GetBoneGlobalPose(skeleton, bone.GetParentIndex()) * GetBoneLocalPose(bone_index);
			m_globalPose[bone_index] = Transform(result);

			return result;
		}

		return GetBoneLocalPose(bone_index);
	}
	glm::mat4 Pose::GetBoneLocalPose(int32_t bone_index)
	{
		if (bone_index < 0 || bone_index > m_localPose.size())
		{
			TRC_ERROR("Invalid bone index, Function: {}", __FUNCTION__);
			return glm::mat4(1.0f);
		}
		return m_localPose[bone_index].GetLocalMatrix();
	}
}