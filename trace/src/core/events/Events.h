#pragma once
#include "core/Core.h"
#include "core/Object.h"
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
		TRC_GAMEPAD_CONNECT,
		TRC_GAMEPAD_DISCONNECT,

		MAX_EVENTS
	};

	
	
	class Event
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


	class ConsoleWriteEvent : public Event
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

	class ApplicationStart : public Event
	{
	public:
		ApplicationStart();
		~ApplicationStart();

		const char* Log();
		virtual const char* GetName() override { return _STR(ApplicationStart); }
	private:
	protected:
	};

	class ApplicationEnd : public Event
	{
	public:
		ApplicationEnd();
		~ApplicationEnd();

		const char* Log();
		virtual const char* GetName() override { return _STR(ApplicationEnd); }
	private:
	protected:
	};

	class WindowClose : public Event
	{

	public:
		WindowClose();
		~WindowClose();
		virtual const char* GetName() override { return _STR(WindowClose); }
	private:
	protected:

	};

	class KeyPressed : public Event
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

	class KeyReleased : public Event
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
	
	class KeyTyped : public Event
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

	class WindowResize : public Event
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

	class MousePressed : public Event
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

	class MouseReleased : public Event
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

	class MouseMove : public Event
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

	class MouseWheel : public Event
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

	class MouseDBClick : public Event
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
	
	class GamepadConnected : public Event
	{

	public:
		GamepadConnected(int32_t controller_id);
		~GamepadConnected();

		virtual const char* GetName() override { return _STR(GamepadConnected); }

		int32_t GetID() { return m_controllerID; }

		void SetID(int32_t controller_id) { m_controllerID = controller_id; }

	private:
		int32_t m_controllerID;
	protected:

	};
	
	class GamepadDisconnected : public Event
	{

	public:
		GamepadDisconnected(int32_t controller_id);
		~GamepadDisconnected();

		virtual const char* GetName() override { return _STR(GamepadDisconnected); }

		int32_t GetID() { return m_controllerID; }

		void SetID(int32_t controller_id) { m_controllerID = controller_id; }

	private:
		int32_t m_controllerID;
	protected:

	};

}
