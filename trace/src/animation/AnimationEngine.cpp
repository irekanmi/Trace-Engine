#include "pch.h"

#include "AnimationEngine.h"
#include "core/Application.h"
#include "scene/Scene.h"
#include "scene/Entity.h"
#include "core/Utils.h"

#include "glm/glm.hpp"



namespace trace {


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

		float elasped_animation_time = Application::get_instance()->GetClock().GetElapsedTime() - state.GetStartTime();

		if (elasped_animation_time > clip->GetDuration())
		{
			if (!state.GetLoop())
			{
				state.Stop();
				return;
			}
			else elasped_animation_time = fmod(elasped_animation_time, clip->GetDuration());
		}
		state.SetElaspedTime(elasped_animation_time);

		for (auto& channel : clip->GetTracks())
		{
			for (auto& track : channel.second)
			{
				const AnimationFrameData* curr = nullptr;
				const AnimationFrameData* prev = nullptr;

				//TODO: Find a better way to find which frames to sample -----------
				int i = track.channel_data.size() - 1;
				for (; i >= 0; i--)
				{
					curr = &track.channel_data[i];
					prev = i != 0 ? &track.channel_data[i - 1] : nullptr;
					if (elasped_animation_time <= curr->time_point)
					{
						if (prev && elasped_animation_time >= prev->time_point) break;
					}

				}
				// -----------------------------------------------------------------

				if (!prev || !curr) continue;

				float lerp_value = (elasped_animation_time - prev->time_point) / (curr->time_point - prev->time_point);

				CalculateAndSetData(prev, curr, scene, channel.first, track.channel_type, lerp_value);
			}
		}

	}

	void AnimationEngine::Animate(Ref<AnimationClip> clip, Scene* scene, float time_point)
	{
		for (auto& channel : clip->GetTracks())
		{
			for (const AnimationTrack& track : channel.second)
			{
				const AnimationFrameData* curr = nullptr;
				const AnimationFrameData* prev = nullptr;

				//TODO: Find a better way to find which frames to sample -----------
				int i = track.channel_data.size() - 1;
				for (; i >= 0; i--)
				{
					curr = &track.channel_data[i];
					prev = i != 0 ? &track.channel_data[i - 1] : nullptr;
					if (time_point <= curr->time_point)
					{
						if (prev && time_point >= prev->time_point) break;
					}

				}
				// -----------------------------------------------------------------

				if (!prev || !curr) continue;

				float lerp_value = (time_point - prev->time_point) / (curr->time_point - prev->time_point);

				CalculateAndSetData(prev, curr, scene, channel.first, track.channel_type, lerp_value);
			}
		}
	}

	AnimationEngine* AnimationEngine::get_instance()
	{
		static AnimationEngine* s_instance = new AnimationEngine();
		return s_instance;
	}

	void AnimationEngine::CalculateAndSetData(const AnimationFrameData* frame_a, const AnimationFrameData* frame_b, Scene* scene, UUID id, AnimationDataType type, float time_point)
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
			glm::vec3& _a = *(glm::vec3*)(&frame_a->data);
			glm::vec3& _b = *(glm::vec3*)(&frame_b->data);

			glm::vec3 result(0.0f);
			result.x = lerp(_a.x, _b.x, time_point);
			result.y = lerp(_a.y, _b.y, time_point);
			result.z = lerp(_a.z, _b.z, time_point);

			entity.GetComponent<TransformComponent>()._transform.SetPosition(result);
		break;
		}
		case AnimationDataType::ROTATION:
		{
			glm::quat& _a = *(glm::quat*)(&frame_a->data);
			glm::quat& _b = *(glm::quat*)(&frame_b->data);

			glm::quat result;
			result = glm::slerp(_a, _b, time_point);

			entity.GetComponent<TransformComponent>()._transform.SetRotation(result);
		break;
		}
		case AnimationDataType::SCALE:
		{
			glm::vec3& _a = *(glm::vec3*)(&frame_a->data);
			glm::vec3& _b = *(glm::vec3*)(&frame_b->data);

			glm::vec3 result(0.0f);
			result.x = lerp(_a.x, _b.x, time_point);
			result.y = lerp(_a.y, _b.y, time_point);
			result.z = lerp(_a.z, _b.z, time_point);

			entity.GetComponent<TransformComponent>()._transform.SetScale(result);

		break;
		}
		case AnimationDataType::TEXT_INTENSITY:
		{
			float& _a = *(float*)(&frame_a->data);
			float& _b = *(float*)(&frame_b->data);

			float result(0.0f);
			result = lerp(_a, _b, time_point);

			entity.GetComponent<TextComponent>().intensity = result;
		break;
		}
		case AnimationDataType::LIGHT_INTENSITY:
		{
			float& _a = *(float*)(&frame_a->data);
			float& _b = *(float*)(&frame_b->data);

			float result(0.0f);
			result = lerp(_a, _b, time_point);

			entity.GetComponent<LightComponent>()._light.params2.y = result;
		break;
		}
		}
		

	}

}