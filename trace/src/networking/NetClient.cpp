#include "pch.h"

#include "networking/NetClient.h"
#include "backends/Networkutils.h"
#include "core/io/Logging.h"

namespace trace::Network {
	bool NetClient::Init(NetworkStateInfo& network_info, bool LAN, uint32_t lan_listen_port, uint32_t max_found_connections)
	{
		m_LAN = LAN;
		bool result = NetFunc::CreateHost(HostType::CLIENT, &network_info, &m_info, LAN, SERVER_PORT);
		found_connections.reserve(max_found_connections);
		listen_port = lan_listen_port;

		lan_data = NetworkStream(1024);//TODO: size should be configurable

		return result;
	}
	bool NetClient::Shutdown()
	{
		if (m_connection.internal_handle)
		{
			NetFunc::Disconnect_C(&m_info, &m_connection);
			m_connection.handle = 0;
			m_connection.host = 0;
			m_connection.port = 0;
		}

		bool result = NetFunc::DestroyHost(&m_info, m_LAN);
		m_info.internal_handle = nullptr;

		return result;
	}
	bool NetClient::Listen(Packet& out_packet_data)
	{
		if (m_LAN && !m_connection.internal_handle)
		{
			Connection src = {};
			if (NetFunc::ReceiveSocketData(&m_info, lan_data, &src))
			{
				lan_data.SetPosition(0);

				uint8_t message_type = 0;
				lan_data.Read(message_type);
				switch (message_type)
				{
				case 24:
				{
					std::string response;
					lan_data.Read(response);
					if (response == "Acknowlegded: Chat App")
					{
						std::string server_name;
						lan_data.Read(server_name);
						src.port -= 24;
						found_connections[server_name] = src;
					}
					break;
				}
				}
			}

			lan_data.SetPosition(0);
			uint8_t message_type = 16;
			lan_data.Write(message_type);
			std::string text = "Looking for connection: Chat App";
			lan_data.Write(text);
			NetFunc::SendSocketData(&m_info, lan_data, nullptr, listen_port + 24);
		}

		bool result = false;
		Connection source = {};
		if (NetFunc::ReceivePacket_C(&m_info, out_packet_data.data, &source))
		{
			PacketType type = PacketType::NONE;
			out_packet_data.data.Read(type);

			switch (type)
			{
			case PacketType::CONNECTION_CHALLENGE:
			{
				if (m_connection.internal_handle)
				{
					return false;
				}

				uint32_t size = out_packet_data.data.GetSize() - out_packet_data.data.GetPosition();

				if (size == (1024 - sizeof(PacketType)))
				{
					uint32_t challenge = 0;
					out_packet_data.data.Read<uint32_t>(challenge);
					uint32_t solution = solve_challenge(challenge);

					// ... Send Solution
					NetworkStream response(1024);
					PacketType response_type = PacketType::CHALLENGE_RESPONSE;
					response.Write(response_type);
					response.Write(solution);

					NetFunc::SendPacket(&m_info, &source, response, PacketSendMode::RELIABLE);
				}

				result = false;
				break;
			}
			case PacketType::CONNECTION_ACCEPTED:
			{
				if (m_connection.internal_handle)
				{
					return false;
				}

				uint32_t size = out_packet_data.data.GetSize() - out_packet_data.data.GetPosition();

				if (size == (1024 - sizeof(PacketType)))
				{
					uint32_t connection_handle = 0;
					out_packet_data.data.Read<uint32_t>(connection_handle);
					m_connection.handle = connection_handle;
					m_connection.host = source.host;
					m_connection.port = source.port;
					m_connection.internal_handle = source.internal_handle;

					// ... Send Acknowlegdement
					NetworkStream response(512);
					PacketType response_type = PacketType::CONNECTION_ACKNOWLEGDED;
					response.Write(response_type);
					response.Write(connection_handle);

					NetFunc::SendPacket(&m_info, &source, response, PacketSendMode::RELIABLE);


					if (on_server_connect)
					{
						on_server_connect(connection_handle);
					}
				}


				result = false;
				break;
			}
			case PacketType::DISCONNECT:
			{
				if (m_connection.internal_handle)
				{
					return false;
				}

				if (on_server_disconnect)
				{
					on_server_disconnect(m_connection.handle);
				}

				m_connection.internal_handle = nullptr;//TODO: Let it be handle by the backend
				m_connection.handle = 0;
				m_connection.host = 0;
				m_connection.port = 0;

				result = false;
				break;
			}
			case PacketType::INCOMING_DATA:
			{
				if (!m_connection.internal_handle)
				{
					return false;
				}



				out_packet_data.connection_handle = m_connection.handle;

				result = true;

				break;
			}
			}
		}

		return result;
	}
	bool NetClient::ConnectTo(const std::string& server, uint32_t port)
	{
		Connection placeholder;// NOTE: this is not used because the actual connection will constructed in the listen function
		return NetFunc::ConnectTo_C(&m_info, &placeholder, server, port);
	}
	bool NetClient::ConnectToLAN(const std::string& server)
	{
		if (m_connection.internal_handle)
		{
			return false;
		}
		auto it = found_connections.find(server);
		if (it != found_connections.end())
		{
			Connection placeholder;// NOTE: this is not used because the actual connection will constructed in the listen function
			return NetFunc::ConnectTo_C(&m_info, &placeholder, &it->second);
		}

		return false;
	}
	Packet NetClient::CreateSendPacket(uint32_t size)
	{
		if (!m_connection.internal_handle)
		{
			TRC_WARN("Client has no connection to a server, Function: {}", __FUNCTION__ );
			return Packet();
		}


		Packet result;
		NetworkStream data(size + sizeof(PacketType) + sizeof(uint32_t));
		PacketType type = PacketType::INCOMING_DATA;
		data.Write(type);
		data.Write(m_connection.handle);
		data.MemSet(data.GetPosition(), data.GetSize(), 0x00);
		result.data = data;
		result.connection_handle = m_connection.handle;



		return result;
	}
	void NetClient::SendPacketToServer(Packet& packet, PacketSendMode mode)
	{
		if (!m_connection.internal_handle)
		{
			TRC_WARN("Client has no connection to a server, Function: {}", __FUNCTION__ );
			return;
		}

		NetFunc::SendPacket(&m_info, &m_connection, packet.data, mode);
	}
	uint32_t NetClient::solve_challenge(uint32_t challenge)
	{
		return (challenge * 8) + 1024;
	}
}