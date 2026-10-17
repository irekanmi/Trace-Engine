#include <pch.h>

#include "TwoBoneIK.h"
#include "scene/Scene.h"
#include "scene/Entity.h"

#include "glm/gtx/norm.hpp"

namespace trace::Animation {




	void TwoBoneIK::Solve(TwoBoneIKData& ik_data, std::array<glm::quat, 2>& out_orientation)
	{

		Transform r_pose(ik_data.anim_pose->GetBoneGlobalPose(ik_data.root_bone_index));
		Transform m_pose(ik_data.anim_pose->GetBoneGlobalPose(ik_data.mid_bone_index));
		Transform e_pose(ik_data.anim_pose->GetBoneGlobalPose(ik_data.end_bone_index));

		Transform r_local_pose(ik_data.anim_pose->GetBoneLocalPose(ik_data.root_bone_index));
		Transform m_local_pose(ik_data.anim_pose->GetBoneLocalPose(ik_data.mid_bone_index));
		Transform e_local_pose(ik_data.anim_pose->GetBoneLocalPose(ik_data.end_bone_index));

		Skeleton* skeleton = ik_data.anim_pose->GetSkeletonInstance()->GetSkeleton().get();
		Bone* root_bone = skeleton->GetBone(ik_data.root_bone_index);

		Transform parent_transform = root_bone->HasParent() ? Transform(ik_data.anim_pose->GetBoneGlobalPose(root_bone->GetParentIndex())) : Transform::Identity();

		glm::vec3 target_dir = ik_data.target_position - r_pose.GetPosition();
		float target_len = glm::length(target_dir);
		target_dir = glm::normalize(target_dir);

		float upper_len = glm::length(m_pose.GetPosition() - r_pose.GetPosition());
		float lower_len = glm::length(e_pose.GetPosition() - m_pose.GetPosition());

		float min_len = glm::abs(upper_len - lower_len);
		float max_len = glm::abs(upper_len + lower_len - 0.01f);
		target_len = glm::clamp(target_len, min_len, max_len);

		float A = lower_len;
		float B = upper_len;
		float C = target_len;

		//Cosine rule
		// A^2 = B^2 + C^2 - 2AB*cos(Ø)
		// cos(Ø) = ( B^2 + C^2 - A^2) / 2BC

		float angle = ((C * C) + (B * B) - (A * A)) / (2 * B * C);
		angle = glm::clamp(angle, -1.0f, 1.0f);
		angle = glm::acos(angle);

		//Determine bend direction using pole vector

		glm::vec3 pole_dir;

		glm::vec3 to_end = glm::normalize(e_pose.GetPosition() - r_pose.GetPosition());
		glm::vec3 to_mid = glm::normalize(m_pose.GetPosition() - r_pose.GetPosition());

		float _dot_prod = glm::dot(to_mid, to_end);
		if (_dot_prod == 1.0f || ik_data.use_hint)
		{
			pole_dir = ik_data.pole_hint - r_pose.GetPosition();
			pole_dir = glm::normalize(pole_dir);
		}
		else
		{
			pole_dir = to_end * _dot_prod;
			pole_dir = glm::normalize(to_mid - pole_dir);
		}

		glm::vec3 plane_normal = glm::normalize(glm::cross(target_dir, pole_dir));
		glm::vec3 bend_direction = glm::normalize(glm::cross(plane_normal, target_dir));

		glm::vec3 final_mid_pos = r_pose.GetPosition() + (target_dir * glm::cos(angle) * upper_len) + (bend_direction * glm::sin(angle) * upper_len);

		glm::vec3 current_upper_dir = glm::normalize(m_pose.GetPosition() - r_pose.GetPosition());
		glm::vec3 desired_upper_dir = glm::normalize(final_mid_pos - r_pose.GetPosition());

		glm::quat upper_rotation = glm::rotation(current_upper_dir, desired_upper_dir);
		glm::quat root_rotation = glm::normalize(upper_rotation * r_pose.GetRotation());
		
		out_orientation[0] = root_bone->HasParent() ? parent_transform.Inverse().GetRotation() * root_rotation : root_rotation;

		glm::vec3 new_mid = root_rotation * m_local_pose.GetPosition();
		new_mid += r_pose.GetPosition();
		glm::quat new_mid_rot = root_rotation * m_local_pose.GetRotation();
		glm::vec3 new_end = new_mid_rot * e_local_pose.GetPosition();
		new_end += new_mid;

		glm::vec3 current_lower_dir = glm::normalize(new_end - new_mid);
		glm::vec3 desired_lower_dir = glm::normalize(ik_data.target_position - new_mid);

		glm::quat lower_rotation = glm::rotation(current_lower_dir, desired_lower_dir);
		glm::quat mid_rot = glm::normalize(lower_rotation * new_mid_rot);

		glm::quat inv_rot = parent_transform.GetRotation() * root_rotation;
		inv_rot = glm::normalize(glm::inverse(inv_rot));

		out_orientation[1] = glm::normalize(inv_rot * mid_rot);


	}


}