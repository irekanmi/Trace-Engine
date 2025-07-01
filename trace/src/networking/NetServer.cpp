#include "pch.h"

#include "networking/NetServer.h"
#include "backends/Networkutils.h"

namespace trace::Network {

	bool NetServer::Init(NetworkStateInfo& network_info, bool LAN, uint32_t port)
	{
		bool result = NetFunc::CreateHost(HostType::SERVER, &network_info, &m_info, LAN, port);
		m_LAN = LAN;

		if (result)
		{
			m_connections.reserve(network_info.max_num_connections);
			num_max_connections = network_info.max_num_connections;
		}

		lan_data = NetworkStream(1024);//TODO: size should be configurable

		return result;
	}
	bool NetServer::Shutdown()
	{
		for (Connection& connection : m_connections)
		{
			if (connection.internal_handle)
			{
				NetFunc::Disconnect_S(&m_info, &connection);
			}
		}

		m_connections.clear();
		m_pendingChallenge.clear();
		m_pendingConnections.clear();
		if (m_LAN)
		{
			lan_data.Destroy();
		}

		bool result = NetFunc::DestroyHost(&m_info, m_LAN);

		return result;
	}
	bool NetServer::Listen(Packet& out_packet_data)
	{
		if (m_LAN)
		{
			Connection src = {};
			if (NetFunc::ReceiveSocketData(&m_info, lan_data, &src))
			{
				lan_data.SetPosition(0);
				uint8_t message_type = 0;
				lan_data.Read<uint8_t>(message_type);
				switch (message_type)
				{
				case 16:
				{
					std::string request;
					lan_data.Read<std::string>(request);
					std::string condition = "Looking for connection: Chat App";
					if (request == condition)
					{
						message_type = 24;
						std::string response = "Acknowlegded: Chat App";
						std::string server_name = "Chat App Server";
						lan_data.SetPosition(0);
						lan_data.Write(message_type);
						lan_data.Write(response);
						lan_data.Write(server_name);

						NetFunc::SendSocketData(&m_info, lan_data, &src, SERVER_PORT);
					}
					break;
				}
				}
			}

		}

		bool result = false;
		Connection source = {};
		if (NetFunc::ReceivePacket_S(&m_info, out_packet_data.data, &source))
		{
			PacketType type = PacketType::NONE;
			out_packet_data.data.Read<PacketType>(type);

			switch (type)
			{
			case PacketType::CONNECTION_REQUEST:
			{
				if (m_connections.size() >= num_max_connections)
				{
					return false;
				}
				uint32_t src_id = source.host ^ source.port;
				m_pendingConnections[src_id] = source;

				// ... Send Challenge
				NetworkStream response(1024);
				PacketType response_type = PacketType::CONNECTION_CHALLENGE;
				response.Write(response_type);
				uint32_t new_challenge = ++challenge;
				response.Write(new_challenge);
				m_pendingChallenge[src_id] = new_challenge;

				NetFunc::SendPacket(&m_info, &source, response, PacketSendMode::RELIABLE);

				result = false;
				break;
			}
			case PacketType::CHALLENGE_RESPONSE:
			{

				uint32_t size = out_packet_data.data.GetSize() - out_packet_data.data.GetPosition();

				uint32_t src_id = source.host ^ source.port;
				uint32_t challenge_result = 0;
				out_packet_data.data.Read<uint32_t>(challenge_result);

				if (check_challenge_response(challenge_result, m_pendingChallenge[src_id]))
				{
					uint32_t connection_handle = challenge_result ^ source.port;

					// ... Send Handle
					NetworkStream response(1024);
					PacketType response_type = PacketType::CONNECTION_ACCEPTED;
					response.Write(response_type);
					response.Write(connection_handle);

					NetFunc::SendPacket(&m_info, &source, response, PacketSendMode::RELIABLE);

					m_pendingConnections[src_id].handle = connection_handle;
					m_connections.push_back(m_pendingConnections[src_id]);
					m_pendingChallenge.erase(src_id);
					m_pendingConnections.erase(src_id);

				}



				result = false;
				break;
			}
			case PacketType::DISCONNECT:
			{
				auto it = std::find_if(m_connections.begin(), m_connections.end(), [&source](Connection& val)
					{
						return (source.host == val.host) && (source.port == val.port);//TODO: Find a way to determine the connection using the handle
					});

				if (it != m_connections.end())
				{
					if (on_client_disconnect)
					{
						on_client_disconnect(it->handle);
					}

					m_connections.erase(it);

				}

				result = false;
				break;
			}
			case PacketType::INCOMING_DATA:
			{

				uint32_t connection_handle = 0;
				out_packet_data.data.Read<uint32_t>(connection_handle);
				Connection* connection = FindConnection(connection_handle);
				out_packet_data.connection_handle = connection_handle;

				result = connection ? true : false;

				break;
			}
			case PacketType::CONNECTION_ACKNOWLEGDED:
			{

				uint32_t connection_handle = 0;
				out_packet_data.data.Read<uint32_t>(connection_handle);
				Connection* connection = FindConnection(connection_handle);
				out_packet_data.connection_handle = connection_handle;

				if (connection && on_client_connect)
				{
					on_client_connect(connection_handle);
				}

				result = false;
				break;
			}
			}
		}

		return result;
	}
	Connection* NetServer::FindConnection(uint32_t connection_handle)
	{
		for (Connection& connection : m_connections)
		{
			if (connection.handle == connection_handle)
			{
				return &connection;
			}
		}

		return nullptr;
	}
	void NetServer::BroadcastToAll(Packet packet, PacketSendMode mode)
	{
		for (Connection& connection : m_connections)
		{
			NetFunc::SendPacket(&m_info, &connection, packet.data, mode);
		}
	}
	void NetServer::Broadcast(uint32_t source_connection, Packet packet, PacketSendMode mode)
	{
		for (Connection& connection : m_connections)
		{
			if (connection.handle == source_connection)
			{
				continue;
			}

			NetFunc::SendPacket(&m_info, &connection, packet.data, mode);
		}
	}
	void NetServer::SendTo(uint32_t connection_handle, Packet packet, PacketSendMode mode)
	{
		for (Connection& connection : m_connections)
		{
			if (connection.handle == connection_handle)
			{
				NetFunc::SendPacket(&m_info, &connection, packet.data, mode);
				break;
			}

		}
	}
	Packet NetServer::CreateSendPacket(uint32_t size)
	{
		Packet result;
		NetworkStream data(size + sizeof(PacketType));
		PacketType type = PacketType::INCOMING_DATA;
		data.Write(type);
		data.MemSet(data.GetPosition(), data.GetSize(), 0x00);
		result.data = data;
		result.connection_handle = 0;
		return result;
	}
	bool NetServer::check_challenge_response(uint32_t result, uint32_t challenge)
	{
		uint32_t value = (result - 1024) / 8;

		return value == challenge;
	}
}