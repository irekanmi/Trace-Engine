#pragma once

#include "resource/Resource.h"
#include "resource/Ref.h"
#include "scene/UUID.h"
#include "reflection/TypeRegistry.h"

#include <vector>

namespace trace {

	class Scene;

}

namespace trace::Animation {

	class SequenceTrack;

	enum class SequenceType
	{
		Unknown,
		Gameplay,
		Cutscene
	};

	class Sequence : public Resource
	{

	public:

		bool Create();
		virtual void Destroy() override;
		
		
		std::vector<SequenceTrack*>& GetTracks() { return m_tracks; }
		SequenceType GetType() { return m_type; }
		float GetDuration() { return m_duration; }

		void SetType(SequenceType type) { m_type = type; }
		void SetDuration(float duration) { m_duration = duration; }
		void RemoveTrack(int32_t index);

	public:
		static Ref<Sequence> Deserialize(UUID id);

	private:
		std::vector<SequenceTrack*> m_tracks;
		SequenceType m_type = SequenceType::Unknown;
		float m_duration = 0.0f;

	protected:
		ACCESS_CLASS_MEMBERS(Sequence);
		GET_TYPE_ID;

	};

	class SequenceInstance
	{

	public:
		
		SequenceInstance();
		SequenceInstance(SequenceInstance& other);
		SequenceInstance(const SequenceInstance& other);
		~SequenceInstance();

		bool CreateInstance(Ref<Sequence> sequence, Scene* scene);
		void DestroyInstance();
		void Update(Scene* scene, float deltaTime);
		void UpdateElaspedTime(Scene* scene, float elasped_time);
		void Start(Scene* scene, UUID id);
		void Stop(Scene* scene, UUID id);
		bool HasStarted() { return m_started; }

		Ref<Sequence> GetSequence() { return m_sequence; }
		float GetElaspedTime() { return m_elaspedTime; }
		std::vector<void*>& GetTracksData() { return m_tracksData; }
		float GetPreviousTime() { return m_previousTime; }

		void SetSequence(Ref<Sequence> sequence) { m_sequence = sequence; }

	private:
		Ref<Sequence> m_sequence;
		std::vector<void*> m_tracksData;
		float m_elaspedTime = 0.0f;
		float m_previousTime = 0.0f;
		bool m_started = false;
		bool m_instanciated = false;

	protected:
		ACCESS_CLASS_MEMBERS(SequenceInstance);

	};

}
