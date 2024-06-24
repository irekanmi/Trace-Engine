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
		virtual bool IsHandled() { return m_handled; }
		virtual EventType GetEventType() { return m_type; }
		
		virtual void SetEventHandled(bool handled) { m_handled = handled; }
		virtual void SetEventType(EventType type) { m_type = type; }

	private:
		
	protected:
		bool m_handled = false;
		EventType m_type = EventType::NONE;
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
		Keys GetKeyCode() { return m_keycode; }

		void SetKeyCode(Keys key_code) { m_keycode = key_code; }

	private:
		Keys m_keycode;
	protected:

	};

	class TRACE_API KeyReleased : public Event
	{

	public:
		KeyReleased(Keys keycode);
		~KeyReleased();

		virtual const char* GetName() override { return _STR(KeyReleased); }
		Keys GetKeyCode() { return m_keycode; }

		void SetKeyCode(Keys key_code) { m_keycode = key_code; }

	private:
		Keys m_keycode;
	protected:

	};
	
	class TRACE_API KeyTyped : public Event
	{

	public:
		KeyTyped(Keys keycode);
		~KeyTyped();

		virtual const char* GetName() override { return _STR(KeyTyped); }
		Keys GetKeyCode() { return m_keycode; }

		void SetKeyCode(Keys key_code) { m_keycode = key_code; }

	private:
		Keys m_keycode;
	protected:

	};

	class TRACE_API WindowResize : public Event
	{

	public:
		WindowResize();
		WindowResize(unsigned int width, unsigned int height);
		~WindowResize();

		virtual const char* GetName() override { return _STR(WindowResize); }

		uint32_t GetWidth() { return m_width; }
		uint32_t GetHeight() { return m_height; }

		void SetWidth(uint32_t width) { m_width = width; }
		void SetHeight(uint32_t height) { m_height = height; }

	private:
		uint32_t m_width;
		uint32_t m_height;
	protected:

	};

	class TRACE_API MousePressed : public Event
	{

	public:
		MousePressed(Buttons buttons);
		~MousePressed();

		virtual const char* GetName() override { return _STR(MousePressed); }

		Buttons GetButton() { return m_button; }

		void SetButton(Buttons button) { m_button = button; }

	private:
		Buttons m_button;
	protected:

	};

	class TRACE_API MouseReleased : public Event
	{

	public:
		MouseReleased(Buttons buttons);
		~MouseReleased();

		virtual const char* GetName() override { return _STR(MouseReleased); }

		Buttons GetButton() { return m_button; }

		void SetButton(Buttons button) { m_button = button; }

	private:
		Buttons m_button;
	protected:

	};

	class TRACE_API MouseMove : public Event
	{
		

	public:
		MouseMove();
		MouseMove(float x, float y);
		~MouseMove();

		virtual const char* GetName() override { return _STR(MouseMove); }

		float GetMouseX() { return m_x; }
		float GetMouseY() { return m_y; }

		void SetMouseX(float mouse_X) { m_x = mouse_X; }
		void SetMouseY(float mouse_Y) { m_y = mouse_Y; }

	private:
		float m_x;
		float m_y;
	protected:
	};

	class TRACE_API MouseWheel : public Event
	{


	public:
		MouseWheel();
		MouseWheel(float x, float y);
		~MouseWheel();

		virtual const char* GetName() override { return _STR(MouseWheel); }
		float GetMouseX() { return m_x; }
		float GetMouseY() { return m_y; }

		void SetMouseX(float mouse_X) { m_x = mouse_X; }
		void SetMouseY(float mouse_Y) { m_y = mouse_Y; }

	private:
		float m_x;
		float m_y;
	protected:
	};

	class TRACE_API MouseDBClick : public Event
	{

	public:
		MouseDBClick(Buttons buttons);
		~MouseDBClick();

		virtual const char* GetName() override { return _STR(MouseDBClick); }

		Buttons GetButton() { return m_button; }

		void SetButton(Buttons button) { m_button = button; }

	private:
		Buttons m_button;
	protected:

	};

}
