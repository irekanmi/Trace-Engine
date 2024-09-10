#include "pch.h"

#include "Components.h"
#include "Entity.h"
#include "scene/Scene.h"
#include "animation/AnimationGraph.h"

namespace trace {



    bool AnimationComponent::InitializeEntities(Scene* scene, UUID parent, bool refresh)
    {
        if (!entities.empty() && !refresh)
        {
            return false;
        }

        if (!anim_graph)
        {
            return false;
        }

        if (refresh)
        {
            entities.clear();
        }

        std::vector<AnimationState>& states = anim_graph->GetStates();

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
                    entities[parent_entity.GetComponent<TagComponent>()._tag] = parent_entity.GetID();
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

}