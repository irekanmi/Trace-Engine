#include "pch.h"

#include "NetworkManager.h"
#include "scripting/ScriptEngine.h"
#include "scripting/ScriptBackend.h"
#include "scene/Entity.h"
#include "scene/Scene.h"
#include "backends/Networkutils.h"
#include "core/Enums.h"
#include "core/io/Logging.h"
#include "core/Platform.h"


namespace trace::Network {


	NetworkManager* NetworkManager::get_instance()
	{
		static NetworkManager* s_instance = new NetworkManager();
		return s_instance;
	}

	void NetworkManager::process_new_client(uint32_t handle)
	{
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

	NetworkManager::NetworkManager()
	{
	}

	NetworkManager::~NetworkManager()
	{
	}

	bool NetworkManager::Init()
	{
		if (!NetFuncLoader::Load_ENet_Func())
		{
			return false;
		}

		bool result = NetFunc::Initialize(m_info);

		m_receivePacket.data = NetworkStream(KB);
		m_type = NetType::UNKNOWN;
		rpc_send_stream = NetworkStream(KB);
		send_stream = NetworkStream(KB);

		return result;
	}

	void NetworkManager::Shutdown()
	{
		if (m_type != NetType::UNKNOWN)
		{
			DestroyNetInstance();
		}
		send_stream.Destroy();
		rpc_send_stream.Destroy();
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
				m_receivePacket.data.MemSet(0, m_receivePacket.data.GetSize(), 0x00);
			}
			break;
		}
		case NetType::LISTEN_SERVER:
		{
			if (server.Listen(m_receivePacket))
			{
				OnPacketRecieve(&m_receivePacket.data, m_receivePacket.connection_handle);
				m_receivePacket.data.MemSet(0, m_receivePacket.data.GetSize(), 0x00);
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

	bool NetworkManager::OnFrameStart()
	{

		switch (m_type)
		{
		case NetType::UNKNOWN:
		{
			return false;
			break;
		}
		case NetType::CLIENT:
		{
			if (!client.HasConnection() || !m_sendPacket.data.HasData())
			{
				return false;
			}

			break;
		}
		}

		return true;
	}

	void NetworkManager::OnFrameEnd()
	{
		if (m_type == NetType::UNKNOWN)
		{
			return;
		}

		// Check if rpc stream can be read ...
		// if so rpc it and add it to the send stream
		PacketSendMode mode = PacketSendMode::UNRELIABLE;
		if (rpc_handle == 0 &&  rpc_send_stream.GetPosition() > 0)
		{
			mode = PacketSendMode::RELIABLE;
			m_sendPacket.data.Write(rpc_send_stream.GetData(), rpc_send_stream.GetPosition());

			rpc_send_stream.SetPosition(0);
			rpc_send_stream.MemSet(0, rpc_send_stream.GetSize(), 0x00);
		}

		if (send_stream.GetPosition() > 0)
		{
			m_sendPacket.data.Write(send_stream.GetData(), send_stream.GetPosition());

			send_stream.SetPosition(0);
			send_stream.MemSet(0, send_stream.GetSize(), 0x00);
		}


		bool has_data = !(m_sendPacket.data.GetPosition() <= m_sendStartPos);

		if (!has_data)
		{
			return;
		}

		switch (m_type)
		{
		case NetType::CLIENT:
		{
			if (client.HasConnection())
			{
				client.SendPacketToServer(m_sendPacket, mode);
			}
			break;
		}
		case NetType::LISTEN_SERVER:
		{
			if (m_info.state_sync)
			{
				for (uint32_t& handle : connected_clients)
				{
					server.SendTo(handle, m_sendPacket, mode);
				}
			}
			else
			{
				server.BroadcastToAll(m_sendPacket, mode);
			}
			break;
		}
		}

		m_sendPacket.data.SetPosition(m_sendStartPos);
		m_sendPacket.data.MemSet(m_sendStartPos, m_sendPacket.data.GetSize(), 0x00);
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
			m_instanceId = 100;//TODO: Generate a server id

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
			m_type = NetType::CLIENT;
			

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
		NetType net_type = m_type;
		m_type = NetType::UNKNOWN;
		switch (net_type)
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
		m_sendPacket.data.Destroy();
		m_sendPacket.connection_handle = 0;
		m_sendStartPos = 0;
		m_instanceId = 0;
		world_state_packet_received = false;
		connected_clients.clear();
		new_clients.clear();
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

		return &send_stream;
	}

	NetworkStream* NetworkManager::GetRPCSendNetworkStream()
	{
		return &rpc_send_stream;
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

		if (m_info.state_sync)
		{
			// Write scene state and send
			new_clients.push_back(handle);
			TRC_INFO("New client: {}", handle);
			if (m_scene)
			{
				Packet scene_state = server.CreateSendPacket(KB);// TODO: Implement custom allocator{ as soon as possible}. it affects resizing of network stream
				PacketMessageType message_type = PacketMessageType::SCENE_STATE;
				scene_state.data.Write(message_type);
				m_scene->WriteSceneState_Server(&scene_state.data);
				server.SendTo(handle, scene_state, PacketSendMode::RELIABLE);
			}

		}
		else
		{			
			process_new_client(handle);
		}

	}

	void NetworkManager::OnClientDisconnect(uint32_t handle)
	{
		if (m_type != NetType::LISTEN_SERVER)
		{
			return;
		}

		TRC_INFO("Client Disconnect: {}", handle);

		if (m_info.state_sync)
		{
			auto c_it = std::find(connected_clients.begin(), connected_clients.end(), handle);
			if (c_it != connected_clients.end())
			{
				connected_clients.erase(c_it);
			}
			
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

		m_instanceId = handle;
		m_sendPacket = client.CreateSendPacket(KB);
		m_sendStartPos = m_sendPacket.data.GetPosition();


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

		m_instanceId = 0;

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

		if (m_scene && m_scene->IsRunning())
		{
			switch (m_type)
			{
			case NetType::CLIENT:
			{
				if (m_info.state_sync)
				{
					if (!world_state_packet_received)
					{
						PacketMessageType message_type = PacketMessageType::UNKNOWN;
						data->Read(message_type);
						if (message_type == PacketMessageType::SCENE_STATE)
						{
							// Process entites...
							m_scene->ReadSceneState_Client(data);
							world_state_packet_received = true;
							Packet scene_state = client.CreateSendPacket(12);// TODO: Implement custom allocator{ as soon as possible}.
							PacketMessageType message_type = PacketMessageType::SCENE_STATE_RECEIVED;
							scene_state.data.Write(message_type);
							client.SendPacketToServer(scene_state, PacketSendMode::RELIABLE);
						}
					}
					else
					{
						m_scene->OnPacketReceive_Client(data, handle);
					}
				}
				else
				{
					m_scene->OnPacketReceive_Client(data, handle);
				}
				break;
			}
			case NetType::LISTEN_SERVER:
			{
				if (m_info.state_sync)
				{
					uint32_t prev_pos = data->GetPosition();
					PacketMessageType message_type = PacketMessageType::UNKNOWN;
					data->Read(message_type);
					if (message_type == PacketMessageType::SCENE_STATE_RECEIVED)
					{
						auto c_it = std::find(connected_clients.begin(), connected_clients.end(), handle);
						if (c_it == connected_clients.end())
						{
							process_new_client(handle);
							connected_clients.push_back(handle);
						}
						return;
					}
					else
					{
						data->SetPosition(prev_pos);
					}

				}
				m_scene->OnPacketReceive_Server(data, handle);
				break;
			}
			}
		}
	}

	bool NetworkManager::IsServer()
	{
		return m_type == NetType::LISTEN_SERVER;
	}

	bool NetworkManager::IsClient()
	{
		return m_type == NetType::CLIENT;
	}

	void NetworkManager::AcquireRPCStream()
	{
		++rpc_handle;
	}

	void NetworkManager::ReleaseRPCStream()
	{
		--rpc_handle;
	}

}