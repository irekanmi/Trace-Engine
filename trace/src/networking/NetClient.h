#pragma once

#include "networking/NetworkTypes.h"
#include "networking/NetworkStream.h"

#include <functional>

namespace trace::Network {


	class NetClient
	{

	public:
		bool Init(NetworkStateInfo& network_info, bool LAN = false, uint32_t lan_listen_port = DISCOVERY_PORT, uint32_t max_found_connections = 8);
		bool Shutdown();

		bool Listen(Packet& out_packet_data, float wait_time);
		Connection* GetConnection(uint32_t connection_handle) { return &m_connection; }
		bool ConnectTo(const std::string& server, uint32_t port = SERVER_PORT);
		bool ConnectToLAN(const std::string& server);
		Packet CreateSendPacket(uint32_t size);
		void SendPacketToServer(Packet& packet, PacketSendMode mode = PacketSendMode::UNRELIABLE);
		std::unordered_map<std::string, Connection>& GetFoundConnections() { return found_connections; }
		bool HasConnection();
		float GetAverageRTT();


		void SetServerConnectCallback(std::function<void(uint32_t connection_handle)> server_connect) { on_server_connect = server_connect; }
		void SetServerDisconnectCallback(std::function<void(uint32_t connection_handle)> server_disconnect) { on_server_disconnect = server_disconnect; }
		void SetProcessPacketCallback(std::function<void(Packet& packet)> _process_packet) { process_packet = _process_packet; }

		void ProcessPacket(Packet& packet, Connection source);

	private:
		uint32_t solve_challenge(uint32_t challenge);

	private:
		HostInfo m_info;
		Connection m_connection;
		std::function<void(uint32_t connection_handle)> on_server_connect;
		std::function<void(uint32_t connection_handle)> on_server_disconnect;
		std::function<void(Packet& packet)> process_packet;
		bool m_LAN = false;
		NetworkStream lan_data;
		std::unordered_map<std::string, Connection> found_connections;
		uint32_t listen_port = DISCOVERY_PORT;
	protected:

	};

}
