#pragma once

#include "networking/NetworkTypes.h"
#include "networking/NetServer.h"
#include "networking/NetClient.h"
#include "networking/NetworkStream.h"

#include <string>
#include <vector>

namespace trace {
	class Scene;
}

namespace trace::Network {


	class NetworkManager
	{

	public:
		NetworkManager();
		~NetworkManager();
		bool Init();
		void Shutdown();
		void Update(float deltaTime);

		void OnSceneStart(Scene* scene);
		void OnSceneStop(Scene* scene);
		void OnGameStart();
		void OnGameStop();
		void OnFrameStart();
		void OnFrameEnd();
		bool CreateListenServer(uint32_t port = SERVER_PORT);
		bool CreateClient(bool LAN = false);
		bool ConnectTo(const std::string server, uint32_t port = SERVER_PORT);
		bool ConnectToLAN(const std::string& server);
		void DestroyNetInstance();
		NetServer* GetServerInstance();
		NetClient* GetClientInstance();
		NetworkStream* GetSendNetworkStream();
		void Send(NetworkStream* packet, PacketSendMode mode);
		NetType GetNetType() { return m_type; }
		uint32_t GetInstanceID() { return m_instanceId; }
		void OnClientConnect(uint32_t handle);
		void OnClientDisconnect(uint32_t handle);
		void OnServerConnect(uint32_t handle);
		void OnServerDisconnect(uint32_t handle);
		void OnPacketRecieve(NetworkStream* data, uint32_t handle);
		bool IsServer();
		bool IsClient();


		static NetworkManager* get_instance();

	private:
		void process_new_client(uint32_t handle);

	private:
		uint32_t m_instanceId = 0;//INFO: It specifies the Id of the currently running instance
		Scene* m_scene = nullptr;
		NetType m_type = NetType::UNKNOWN;
		NetServer server;
		NetClient client;
		Packet m_sendPacket;
		Packet m_receivePacket;
		NetworkStateInfo m_info;
		uint32_t m_sendStartPos = 0;
		bool world_state_packet_received = false;
		std::vector<uint32_t> new_clients;

	protected:

	};

}
