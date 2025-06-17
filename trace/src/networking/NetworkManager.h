#pragma once

#include "networking/NetworkTypes.h"
#include "networking/NetServer.h"
#include "networking/NetClient.h"
#include "networking/NetworkStream.h"

#include <string>

namespace trace {
	class Scene;
}

namespace trace::Network {


	class NetworkManager
	{

	public:
		bool Init();
		void Shutdown();

		void OnSceneStart(Scene* scene);
		void OnSceneStop(Scene* scene);
		void OnGameStart();
		void OnGameStop();
		void OnFrameStart();
		void OnFrameEnd();
		bool CreateListenServer(uint32_t port = SERVER_PORT);
		bool ConnectTo(const std::string server, uint32_t port = SERVER_PORT);
		bool ConnectToLAN(uint32_t port = DISCOVERY_PORT);
		void DestroyNetInstance();
		NetServer* GetServerInstance();
		NetClient* GetClientInstance();
		NetworkStream* GetSendNetworkStream();
		NetworkStream* GetRPCNetworkStream();
		void Send(NetworkStream* packet, uint16_t mode);
		NetType GetNetType() { return m_type; }
		uint32_t GetInstanceID() { return m_instanceId; }
		void OnClientConnect(uint32_t handle);
		void OnClientDisconnect(uint32_t handle);
		void OnServerConnect(uint32_t handle);
		void OnServerDisconnect(uint32_t handle);
		void OnPacketRecieve(NetworkStream* data, uint32_t handle);


		static NetworkManager* get_instance();
	private:
		uint32_t m_instanceId = 0;//INFO: It specifies the Id of the currently running instance
		Scene* m_scene = nullptr;
		NetType m_type = NetType::UNKNOWN;
		union
		{
			NetServer server;
			NetClient client;
		};
		NetworkStream m_sendPacket;
		NetworkStream m_rpcPacket;

	protected:

	};

}
