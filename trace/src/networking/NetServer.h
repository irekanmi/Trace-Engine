#pragma once

#include "networking/NetworkTypes.h"

#include <functional>

namespace trace::Network {


	class NetServer
	{

	public:

		bool Init(NetworkStateInfo& network_info, bool LAN = false, uint32_t port = SERVER_PORT);
		bool Shutdown();

		bool Listen(Packet& out_packet_data);
		Connection* FindConnection(uint32_t connection_handle);
		void BroadcastToAll(Packet packet, PacketSendMode mode = PacketSendMode::UNRELIABLE);
		void Broadcast(uint32_t source_connection, Packet packet, PacketSendMode mode = PacketSendMode::UNRELIABLE);
		void SendTo(uint32_t connection_handle, Packet packet, PacketSendMode mode = PacketSendMode::UNRELIABLE);
		Packet CreateSendPacket(uint32_t size);

		void SetClientConnectCallback(std::function<void(uint32_t connection_id)> client_connect) { on_client_connect = client_connect; }
		void SetClientDisconnectCallback(std::function<void(uint32_t connection_id)> client_disconnect) { on_client_disconnect = client_disconnect; }


	private:
		bool check_challenge_response(uint32_t result, uint32_t challenge);

	private:
		HostInfo m_info;
		std::vector<Connection> m_connections;// Clients
		std::unordered_map<uint32_t, Connection> m_pendingConnections;// Pending Clients
		std::unordered_map<uint32_t, uint32_t> m_pendingChallenge;// Pending Clients
		uint32_t num_max_connections = 0;
		uint32_t challenge = 256;// TEMP
		std::function<void(uint32_t connection_id)> on_client_connect;
		std::function<void(uint32_t connection_id)> on_client_disconnect;
		bool m_LAN = false;
		NetworkStream lan_data;

	protected:

	};

}
