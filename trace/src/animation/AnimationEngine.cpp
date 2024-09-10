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

	void AnimationEngine::Animate(AnimationState& state, Scene* scene, std::unordered_map<std::string, UUID>& data_map)
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
			auto it = data_map.find(channel.first);
			if (it == data_map.end())
			{
				continue;
			}
			UUID object = it->second;
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

				if (!prev || !curr)
				{
					continue;
				}

				float lerp_value = (elasped_animation_time - prev->time_point) / (curr->time_point - prev->time_point);

				CalculateAndSetData(prev, curr, scene, object, track.channel_type, lerp_value);
			}
		}

	}

	void AnimationEngine::Animate(Ref<AnimationClip> clip, Scene* scene, float time_point, std::unordered_map<std::string, UUID>& data_map)
	{
		for (auto& channel : clip->GetTracks())
		{
			auto it = data_map.find(channel.first);
			if (it == data_map.end())
			{
				continue;
			}
			UUID object = it->second;
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

				if (!prev || !curr)
				{
					continue;
				}

				float lerp_value = (time_point - prev->time_point) / (curr->time_point - prev->time_point);

				CalculateAndSetData(prev, curr, scene, object, track.channel_type, lerp_value);
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
		case AnimationDataType::IMAGE_COLOR:
		{
			uint32_t& _a = *(uint32_t*)(&frame_a->data);
			uint32_t& _b = *(uint32_t*)(&frame_b->data);

			uint32_t result = TRC_COL32_WHITE;

			glm::vec4 color_a = colorFromUint32(_a);
			glm::vec4 color_b = colorFromUint32(_b);

			glm::vec4 color_result(0.0f);
			color_result.r = lerp(color_a.r, color_b.r, time_point);
			color_result.g = lerp(color_a.g, color_b.g, time_point);
			color_result.b = lerp(color_a.b, color_b.b, time_point);
			color_result.a = lerp(color_a.a, color_b.a, time_point);

			result = colorVec4ToUint(color_result);

			entity.GetComponent<ImageComponent>().color = result;
			break;
		}
		}
		

	}

}