#pragma once
#include <core/Core.h>
#include <core/Object.h>
#include <core/pch.h>
#include "core/Enums.h"


namespace trace {



	enum EventType
	{
		NONE = 0,

		TRC_CONSOLE_WRITE,
		TRC_APP_START,
		TRC_APP_END,
		TRC_WND_CLOSE,
		TRC_KEY_PRESSED,
		TRC_KEY_RELEASED,
		TRC_BUTTON_PRESSED,
		TRC_BUTTON_RELEASED,

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

		virtual const char* GetName() { return _STR(Event); }
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
		virtual const char* GetName() override { return _STR(ConsoleWriteEvent); }
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
		virtual const char* GetName() override { return _STR(ApplicationStart); }
	private:
	protected:
	};

	class TRACE_API ApplicationEnd : public Event
	{
	public:
		ApplicationEnd();
		~ApplicationEnd();

		const char* Log();
		virtual const char* GetName() override { return _STR(ApplicationEnd); }
	private:
	protected:
	};

	class TRACE_API WindowClose : public Event
	{

	public:
		WindowClose();
		~WindowClose();
		virtual const char* GetName() override { return _STR(WindowClose); }
	private:
	protected:

	};

	class TRACE_API KeyPressed : public Event
	{

	public:
		KeyPressed(int keycode);
		~KeyPressed();

		virtual const char* GetName() override { return _STR(KeyPressed); }
		int m_keycode;
	private:
	protected:

	};

	class TRACE_API KeyReleased : public Event
	{

	public:
		KeyReleased(int keycode);
		~KeyReleased();

		virtual const char* GetName() override { return _STR(KeyReleased); }
		int m_keycode;
	private:
	protected:

	};
}
