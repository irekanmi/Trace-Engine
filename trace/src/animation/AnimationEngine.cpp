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
			if (!state.GetLoop())
			{
				state.Stop();
				return;
			}
			else _t = fmod(_t, clip->GetDuration());
		}
		state.SetElaspedTime(_t);

		for (auto& channel : clip->GetTracks())
		{
			for (auto& track : channel.second)
			{
				AnimationFrameData* curr = nullptr;
				AnimationFrameData* prev = nullptr;

				//TODO: Find a better way to find which frames to sample -----------
				int i = track.channel_data.size() - 1;
				for (; i >= 0; i--)
				{
					curr = &track.channel_data[i];
					prev = i != 0 ? &track.channel_data[i - 1] : nullptr;
					if (_t <= curr->time_point)
					{
						if (prev && _t >= prev->time_point) break;
					}

				}
				// -----------------------------------------------------------------

				if (!prev || !curr) continue;

				float lerp_value = (_t - prev->time_point) / (curr->time_point - prev->time_point);

				CalculateAndSetData(prev, curr, scene, channel.first, track.channel_type, lerp_value);
			}
		}

	}

	void AnimationEngine::Animate(Ref<AnimationClip> clip, Scene* scene, float t)
	{
		for (auto& channel : clip->GetTracks())
		{
			for (auto& track : channel.second)
			{
				AnimationFrameData* curr = nullptr;
				AnimationFrameData* prev = nullptr;

				//TODO: Find a better way to find which frames to sample -----------
				int i = track.channel_data.size() - 1;
				for (; i >= 0; i--)
				{
					curr = &track.channel_data[i];
					prev = i != 0 ? &track.channel_data[i - 1] : nullptr;
					if (t <= curr->time_point)
					{
						if (prev && t >= prev->time_point) break;
					}

				}
				// -----------------------------------------------------------------

				if (!prev || !curr) continue;

				float lerp_value = (t - prev->time_point) / (curr->time_point - prev->time_point);

				CalculateAndSetData(prev, curr, scene, channel.first, track.channel_type, lerp_value);
			}
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