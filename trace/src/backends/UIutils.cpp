#include "pch.h"

#include "UIutils.h"
#include "core/Application.h"
#include "render/Renderer.h"

#define UI_FUNC_IS_VALID(function)							 \
	if(!function)                                                \
	{                                                            \
		TRC_ERROR(                                               \
	"{} is not available, please check for any errors"           \
		, #function);                                            \
		return false;                                            \
	}



// ImGui Funcs ------------------------------------------
bool __ImGui_InitUIRenderBackend(trace::Application* application, trace::Renderer* renderer);
bool __ImGui_UINewFrame();
bool __ImGui_UIEndFrame();
bool __ImGui_UIRenderFrame(trace::Renderer* renderer);
bool __ImGui_ShutdownUIRenderBackend();
// ----------------------------------------------------

namespace trace {

	__InitUIRenderBackend UIFunc::_initUIRenderBackend = nullptr;
	__ShutdownUIRenderBackend UIFunc::_shutdownUIRenderBackend = nullptr;
	__UINewFrame UIFunc::_uiNewFrame = nullptr;
	__UIEndFrame UIFunc::_uiEndFrame = nullptr;
	__UIRenderFrame UIFunc::_uiRenderFrame = nullptr;

	bool UIFuncLoader::LoadImGuiFunc()
	{
		UIFunc::_initUIRenderBackend = __ImGui_InitUIRenderBackend;
		UIFunc::_shutdownUIRenderBackend = __ImGui_ShutdownUIRenderBackend;
		UIFunc::_uiNewFrame = __ImGui_UINewFrame;
		UIFunc::_uiEndFrame = __ImGui_UIEndFrame;
		UIFunc::_uiRenderFrame = __ImGui_UIRenderFrame;
		return true;
	}

	bool UIFunc::InitUIRenderBackend(Application* application, Renderer* renderer)
	{
		UI_FUNC_IS_VALID(_initUIRenderBackend);
		return _initUIRenderBackend(application, renderer);
	}

	bool UIFunc::UINewFrame()
	{
		UI_FUNC_IS_VALID(_uiNewFrame);
		return _uiNewFrame();
	}

	bool UIFunc::UIEndFrame()
	{
		UI_FUNC_IS_VALID(_uiEndFrame);
		return _uiEndFrame();
	}
	
	bool UIFunc::UIRenderFrame(Renderer* renderer)
	{
		UI_FUNC_IS_VALID(_uiRenderFrame);
		return _uiRenderFrame(renderer);
	}


	bool UIFunc::ShutdownUIRenderBackend()
	{
		UI_FUNC_IS_VALID(_shutdownUIRenderBackend);
		return _shutdownUIRenderBackend();
	}

}





// IMGUI ----------------------------------------------------


#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_vulkan.h"
#include "imgui_impl_glfw.h"
#include "core/platform/Vulkan/VKtypes.h"
#include "core/platform/Vulkan/VkUtils.h"
#include "core/events/EventsSystem.h"

static void check_result_vk_fn(VkResult result)
{
	VK_ASSERT(result, "IMGUI_VULKAN {}", __FUNCTION__);
}

static void init_win32(trace::Application* app)
{
	trace::Window* window = app->GetWindow();
	ImGui_ImplWin32_Init(window->GetHandle());
}


static void init_vulkan(trace::Renderer* renderer)
{
	trace::VKDeviceHandle* vk = reinterpret_cast<trace::VKDeviceHandle*>(renderer->GetDevice()->GetRenderHandle()->m_internalData);
	trace::VKHandle* vk_handle = reinterpret_cast<trace::VKHandle*>(renderer->GetContext()->GetRenderHandle()->m_internalData);
	trace::VKRenderPass* vk_pass = reinterpret_cast<trace::VKRenderPass*>(renderer->GetRenderPass("FORWARD_PASS")->GetRenderHandle()->m_internalData);

	ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.Instance = vk_handle->m_instance;
	init_info.PhysicalDevice = vk->m_physicalDevice;
	init_info.Device = vk->m_device;
	init_info.QueueFamily = vk->m_queues.graphics_queue;
	init_info.Queue = vk->m_graphicsQueue;
	init_info.PipelineCache = nullptr;
	init_info.DescriptorPool = vk->m_frameDescriptorPool[0];
	init_info.Subpass = 0;
	init_info.MinImageCount = vk->frames_in_flight;
	init_info.ImageCount = vk->frames_in_flight;
	init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT; // TODO: Configurable
	init_info.Allocator = vk_handle->m_alloc_callback;
	init_info.CheckVkResultFn = check_result_vk_fn;
	ImGui_ImplVulkan_Init(&init_info, vk_pass->m_handle);

	
	trace::VKCommmandBuffer cmd_buf = {};
	vk::_BeginCommandBufferSingleUse(vk, vk->m_graphicsCommandPool, &cmd_buf);
	ImGui_ImplVulkan_CreateFontsTexture(cmd_buf.m_handle);
	vk::_EndCommandBufferSingleUse(vk, vk->m_graphicsCommandPool, vk->m_graphicsQueue, &cmd_buf);
	ImGui_ImplVulkan_DestroyFontUploadObjects();
}


int translateKeyTrace_ImGui(trace::Keys key)
{
	switch (key)
	{
			case trace::KEY_BACKSPACE: { return ImGuiKey_Backspace;}
			case trace::KEY_ENTER: { return ImGuiKey_Enter;}
			case trace::KEY_TAB: { return ImGuiKey_Tab;}
			case trace::KEY_SHIFT: { return ImGuiKey_LeftShift;}
			case trace::KEY_CONTROL: { return ImGuiKey_LeftCtrl;}
			case trace::KEY_PAUSE: { return ImGuiKey_Pause;}
			case trace::KEY_CAPITAL: { return ImGuiKey_CapsLock;}
			case trace::KEY_ESCAPE: { return ImGuiKey_Escape;}
			case trace::KEY_CONVERT: { return ImGuiKey_None;} //TODO: Check Docs
			case trace::KEY_NONCONVERT: { return ImGuiKey_None;} //TODO: Check Docs
			case trace::KEY_ACCEPT: { return ImGuiKey_None;} //TODO: Check Docs
			case trace::KEY_MODECHANGE: { return ImGuiKey_None;} //TODO: Check Docs
			case trace::KEY_SPACE: { return ImGuiKey_Space;}
			case trace::KEY_PRIOR: { return ImGuiKey_None;} //TODO: Check Docs
			case trace::KEY_NEXT: { return ImGuiKey_None;} //TODO: Check Docs
			case trace::KEY_END: { return ImGuiKey_End;}
			case trace::KEY_HOME: { return ImGuiKey_Home;}
			case trace::KEY_LEFT: { return ImGuiKey_LeftArrow;}
			case trace::KEY_UP: { return ImGuiKey_UpArrow;}
			case trace::KEY_RIGHT: { return ImGuiKey_RightArrow;}
			case trace::KEY_DOWN: { return ImGuiKey_DownArrow;}
			case trace::KEY_SELECT: { return ImGuiKey_None;} //TODO: Check Docs
			case trace::KEY_PRINT: { return ImGuiKey_None;} //TODO: Check Docs
			case trace::KEY_EXECUTION: { return ImGuiKey_None;} //TODO: Check Docs
			case trace::KEY_SNAPSHOT: { return ImGuiKey_PrintScreen;}
			case trace::KEY_INSERT: { return ImGuiKey_Insert;}
			case trace::KEY_DELETE: { return ImGuiKey_Delete;}
			case trace::KEY_HELP: { return ImGuiKey_None;} //TODO: Check Docs
			case trace::KEY_0: { return ImGuiKey_0;}
			case trace::KEY_1: { return ImGuiKey_1;}
			case trace::KEY_2: { return ImGuiKey_2;}
			case trace::KEY_3: { return ImGuiKey_3;}
			case trace::KEY_4: { return ImGuiKey_4;}
			case trace::KEY_5: { return ImGuiKey_5;}
			case trace::KEY_6: { return ImGuiKey_6;}
			case trace::KEY_7: { return ImGuiKey_7;}
			case trace::KEY_8: { return ImGuiKey_8;}
			case trace::KEY_9: { return ImGuiKey_9;}
			case trace::KEY_A: { return ImGuiKey_A;}
			case trace::KEY_B: { return ImGuiKey_B;}
			case trace::KEY_C: { return ImGuiKey_C;}
			case trace::KEY_D: { return ImGuiKey_D;}
			case trace::KEY_E: { return ImGuiKey_E;}
			case trace::KEY_F: { return ImGuiKey_F;}
			case trace::KEY_G: { return ImGuiKey_G;}
			case trace::KEY_H: { return ImGuiKey_H;}
			case trace::KEY_I: { return ImGuiKey_I;}
			case trace::KEY_J: { return ImGuiKey_J;}
			case trace::KEY_K: { return ImGuiKey_K;}
			case trace::KEY_L: { return ImGuiKey_L;}
			case trace::KEY_M: { return ImGuiKey_M;}
			case trace::KEY_N: { return ImGuiKey_N;}
			case trace::KEY_O: { return ImGuiKey_O;}
			case trace::KEY_P: { return ImGuiKey_P;}
			case trace::KEY_Q: { return ImGuiKey_Q;}
			case trace::KEY_R: { return ImGuiKey_R;}
			case trace::KEY_S: { return ImGuiKey_S;}
			case trace::KEY_T: { return ImGuiKey_T;}
			case trace::KEY_U: { return ImGuiKey_U;}
			case trace::KEY_V: { return ImGuiKey_V;}
			case trace::KEY_W: { return ImGuiKey_W;}
			case trace::KEY_X: { return ImGuiKey_X;}
			case trace::KEY_Y: { return ImGuiKey_Y;}
			case trace::KEY_Z: { return ImGuiKey_Z;}
			case trace::KEY_LWIN: { return ImGuiKey_LeftSuper;}
			case trace::KEY_RWIN: { return ImGuiKey_RightSuper;}
			case trace::KEY_APPS: { return ImGuiKey_None;} //TODO: Check Docs
			case trace::KEY_SLEEP: { return ImGuiKey_None;} //TODO: Check Docs
			case trace::KEY_NUMPAD0: { return ImGuiKey_Keypad0;}
			case trace::KEY_NUMPAD1: { return ImGuiKey_Keypad1;}
			case trace::KEY_NUMPAD2: { return ImGuiKey_Keypad2;}
			case trace::KEY_NUMPAD3: { return ImGuiKey_Keypad3;}
			case trace::KEY_NUMPAD4: { return ImGuiKey_Keypad4;}
			case trace::KEY_NUMPAD5: { return ImGuiKey_Keypad5;}
			case trace::KEY_NUMPAD6: { return ImGuiKey_Keypad6;}
			case trace::KEY_NUMPAD7: { return ImGuiKey_Keypad7;}
			case trace::KEY_NUMPAD8: { return ImGuiKey_Keypad8;}
			case trace::KEY_NUMPAD9: { return ImGuiKey_Keypad9;}
			case trace::KEY_MULTIPLY: { return ImGuiKey_KeypadMultiply;}
			case trace::KEY_ADD: { return ImGuiKey_KeypadAdd;}
			case trace::KEY_SEPARATOR: { return ImGuiKey_None;} //TODO: Check Docs
			case trace::KEY_SUBTRACT: { return ImGuiKey_KeypadSubtract;}
			case trace::KEY_DECIMAL: { return ImGuiKey_KeypadDecimal;}
			case trace::KEY_DIVIDE: { return ImGuiKey_KeypadDivide;}
			case trace::KEY_F1: { return ImGuiKey_F1;}
			case trace::KEY_F2: { return ImGuiKey_F2;}
			case trace::KEY_F3: { return ImGuiKey_F3;}
			case trace::KEY_F4: { return ImGuiKey_F4;}
			case trace::KEY_F5: { return ImGuiKey_F5;}
			case trace::KEY_F6: { return ImGuiKey_F6;}
			case trace::KEY_F7: { return ImGuiKey_F7;}
			case trace::KEY_F8: { return ImGuiKey_F8;}
			case trace::KEY_F9: { return ImGuiKey_F9;}
			case trace::KEY_F10: { return ImGuiKey_F10;}
			case trace::KEY_F11: { return ImGuiKey_F11;}
			case trace::KEY_F12: { return ImGuiKey_F12;}
			case trace::KEY_NUMLOCK: { return ImGuiKey_NumLock;}
			case trace::KEY_SCROLL: { return ImGuiKey_ScrollLock;}
			case trace::KEY_NUMPAD_EQUAL: { return ImGuiKey_Equal;}
			case trace::KEY_LSHIFT: { return ImGuiKey_LeftShift;}
			case trace::KEY_RSHIFT: { return ImGuiKey_RightShift;}
			case trace::KEY_LCONTROL: { return ImGuiKey_LeftCtrl;}
			case trace::KEY_RCONTROL: { return ImGuiKey_RightCtrl;}
			case trace::KEY_LALT: { return ImGuiKey_LeftAlt;}
			case trace::KEY_RALT: { return ImGuiKey_RightAlt;}
			case trace::KEY_SEMICOLON: { return ImGuiKey_Semicolon;}
			case trace::KEY_PLUS: { return ImGuiKey_KeypadAdd;}
			case trace::KEY_COMMA: { return ImGuiKey_Comma;}
			case trace::KEY_MINUS: { return ImGuiKey_Minus;}
			case trace::KEY_PERIOD: { return ImGuiKey_Period;}
			case trace::KEY_SLASH: { return ImGuiKey_Slash;}
			case trace::KEY_GRAVE: { return ImGuiKey_GraveAccent; }
	}
	return 0;
}

int translateButtonTrace_ImGui(trace::Buttons button)
{
	switch (button)
	{
			case trace::BUTTON_LEFT: { return ImGuiMouseButton_Left;}
			case trace::BUTTON_RIGHT: { return ImGuiMouseButton_Right;}
			case trace::BUTTON_MIDDLE: { return ImGuiMouseButton_Middle;}
	}

	return 0;
}

static void OnEvent(trace::Event* p_event)
{
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	switch (p_event->m_type)
	{
	case trace::EventType::TRC_KEY_PRESSED:
	{
		trace::KeyPressed* press = reinterpret_cast<trace::KeyPressed*>(p_event);
		io.AddKeyEvent((ImGuiKey)translateKeyTrace_ImGui(press->m_keycode), true);

		break;
	}
	case trace::EventType::TRC_WND_CLOSE:
	{

		break;
	}
	case trace::EventType::TRC_KEY_RELEASED:
	{
		trace::KeyReleased* release = reinterpret_cast<trace::KeyReleased*>(p_event);
		io.AddKeyEvent((ImGuiKey)translateKeyTrace_ImGui(release->m_keycode), false);
		break;
	}

	case trace::EventType::TRC_WND_RESIZE:
	{
		trace::WindowResize* wnd = reinterpret_cast<trace::WindowResize*>(p_event);
		io.DisplaySize = ImVec2(wnd->m_width, wnd->m_height);
		io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);

		break;
	}
	case trace::EventType::TRC_BUTTON_PRESSED:
	{
		trace::MousePressed* press = reinterpret_cast<trace::MousePressed*>(p_event);
		io.AddMouseButtonEvent(translateButtonTrace_ImGui(press->m_button), true);
		break;
	}
	case trace::EventType::TRC_BUTTON_RELEASED:
	{
		trace::MouseReleased* release = reinterpret_cast<trace::MouseReleased*>(p_event);
		io.AddMouseButtonEvent(translateButtonTrace_ImGui(release->m_button), false);
		break;
	}
	case trace::EventType::TRC_MOUSE_MOVE:
	{
		trace::MouseMove* move = reinterpret_cast<trace::MouseMove*>(p_event);
		io.AddMousePosEvent(move->m_x, move->m_y);
		break;
	}
	
	case trace::EventType::TRC_KEY_TYPED:
	{
		trace::KeyTyped* typed = reinterpret_cast<trace::KeyTyped*>(p_event);
		unsigned int c = typed->m_keycode;
		if (c > 0 && c < 0x10000)
			io.AddInputCharacter((unsigned short)c);

		break;
	}

	}
}

bool __ImGui_InitUIRenderBackend(trace::Application* application, trace::Renderer* renderer)
{

	trace::EventsSystem::get_instance()->AddEventListener(trace::EventType::TRC_KEY_RELEASED, OnEvent);
	trace::EventsSystem::get_instance()->AddEventListener(trace::EventType::TRC_WND_RESIZE, OnEvent);
	trace::EventsSystem::get_instance()->AddEventListener(trace::EventType::TRC_KEY_PRESSED, OnEvent);
	trace::EventsSystem::get_instance()->AddEventListener(trace::EventType::TRC_WND_CLOSE, OnEvent);
	trace::EventsSystem::get_instance()->AddEventListener(trace::EventType::TRC_BUTTON_PRESSED, OnEvent);
	trace::EventsSystem::get_instance()->AddEventListener(trace::EventType::TRC_BUTTON_RELEASED, OnEvent);
	trace::EventsSystem::get_instance()->AddEventListener(trace::EventType::TRC_MOUSE_MOVE, OnEvent);
	trace::EventsSystem::get_instance()->AddEventListener(trace::EventType::TRC_KEY_TYPED, OnEvent);

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
	//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
	//io.ConfigViewportsNoAutoMerge = true;
	//io.ConfigViewportsNoTaskBarIcon = true;

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight();



	switch (trace::AppSettings::wintype)
	{
	case trace::WindowType::WIN32_WINDOW:
	{
		init_win32(application);
		break;
	}
	case trace::WindowType::GLFW_WINDOW:
	{
		ImGui_ImplGlfw_InitForVulkan((GLFWwindow*)application->GetWindow()->GetHandle(), false); // TODO: Implement for other Render API
		break;
	}
	}

	switch (trace::AppSettings::graphics_api)
	{
	case trace::RenderAPI::Vulkan:
	{
		init_vulkan(renderer);
		break;
	}
	}

	return true;
}

bool __ImGui_UINewFrame()
{

	switch (trace::AppSettings::graphics_api)
	{
	case trace::RenderAPI::Vulkan:
	{
		ImGui_ImplVulkan_NewFrame();
		break;
	}
	}

	switch (trace::AppSettings::wintype)
	{
	case trace::WindowType::WIN32_WINDOW:
	{
		ImGui_ImplWin32_NewFrame();
		break;
	}
	case trace::WindowType::GLFW_WINDOW:
	{
		ImGui_ImplGlfw_NewFrame(); // TODO: Implement for other Render API
		break;
	}
	}
	ImGui::NewFrame();

	return true;
}

bool __ImGui_UIEndFrame()
{
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	switch (trace::AppSettings::graphics_api)
	{
	case trace::RenderAPI::Vulkan:
	{
		break;
	}
	}

	switch (trace::AppSettings::wintype)
	{
	case trace::WindowType::WIN32_WINDOW:
	{
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}
		break;
	}
	}
	return true;
}

bool __ImGui_UIRenderFrame(trace::Renderer* renderer)
{
	// Rendering
	ImGui::Render();
	ImDrawData* main_draw_data = ImGui::GetDrawData();

	switch (trace::AppSettings::graphics_api)
	{
	case trace::RenderAPI::Vulkan:
	{
		trace::VKDeviceHandle* vk = reinterpret_cast<trace::VKDeviceHandle*>(renderer->GetDevice()->GetRenderHandle()->m_internalData);
		trace::VKCommmandBuffer& cmd_buf = vk->m_graphicsCommandBuffers[vk->m_imageIndex];
		// Record dear imgui primitives into command buffer
		ImGui_ImplVulkan_RenderDrawData(main_draw_data, cmd_buf.m_handle);
		break;
	}
	}

	return true;
}


bool __ImGui_ShutdownUIRenderBackend()
{

	switch (trace::AppSettings::graphics_api)
	{
	case trace::RenderAPI::Vulkan:
	{
		ImGui_ImplVulkan_Shutdown();
		break;
	}
	}

	switch (trace::AppSettings::wintype)
	{
	case trace::WindowType::WIN32_WINDOW:
	{
		ImGui_ImplWin32_Shutdown();
		break;
	}
	case trace::WindowType::GLFW_WINDOW:
	{
		ImGui_ImplGlfw_Shutdown(); // TODO: Implement for other Render API
		break;
	}
	}

	ImGui::DestroyContext();

	return true;
}

// ----------------------------------------------------------