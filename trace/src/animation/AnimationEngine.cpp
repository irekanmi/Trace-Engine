#include "pch.h"

#include "AnimationEngine.h"
#include "core/Application.h"
#include "scene/Scene.h"
#include "scene/Entity.h"
#include "core/Utils.h"

#include "glm/glm.hpp"



namespace trace {

	AnimationEngine* AnimationEngine::s_instance = nullptr;

	bool AnimationEngine::Init()
	{
		return true;
	}

	void AnimationEngine::Shutdown()
	{
	}

	void AnimationEngine::Animate(AnimationState& state, Scene* scene)
	{

		if (!state.IsPlaying()) return;

		Ref<AnimationClip> clip = state.GetAnimationClip();

		float _t = Application::get_instance()->GetClock().GetElapsedTime() - state.GetStartTime();

		if (_t > clip->GetDuration())
		{
			if (!state.GetLoop()) return;
			else _t = fmod(_t, clip->GetDuration());
		}


		for (AnimationTrack& track : clip->GetTracks())
		{
			AnimationFrameData* _f1 = nullptr;
			AnimationFrameData* _f2 = nullptr;

			//TODO: Find a better way to find which frames to sample -----------
			int i = 1;
			for (AnimationFrameData& data : track.channel_data)
			{
				if (_t >= data.time_point)
				{
					_f1 = &data;
					if (i < track.channel_data.size()) _f2 = &track.channel_data[i];
					if (i == track.channel_data.size() && state.GetLoop()) _f2 = &track.channel_data[0];
				}

				++i;
			}
			// -----------------------------------------------------------------
			
			if (!_f1 || !_f2) continue;

			float lerp_value = (_t - _f1->time_point) / (_f2->time_point - _f1->time_point);

			CalculateAndSetData(_f1, _f2, scene, track.entity_handle, track.channel_type, lerp_value);
		}

	}

	AnimationEngine* AnimationEngine::get_instance()
	{
		if (!s_instance)
		{
			s_instance = new AnimationEngine;
		}
		return s_instance;
	}

	void AnimationEngine::CalculateAndSetData(AnimationFrameData* a, AnimationFrameData* b, Scene* scene, UUID id, AnimationDataType type, float t)
	{

		Entity entity = scene->GetEntity(id);

		if (!entity)
		{
			TRC_ERROR("Invalid entity: File->{}, Line->{}", __FILE__, __LINE__);
			return;
		}

		switch (type)
		{
		case AnimationDataType::NONE:
		{
		break;
		}
		case AnimationDataType::POSITION:
		{
			glm::vec3& _a = *(glm::vec3*)(&a->data);
			glm::vec3& _b = *(glm::vec3*)(&b->data);

			glm::vec3 result(0.0f);
			result.x = lerp(_a.x, _b.x, t);
			result.y = lerp(_a.y, _b.y, t);
			result.z = lerp(_a.z, _b.z, t);

			entity.GetComponent<TransformComponent>()._transform.SetPosition(result);
		break;
		}
		case AnimationDataType::ROTATION:
		{
			glm::quat& _a = *(glm::quat*)(&a->data);
			glm::quat& _b = *(glm::quat*)(&b->data);

			glm::quat result;
			result = glm::slerp(_a, _b, t);

			entity.GetComponent<TransformComponent>()._transform.SetRotation(result);
		break;
		}
		case AnimationDataType::SCALE:
		{
			glm::vec3& _a = *(glm::vec3*)(&a->data);
			glm::vec3& _b = *(glm::vec3*)(&b->data);

			glm::vec3 result(0.0f);
			result.x = lerp(_a.x, _b.x, t);
			result.y = lerp(_a.y, _b.y, t);
			result.z = lerp(_a.z, _b.z, t);

			entity.GetComponent<TransformComponent>()._transform.SetScale(result);

		break;
		}
		case AnimationDataType::TEXT_INTENSITY:
		{
			float& _a = *(float*)(&a->data);
			float& _b = *(float*)(&b->data);

			float result(0.0f);
			result = lerp(_a, _b, t);

			entity.GetComponent<TextComponent>().intensity = result;
		break;
		}
		case AnimationDataType::LIGHT_INTENSITY:
		{
			float& _a = *(float*)(&a->data);
			float& _b = *(float*)(&b->data);

			float result(0.0f);
			result = lerp(_a, _b, t);

			entity.GetComponent<LightComponent>()._light.params2.y = result;
		break;
		}
		}
		

	}

}