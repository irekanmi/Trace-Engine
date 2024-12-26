#include "pch.h"

#include "animation/AnimationSequence.h"
#include "animation/AnimationSequenceTrack.h"
#include "animation/SequenceTrackChannel.h"
#include "scene/Scene.h"
#include "scene/Entity.h"
#include "serialize/AnimationsSerializer.h"

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

	Ref<Sequence> Sequence::Deserialize(const std::string& file_path)
	{
		return AnimationsSerializer::DeserializeSequence(file_path);
	}

	SequenceInstance::SequenceInstance()
	{
		m_instanciated = false;
		m_started = false;
	}
	SequenceInstance::SequenceInstance(SequenceInstance& other)
	{
		m_instanciated = false;
		m_started = false;
		m_sequence = other.m_sequence;
	}
	SequenceInstance::SequenceInstance(const SequenceInstance& other)
	{
		m_instanciated = false;
		m_started = false;
		m_sequence = const_cast<Ref<Sequence>&>(other.m_sequence);
	}

	SequenceInstance::~SequenceInstance()
	{
		DestroyInstance();
	}

	bool SequenceInstance::CreateInstance(Ref<Sequence> sequence, Scene* scene)
	{
		if (!sequence)
		{
			TRC_ERROR("Invalid Sequence handle");
			return false;
		}
		if (!scene)
		{
			TRC_ERROR("Invalid scene handle");
			return false;
		}

		if (m_instanciated)
		{
			TRC_ERROR("These Instance has already been created, Try to destroy the instance before trying again");
			return false;
		}

		m_sequence = sequence;
		std::vector<SequenceTrack*>& tracks = sequence->GetTracks();
		m_tracksData.resize(tracks.size());
		for (int32_t i = 0; i < tracks.size(); i++)
		{
			tracks[i]->Instanciate(this, scene, i);
		}

		m_instanciated = true;
		return true;
	}

	void SequenceInstance::DestroyInstance()
	{
		if (m_instanciated)
		{
			for (auto& data : m_tracksData)
			{
				delete data;//TODO: Use a custom allocator
			}
			m_instanciated = false;
		}
	}

	void SequenceInstance::Update(Scene* scene, float deltaTime)
	{
		if (!m_started)
		{
			TRC_WARN("Sequence has not started can't update sequence instance. Function: {}", __FUNCTION__);
			return;
		}

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
	}

	void SequenceInstance::Stop(Scene* scene, UUID id)
	{
		m_started = false;
	}

}