#include "pch.h"

#include "animation/AnimationSequence.h"
#include "animation/AnimationSequenceTrack.h"
#include "animation/SequenceTrackChannel.h"
#include "scene/Scene.h"
#include "scene/Entity.h"

namespace trace::Animation {

	bool Sequence::Create()
	{
		return true;
	}

	void Sequence::Destroy()
	{
		for (SequenceTrack* track : m_tracks)
		{
			for (SequenceTrackChannel* channel : track->GetTrackChannels())
			{
				delete channel;//TODO Use a custom allocator
			}

			delete track;//TODO Use a custom allocator
		}
	}

	bool SequenceInstance::CreateInstance(Ref<Sequence> sequence, Scene* scene)
	{
		m_destroyed = false;
		std::vector<SequenceTrack*>& tracks = sequence->GetTracks();
		m_tracksData.resize(tracks.size());
		for (int32_t i = 0; i < tracks.size(); i++)
		{
			tracks[i]->Instanciate(this, scene, i);
		}

		return true;
	}

	void SequenceInstance::DestroyInstance()
	{

		for (auto& data : m_tracksData)
		{
			delete data;//TODO: Use a custom allocator
		}
		m_destroyed = true;
	}

	void SequenceInstance::Update(Scene* scene, float deltaTime)
	{

		std::vector<SequenceTrack*>& tracks = m_sequence->GetTracks();

		if (m_elaspedTime >= m_sequence->GetDuration())
		{
			return;
		}

		uint32_t index = 0;
		for (SequenceTrack*& track : tracks)
		{
			track->Update(this, scene, index);

			index++;
		}

		m_elaspedTime += deltaTime;

	}

	void SequenceInstance::Start(Scene* scene, UUID id)
	{
		m_started = true;
		CreateInstance(m_sequence, scene);
	}

	void SequenceInstance::Stop(Scene* scene, UUID id)
	{
		m_started = false;
		DestroyInstance();
	}

}