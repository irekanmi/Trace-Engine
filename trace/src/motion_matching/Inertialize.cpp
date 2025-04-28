#include "pch.h"

#include "motion_matching/Inertialize.h"
#include "scene/Entity.h"
#include "scene/Scene.h"

#include "orange_duck/spring.h"
#include "core/maths/Conversion.h"

namespace trace::MotionMatching {

    

    static inline glm::vec3 get_angular_rotation(glm::quat& val)
    {
        glm::vec3 axis = glm::axis(val);
        float angle = glm::angle(val);

        return axis * angle;
    }

    bool Inertialize::Initialize(Animation::Skeleton* skeleton)
    {
        int32_t num_bones = static_cast<int32_t>(skeleton->GetBones().size());

        if (m_offsetAngularVelocity.empty())
        {
            m_offsetAngularVelocity.resize(num_bones);
            m_offsetPosition.resize(num_bones);
            m_offsetRotation.resize(num_bones);
            m_offsetVelocity.resize(num_bones);
        }
        return true;
    }

    bool Inertialize::Init(Animation::Pose* source_prev_pose, Animation::Pose* source_pose, Animation::Pose* target_prev_pose, Animation::Pose* target_pose, Animation::Skeleton* skeleton, float animation_dt)
    {


        Transform& src_root_data = source_pose->GetRootMotionDelta();
        Transform& src_prev_root_data = source_prev_pose->GetRootMotionDelta();
        Transform& trg_root_data = target_pose->GetRootMotionDelta();

        int32_t num_bones = static_cast<int32_t>(skeleton->GetBones().size());

        if (m_offsetAngularVelocity.empty())
        {
            m_offsetAngularVelocity.resize(num_bones);
            m_offsetPosition.resize(num_bones);
            m_offsetRotation.resize(num_bones);
            m_offsetVelocity.resize(num_bones);
        }

        for (int32_t i = 0; i < num_bones; i++)
        {
            Transform& src_data = source_pose->GetLocalPose()[i];
            Transform& src_prev_data = source_prev_pose->GetLocalPose()[i];
            Transform& trg_data = target_pose->GetLocalPose()[i];
            Transform& trg_prev_data = target_prev_pose->GetLocalPose()[i];

            glm::vec3 src_velocity = (src_data.GetPosition() - src_prev_data.GetPosition()) / animation_dt;
            glm::vec3 trg_velocity = (trg_data.GetPosition() - trg_prev_data.GetPosition()) / animation_dt;

            glm::quat src_delta_rot = src_data.GetRotation() * glm::inverse(src_prev_data.GetRotation());
            glm::vec3 src_angular_vel = get_angular_rotation(src_delta_rot) / animation_dt;
            
            glm::quat trg_delta_rot = trg_data.GetRotation() * glm::inverse(trg_prev_data.GetRotation());
            glm::vec3 trg_angular_vel = get_angular_rotation(trg_delta_rot) / animation_dt;

            orange_duck::inertialize_transition(
                m_offsetRotation[i],
                m_offsetAngularVelocity[i],
                glm_quat_to_org(src_data.GetRotation()),
                glm_vec_to_org(src_angular_vel),
                glm_quat_to_org(trg_data.GetRotation()),
                glm_vec_to_org(trg_angular_vel)
            );

            orange_duck::inertialize_transition(
                m_offsetPosition[i],
                m_offsetVelocity[i],
                glm_vec_to_org(src_data.GetPosition()),
                glm_vec_to_org(src_velocity),
                glm_vec_to_org(trg_data.GetPosition()),
                glm_vec_to_org(trg_velocity)
            );

        }

        return true;
    }

    bool Inertialize::Update(float deltaTime, Animation::Pose* out_pose, Animation::Pose* target_pose, Animation::Skeleton* skeleton)
    {

        Transform& root_data = out_pose->GetRootMotionDelta();


        int32_t num_bones = static_cast<int32_t>(skeleton->GetBones().size());
        float half_life = 0.1f;

        for (int32_t i = 0; i < num_bones; i++)
        {
            Transform& _data = out_pose->GetLocalPose()[i];
            Transform& trg_data = target_pose->GetLocalPose()[i];
            
            orange_duck::quat out_rot = {};
            orange_duck::vec3 out_ang_vel = {};
            orange_duck::inertialize_update(
                out_rot,
                out_ang_vel,
                m_offsetRotation[i],
                m_offsetAngularVelocity[i],
                glm_quat_to_org(trg_data.GetRotation()),
                orange_duck::vec3(),
                half_life,
                deltaTime
            );
            _data.SetRotation(org_quat_to_glm(out_rot));

            orange_duck::vec3 out_pos = {};
            orange_duck::vec3 out_vel = {};
            orange_duck::inertialize_update(
                out_pos,
                out_vel,
                m_offsetPosition[i],
                m_offsetVelocity[i],
                glm_vec_to_org(trg_data.GetPosition()),
                orange_duck::vec3(),
                half_life,
                deltaTime
            );
            _data.SetPosition(org_vec_to_glm(out_pos));
            

        }

        return true;
    }


}