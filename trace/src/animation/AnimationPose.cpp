#include "pch.h"

#include "animation/AnimationPose.h"
#include "core/io/Logging.h"
#include "core/Application.h"

#include "scene/Scene.h"
#include "scene/Entity.h"

namespace trace::Animation {
	bool Pose::Init(SkeletonInstance* skeleton)
	{

		if (!skeleton)
		{
			TRC_ERROR("Invalid Skeleton Instance. Function: {}", __FUNCTION__);
			return false;
		}

		if (!skeleton->GetSkeleton())
		{
			TRC_ERROR("Invalid Skeleton Handle. Function: {}", __FUNCTION__);
			return false;
		}

		m_skeleton = skeleton;

		std::vector<UUID>& entities = skeleton->GetEntites();
		m_localPose.clear();
		m_localPose.resize(entities.size());

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
}