#include "pch.h"

#include "Entity.h"
#include "scene/Scene.h"
#include "Components.h"
#include "animation/AnimationGraph.h"


namespace trace {



    bool AnimationComponent::InitializeEntities(Scene* scene, UUID parent, bool refresh)
    {
        if (!entities.empty() && !refresh)
        {
            return false;
        }

        if (!m_animGraph)
        {
            return false;
        }

        if (refresh)
        {
            entities.clear();
        }

        std::vector<AnimationState>& states = m_animGraph->GetStates();

        for (AnimationState& state : states)
        {

            Ref<AnimationClip> anim_clip = state.GetAnimationClip();
            if (!anim_clip)
            {
                continue;
            }
            Entity parent_entity = scene->GetEntity(parent);

            switch (anim_clip->GetType())
            {
            case AnimationClipType::SKELETAL_ANIMATIOM:
            {

                if (parent_entity)
                {
                    entities[parent_entity.GetComponent<TagComponent>().GetTag()] = parent_entity.GetID();
                }
                break;
            }
            }

            for (auto& track : anim_clip->GetTracks())
            {
                Entity entity;
                switch (anim_clip->GetType())
                {
                case AnimationClipType::SEQUENCE:
                {
                    entity = scene->GetEntityByName(track.first);
                    break;
                }
                case AnimationClipType::SKELETAL_ANIMATIOM:
                {
                    
                    if (parent_entity)
                    {
                        entity = scene->GetChildEntityByName(parent_entity, track.first);
                    }
                    break;
                }
                }

                if (entity)
                {
                    entities[track.first] = entity.GetID();
                }
            }

            

        }

        return true;
    }

    void AnimationComponent::SetAnimationGraph(Ref<AnimationGraph> animation_graph)
    {
        if (!animation_graph)
        {
            return;
        }

        m_animGraph = animation_graph;
        runtime_graph = *(m_animGraph.get());
        
        runtime_graph.SetAsRuntime();
    }

    void TagComponent::SetTag(const std::string& name)
    {
        m_tag = name;
        m_id = std::hash<std::string>{}(name);
    }

    void SkinnedModelRenderer::SetSkeleton(Ref<Skeleton> skeleton, Scene* scene, UUID id)
    {
        if (!skeleton)
        {
            return;
        }

        m_skeleton = skeleton;
        runtime_skeleton = *(skeleton.get());
        runtime_skeleton.SetAsRuntime(scene, id);
    }

}