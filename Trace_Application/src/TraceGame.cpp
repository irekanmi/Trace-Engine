
#include "TraceGame.h"
#include "scene/Entity.h"
#include "core/Utils.h"
#include "core/Coretypes.h"
#include "serialize/FileStream.h"
#include "builder/ProjectBuilder.h"
#include "scene/SceneManager.h"
#include "scripting/ScriptEngine.h"
#include "core/events/Events.h"
#include "render/Renderer.h"
#include "core/events/EventsSystem.h"

namespace trace {


	std::unordered_map<std::string, UUID> file_ids;
	std::unordered_map<UUID, std::string> id_names;

	bool TraceGame::Init()
	{

		load_assetdb();
		load_appinfo();
		ProjectBuilder::LoadBuildPack();

		return true;
	}

	void TraceGame::Start()
	{

		// Register Events
		{
			trace::EventsSystem::get_instance()->AddEventListener(trace::EventType::TRC_KEY_RELEASED, BIND_EVENT_FN(TraceGame::OnEvent));
			trace::EventsSystem::get_instance()->AddEventListener(trace::EventType::TRC_WND_RESIZE, BIND_EVENT_FN(TraceGame::OnEvent));
			trace::EventsSystem::get_instance()->AddEventListener(trace::EventType::TRC_KEY_PRESSED, BIND_EVENT_FN(TraceGame::OnEvent));
			trace::EventsSystem::get_instance()->AddEventListener(trace::EventType::TRC_WND_CLOSE, BIND_EVENT_FN(TraceGame::OnEvent));
			trace::EventsSystem::get_instance()->AddEventListener(trace::EventType::TRC_BUTTON_PRESSED, BIND_EVENT_FN(TraceGame::OnEvent));
			trace::EventsSystem::get_instance()->AddEventListener(trace::EventType::TRC_BUTTON_RELEASED, BIND_EVENT_FN(TraceGame::OnEvent));
			trace::EventsSystem::get_instance()->AddEventListener(trace::EventType::TRC_MOUSE_MOVE, BIND_EVENT_FN(TraceGame::OnEvent));
			trace::EventsSystem::get_instance()->AddEventListener(trace::EventType::TRC_MOUSE_DB_CLICK, BIND_EVENT_FN(TraceGame::OnEvent));
			trace::EventsSystem::get_instance()->AddEventListener(trace::EventType::TRC_KEY_TYPED, BIND_EVENT_FN(TraceGame::OnEvent));
			trace::EventsSystem::get_instance()->AddEventListener(trace::EventType::TRC_MOUSE_WHEEL, BIND_EVENT_FN(TraceGame::OnEvent));
		};

		std::string assembly_path;
		FindDirectory(AppSettings::exe_path, "Data/Assembly/" + m_name + ".dll", assembly_path);
		ScriptEngine::get_instance()->ReloadAssembly(assembly_path);



		m_currentScene = SceneManager::get_instance()->LoadScene_Runtime(m_startScene);

		m_currentScene->OnStart();
		m_currentScene->OnScriptStart();

		m_currentScene->OnViewportChange(Renderer::get_instance()->GetFrameWidth(), Renderer::get_instance()->GetFrameHeight());
	}

	void TraceGame::Update(float deltaTime)
	{
		m_currentScene->OnAnimationUpdate(deltaTime);
		m_currentScene->OnScriptUpdate(deltaTime);
		m_currentScene->OnPhysicsUpdate(deltaTime);
		m_currentScene->OnUpdate(deltaTime);
		m_currentScene->OnRender();
	}

	void TraceGame::Render(float deltaTime)
	{
	}

	void TraceGame::End()
	{
		m_currentScene->OnStop();
		m_currentScene->OnScriptStop();

		m_currentScene.free();
	}

	void TraceGame::OnEvent(Event* p_event)
	{


		switch (p_event->GetEventType())
		{
		case TRC_KEY_PRESSED:
		{
			trace::KeyPressed* press = reinterpret_cast<trace::KeyPressed*>(p_event);



			break;
		}
		case TRC_WND_CLOSE:
		{

			break;
		}
		case TRC_KEY_RELEASED:
		{
			trace::KeyReleased* release = reinterpret_cast<trace::KeyReleased*>(p_event);


			break;
		}

		case TRC_WND_RESIZE:
		{
			trace::WindowResize* wnd = reinterpret_cast<trace::WindowResize*>(p_event);
			if (m_currentScene)
			{
				m_currentScene->OnViewportChange(wnd->GetWidth(), wnd->GetHeight());
			}

			break;
		}
		case TRC_BUTTON_PRESSED:
		{
			trace::MousePressed* press = reinterpret_cast<trace::MousePressed*>(p_event);
			break;
		}
		case TRC_BUTTON_RELEASED:
		{
			trace::MouseReleased* release = reinterpret_cast<trace::MouseReleased*>(p_event);
			break;
		}
		case TRC_MOUSE_MOVE:
		{
			trace::MouseMove* move = reinterpret_cast<trace::MouseMove*>(p_event);
			break;
		}
		case TRC_MOUSE_DB_CLICK:
		{
			trace::MouseDBClick* click = reinterpret_cast<trace::MouseDBClick*>(p_event);
			break;
		}

		case TRC_KEY_TYPED:
		{
			trace::KeyTyped* typed = reinterpret_cast<trace::KeyTyped*>(p_event);


			break;
		}

		case TRC_MOUSE_WHEEL:
		{
			MouseWheel* wheel = reinterpret_cast<MouseWheel*>(p_event);
			break;
		}

		}

	}

	TraceGame* TraceGame::get_instance()
	{
		static TraceGame* s_instance = new TraceGame;//TODO: Use custom allocator
		return s_instance;
	}

	void TraceGame::load_appinfo()
	{

		std::string appinfo_path;
		FindDirectory(AppSettings::exe_path, "Meta/appinfo.trbin", appinfo_path);

		FileStream stream(appinfo_path, FileMode::READ);
		int bin_id = 0;
		stream.Read<int>(bin_id);
		if (bin_id != (int)(TRC_APP_INFO_ID))
		{
			TRC_ERROR("file is not a valid app info data, path -> {}", appinfo_path);
			return;
		}

		int trace_version = 0;
		stream.Read<int>(trace_version);// TODO: To be used later
		uint64_t build_version = 0;
		stream.Read<uint64_t>(build_version);// TODO: To be used later

		int name_length = 0;
		stream.Read<int>(name_length);
		char buf[512] = { 0 };//NOTE: Assuming name would not be greater than 512
		stream.Read(buf, name_length);
		m_name = buf;
		UUID start_scene_id;
		stream.Read<UUID>(start_scene_id);
		m_startScene = start_scene_id;

	}

	void TraceGame::load_assetdb()
	{

		if (!ProjectBuilder::LoadAssetsDB(file_ids, id_names))
		{
			TRC_ERROR("Unable to load assets db");
		}

	}

	std::filesystem::path GetPathFromUUID(UUID uuid)
	{
		return "";
	}
	UUID GetUUIDFromName(const std::string& name)
	{
		return file_ids[name];
	}

	std::string GetNameFromUUID(UUID uuid)
	{
		return id_names[uuid];
	}

}