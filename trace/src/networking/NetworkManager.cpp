#include "pch.h"

#include "NetworkManager.h"
#include "scripting/ScriptEngine.h"
#include "scripting/ScriptBackend.h"
#include "scene/Entity.h"
#include "scene/Scene.h"

namespace trace::Network {


	NetworkManager* NetworkManager::get_instance()
	{
		static NetworkManager* s_instance = new NetworkManager();
		return s_instance;
	}

	bool NetworkManager::Init()
	{
		return true;
	}

	void NetworkManager::Shutdown()
	{
	}

	void NetworkManager::OnSceneStart(Scene* scene)
	{
		m_scene = scene;
	}

	void NetworkManager::OnSceneStop(Scene* scene)
	{
		m_scene = nullptr;
	}

	void NetworkManager::OnGameStart()
	{
	}

	void NetworkManager::OnGameStop()
	{
		if (m_type != NetType::UNKNOWN)
		{
			DestroyNetInstance();
		}
	}

	void NetworkManager::OnFrameStart()
	{
	}

	void NetworkManager::OnFrameEnd()
	{
	}

	bool NetworkManager::CreateListenServer(uint32_t port)
	{
		return false;
	}

	bool NetworkManager::ConnectTo(const std::string server, uint32_t port)
	{
		return false;
	}

	bool NetworkManager::ConnectToLAN(uint32_t port)
	{
		return false;
	}

	void NetworkManager::DestroyNetInstance()
	{
	}

	NetServer* NetworkManager::GetServerInstance()
	{
		return nullptr;
	}

	NetClient* NetworkManager::GetClientInstance()
	{
		return nullptr;
	}

	NetworkStream* NetworkManager::GetSendNetworkStream()
	{
		return nullptr;
	}

	NetworkStream* NetworkManager::GetRPCNetworkStream()
	{
		return nullptr;
	}

	void NetworkManager::Send(NetworkStream* packet, uint16_t mode)
	{
	}

	void NetworkManager::OnClientConnect(uint32_t handle)
	{
		if (m_type != NetType::LISTEN_SERVER)
		{
			return;
		}

		Script* network_script = ScriptEngine::get_instance()->GetNetworkScript();
		ScriptMethod* on_client_connect = network_script->GetMethod("OnClientConnect");
		if (on_client_connect)
		{
			InvokeScriptMethod_Class(*on_client_connect, *network_script, nullptr);
		}
	}

	void NetworkManager::OnClientDisconnect(uint32_t handle)
	{
		if (m_type != NetType::LISTEN_SERVER)
		{
			return;
		}

		Script* network_script = ScriptEngine::get_instance()->GetNetworkScript();
		ScriptMethod* on_client_disconnect = network_script->GetMethod("OnClientDisconnect");
		if (on_client_disconnect)
		{
			InvokeScriptMethod_Class(*on_client_disconnect, *network_script, nullptr);
		}
	}

	void NetworkManager::OnServerConnect(uint32_t handle)
	{
		if (m_type != NetType::CLIENT)
		{
			return;
		}

		Script* network_script = ScriptEngine::get_instance()->GetNetworkScript();
		ScriptMethod* on_server_connect = network_script->GetMethod("OnServerConnect");
		if (on_server_connect)
		{
			InvokeScriptMethod_Class(*on_server_connect, *network_script, nullptr);
		}
	}

	void NetworkManager::OnServerDisconnect(uint32_t handle)
	{
		if (m_type != NetType::CLIENT)
		{
			return;
		}

		Script* network_script = ScriptEngine::get_instance()->GetNetworkScript();
		ScriptMethod* on_server_disconnect = network_script->GetMethod("OnServerDisconnect");
		if (on_server_disconnect)
		{
			InvokeScriptMethod_Class(*on_server_disconnect, *network_script, nullptr);
		}
	}

	void NetworkManager::OnPacketRecieve(NetworkStream* data, uint32_t handle)
	{
		if (m_type == NetType::UNKNOWN)
		{
			return;
		}

		if (m_scene)
		{
			switch (m_type)
			{
			case NetType::CLIENT:
			{
				m_scene->OnPacketReceive_Client(data, handle);
				break;
			}
			case NetType::LISTEN_SERVER:
			{
				m_scene->OnPacketReceive_Server(data, handle);
				break;
			}
			}
		}
	}

}