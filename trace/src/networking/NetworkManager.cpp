#include "pch.h"

#include "NetworkManager.h"
#include "scripting/ScriptEngine.h"
#include "scripting/ScriptBackend.h"
#include "scene/Entity.h"
#include "scene/Scene.h"
#include "backends/Networkutils.h"
#include "core/Enums.h"
#include "core/io/Logging.h"


namespace trace::Network {


	NetworkManager* NetworkManager::get_instance()
	{
		static NetworkManager* s_instance = new NetworkManager();
		return s_instance;
	}

	NetworkManager::NetworkManager()
	{
	}

	NetworkManager::~NetworkManager()
	{
	}

	bool NetworkManager::Init()
	{
		bool result = NetFunc::Initialize(m_info);

		m_receivePacket.data = NetworkStream(KB);
		m_type = NetType::UNKNOWN;

		return true;// result;
	}

	void NetworkManager::Shutdown()
	{
		if (m_type != NetType::UNKNOWN)
		{
			DestroyNetInstance();
		}
		m_receivePacket.data.Destroy();
		NetFunc::Shutdown();
	}

	void NetworkManager::Update(float deltaTime)
	{
		if (m_type == NetType::UNKNOWN)
		{
			return;
		}

		switch (m_type)
		{
		case NetType::CLIENT:
		{
			if (client.Listen(m_receivePacket))
			{
				OnPacketRecieve(&m_receivePacket.data, m_receivePacket.connection_handle);
			}
			break;
		}
		case NetType::LISTEN_SERVER:
		{
			if (server.Listen(m_receivePacket))
			{
				OnPacketRecieve(&m_receivePacket.data, m_receivePacket.connection_handle);
			}
			break;
		}
		}
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
		if (m_type == NetType::UNKNOWN)
		{
			return;
		}
		m_sendPacket.data.SetPosition(m_sendStartPos);
	}

	void NetworkManager::OnFrameEnd()
	{
		if (m_type == NetType::UNKNOWN)
		{
			return;
		}

		bool has_data = (m_sendPacket.data.GetPosition() <= m_sendStartPos);

		if (!has_data)
		{
			return;
		}

		switch (m_type)
		{
		case NetType::CLIENT:
		{
			client.SendPacketToServer(m_sendPacket);
			break;
		}
		case NetType::LISTEN_SERVER:
		{
			server.BroadcastToAll(m_sendPacket);
			break;
		}
		}
	}

	bool NetworkManager::CreateListenServer(uint32_t port)
	{
		if (m_type != NetType::UNKNOWN)
		{
			TRC_WARN("A Network Instance has already been created, Function: {}", __FUNCTION__);
			return false;
		}

		bool result = true;

		result = server.Init(m_info, false, port);

		if (result)
		{
			m_type = NetType::LISTEN_SERVER;
			m_sendPacket = server.CreateSendPacket(KB);
			m_sendStartPos = m_sendPacket.data.GetPosition();

			server.SetClientConnectCallback(BIND_EVENT_FN(NetworkManager::OnClientConnect));
			server.SetClientDisconnectCallback(BIND_EVENT_FN(NetworkManager::OnClientDisconnect));
		}

		

		return result;
	}

	bool NetworkManager::CreateClient(bool LAN)
	{
		if (m_type != NetType::UNKNOWN)
		{
			TRC_WARN("A Network Instance has already been created, Function: {}", __FUNCTION__);
			return false;
		}
		bool result = true;

		result = client.Init(m_info, LAN);

		if (result)
		{
			m_type = NetType::LISTEN_SERVER;
			m_sendPacket = client.CreateSendPacket(KB);
			m_sendStartPos = m_sendPacket.data.GetPosition();

			client.SetServerConnectCallback(BIND_EVENT_FN(NetworkManager::OnServerConnect));
			client.SetServerDisconnectCallback(BIND_EVENT_FN(NetworkManager::OnServerDisconnect));
		}

		return result;
	}

	bool NetworkManager::ConnectTo(const std::string server, uint32_t port)
	{
		bool result = true;

		if (m_type != NetType::CLIENT)
		{
			return false;
		}

		client.ConnectTo(server, port);

		return result;
	}

	bool NetworkManager::ConnectToLAN(const std::string& server)
	{
		bool result = true;

		if (m_type != NetType::CLIENT)
		{
			return false;
		}

		client.ConnectToLAN(server);

		return result;
	}

	void NetworkManager::DestroyNetInstance()
	{
		switch (m_type)
		{
		case NetType::CLIENT:
		{
			client.Shutdown();
			break;
		}
		case NetType::LISTEN_SERVER:
		{
			server.Shutdown();
			break;
		}
		}
		m_type = NetType::UNKNOWN;
		m_sendPacket.data.Destroy();
		m_sendPacket.connection_handle = 0;
	}

	NetServer* NetworkManager::GetServerInstance()
	{
		switch (m_type)
		{
		case NetType::LISTEN_SERVER:
		{
			return &server;
			break;
		}
		}

		return nullptr;
	}

	NetClient* NetworkManager::GetClientInstance()
	{
		switch (m_type)
		{
		case NetType::CLIENT:
		{
			return &client;
			break;
		}
		}

		return nullptr;
	}

	NetworkStream* NetworkManager::GetSendNetworkStream()
	{
		if (m_type == NetType::UNKNOWN)
		{
			return nullptr;
		}

		return &m_sendPacket.data;
	}

	void NetworkManager::Send(NetworkStream* packet, PacketSendMode mode)
	{
		//TODO: Implement Later
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
			void* params[] =
			{
				&handle
			};
			InvokeScriptMethod_Class(*on_client_connect, *network_script, params);
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
			void* params[] =
			{
				&handle
			};
			InvokeScriptMethod_Class(*on_client_disconnect, *network_script, params);
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
			void* params[] =
			{
				&handle
			};
			InvokeScriptMethod_Class(*on_server_connect, *network_script, params);
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
			void* params[] =
			{
				&handle
			};
			InvokeScriptMethod_Class(*on_server_disconnect, *network_script, params);
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