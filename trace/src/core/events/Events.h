#pragma once
#include <core/Core.h>
#include <core/Object.h>
#include <core/pch.h>
#include "core/Enums.h"
#include "core/input/Keys.h"

#include <functional>

namespace trace {



	enum EventType
	{
		NONE = 0,

		TRC_CONSOLE_WRITE,
		TRC_APP_START,
		TRC_APP_END,
		TRC_WND_CLOSE,
		TRC_WND_RESIZE,
		TRC_KEY_PRESSED,
		TRC_KEY_RELEASED,
		TRC_KEY_TYPED,
		TRC_BUTTON_PRESSED,
		TRC_BUTTON_RELEASED,
		TRC_MOUSE_MOVE,
		TRC_MOUSE_WHEEL,
		TRC_MOUSE_DB_CLICK,

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
		KeyPressed(Keys keycode);
		~KeyPressed();

		virtual const char* GetName() override { return _STR(KeyPressed); }
		Keys m_keycode;
	private:
	protected:

	};

	class TRACE_API KeyReleased : public Event
	{

	public:
		KeyReleased(Keys keycode);
		~KeyReleased();

		virtual const char* GetName() override { return _STR(KeyReleased); }
		Keys m_keycode;
	private:
	protected:

	};
	
	class TRACE_API KeyTyped : public Event
	{

	public:
		KeyTyped(Keys keycode);
		~KeyTyped();

		virtual const char* GetName() override { return _STR(KeyTyped); }
		Keys m_keycode;
	private:
	protected:

	};

	class TRACE_API WindowResize : public Event
	{

	public:
		WindowResize();
		WindowResize(unsigned int width, unsigned int height);
		~WindowResize();

		virtual const char* GetName() override { return _STR(WindowResize); }

		unsigned int m_width;
		unsigned int m_height;
	private:
	protected:

	};

	class TRACE_API MousePressed : public Event
	{

	public:
		MousePressed(Buttons buttons);
		~MousePressed();

		virtual const char* GetName() override { return _STR(MousePressed); }
		Buttons m_button;
	private:
	protected:

	};

	class TRACE_API MouseReleased : public Event
	{

	public:
		MouseReleased(Buttons buttons);
		~MouseReleased();

		virtual const char* GetName() override { return _STR(MouseReleased); }
		Buttons m_button;
	private:
	protected:

	};

	class TRACE_API MouseMove : public Event
	{
		

	public:
		MouseMove();
		MouseMove(float x, float y);
		~MouseMove();

		virtual const char* GetName() override { return _STR(MouseMove); }

		float m_x;
		float m_y;
	private:
	protected:
	};

	class TRACE_API MouseWheel : public Event
	{


	public:
		MouseWheel();
		MouseWheel(float x, float y);
		~MouseWheel();

		virtual const char* GetName() override { return _STR(MouseWheel); }

		float m_x;
		float m_y;
	private:
	protected:
	};

	class TRACE_API MouseDBClick : public Event
	{

	public:
		MouseDBClick(Buttons buttons);
		~MouseDBClick();

		virtual const char* GetName() override { return _STR(MouseDBClick); }
		Buttons m_button;
	private:
	protected:

	};

}
