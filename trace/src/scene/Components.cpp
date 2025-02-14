#include "pch.h"

#include "Entity.h"
#include "scene/Scene.h"
#include "Components.h"
#include "animation/AnimationGraph.h"
#include "core/Utils.h"
#include "animation/AnimationEngine.h"


namespace trace {



    bool AnimationComponent::InitializeEntities(Scene* scene)
    {
        if (!entities.empty())
        {
            return false;
        }

        if (!animation)
        {
            TRC_WARN("Ensure to include an animation before using component");
            return false;
        }

        for (auto& track : animation->GetTracks())
        {
            Entity entity;
            switch (animation->GetType())
            {
            case AnimationClipType::SEQUENCE:
            {
                entity = scene->GetEntityByName(track.first);
                break;
            }

            }

            if (entity)
            {
                entities[track.first] = entity.GetID();
            }
        }

        return true;
    }

    void AnimationComponent::Start()
    {
        started = true;
    }

    void AnimationComponent::Stop()
    {
        started = false;
        elasped_time = 0.0f;
    }

    void AnimationComponent::Pause()
    {
        started = false;
    }

    void AnimationComponent::Update(float deltaTime, Scene* scene)
    {
        if (!started)
        {
            return;
        }

        AnimationEngine::get_instance()->Animate(this, scene, elasped_time, loop);

        elasped_time += deltaTime;
    }



    void TagComponent::SetTag(const std::string& name)
    {
        m_tag = name;
        m_id = STR_ID(name);
    }

    void SkinnedModelRenderer::SetSkeleton(Ref<Animation::Skeleton> skeleton, Scene* scene, UUID id)
    {
        if (!skeleton)
        {
            return;
        }

        runtime_skeleton.CreateInstance(skeleton, scene, id);
    }

}