#pragma once


namespace trace {
	class Event;

	class AnimationSequencer
	{
	public:
		bool Init();
		void Shutdown();

		void Update(float deltaTime);
		void Render(float deltaTime);
		void OnEvent(Event* p_event);

	private:

	};

}