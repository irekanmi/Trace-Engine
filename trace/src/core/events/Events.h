#pragma once
#include <core/Core.h>
#include <core/Object.h>
#include <core/pch.h>


namespace trace {



	enum EventType
	{
		NONE = 0,

		TRC_CONSOLE_WRITE,
		TRC_APP_START,
		TRC_APP_END,

		MAX_EVENTS
	};

	
	
	class TRACE_API Event
	{

	public:
		Event()
		{

		}
		~Event()
		{

		}
		bool m_handled = false;
		EventType m_type = EventType::NONE;
	private:
		
	protected:
	};
	
	//typedef void(*EventCallbackFunc)(Event*);

	using EventCallbackFunc = std::function<void(Event*)>;

	struct EventData
	{
		Event m_data;
		EventCallbackFunc callback;
	};


	class TRACE_API ConsoleWriteEvent : public Event
	{
	public:
		ConsoleWriteEvent();
		~ConsoleWriteEvent();
		void SetData(std::string data);
		std::string GetData();

	private:
		std::string m_data;
	protected:
	};

	class TRACE_API ApplicationStart : public Event
	{
	public:
		ApplicationStart();
		~ApplicationStart();

		const char* Log();

	private:
	protected:
	};

	class TRACE_API ApplicationEnd : public Event
	{
	public:
		ApplicationEnd();
		~ApplicationEnd();

		const char* Log();

	private:
	protected:
	};
}
