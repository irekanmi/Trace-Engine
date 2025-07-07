#include "pch.h"

#include "backends/Networkutils.h"
#include "core/io/Logging.h"

using namespace trace::Network;

#define NET_FUNC_IS_VALID(function)							 \
	if(!function)                                                \
	{                                                            \
		TRC_ERROR(                                               \
	"{} is not available, please check for any errors"           \
		, #function);                                            \
		return false;                                            \
	}


bool __ENet_Initialize(NetworkStateInfo & info);
bool __ENet_Shutdown();
bool __ENet_CreateHost(HostType type, NetworkStateInfo * network_info, HostInfo * out_result, bool LAN, uint32_t port);
bool __ENet_DestroyHost(HostInfo * host, bool LAN);
bool __ENet_ConnectTo_C(HostInfo * host, Connection * out_handle, const std::string & server, uint32_t port);
bool __ENet_ConnectTo__C(HostInfo * host, Connection * out_handle, Connection * server);
bool __ENet_Disconnect_C(HostInfo * host, Connection * connection);
bool __ENet_Disconnect_S(HostInfo * host, Connection * connection);
bool __ENet_ReceivePacket_C(HostInfo * host, NetworkStream & packet_data, Connection * out_source_connection);
bool __ENet_ReceivePacket_S(HostInfo * host, NetworkStream & packet_data, Connection * out_source_connection);
bool __ENet_SendPacket(HostInfo * host, Connection * connection, NetworkStream & packet_data, PacketSendMode mode);
bool __ENet_ReceiveSocketData(HostInfo * host, NetworkStream & packet_data, Connection * source);
bool __ENet_SendSocketData(HostInfo * host, NetworkStream & packet_data, Connection * source, uint32_t port);

namespace trace {

	bool NetFuncLoader::Load_ENet_Func()
	{
		NetFunc::_initialize = __ENet_Initialize;
		NetFunc::_shutdown = __ENet_Shutdown;
		NetFunc::_createHost = __ENet_CreateHost;
		NetFunc::_destroyHost = __ENet_DestroyHost;
		NetFunc::_connectTo_C = __ENet_ConnectTo_C;
		NetFunc::_connectTo__C = __ENet_ConnectTo__C;
		NetFunc::_disconnect_C = __ENet_Disconnect_C;
		NetFunc::_disconnect_S = __ENet_Disconnect_S;
		NetFunc::_receivePacket_C = __ENet_ReceivePacket_C;
		NetFunc::_receivePacket_S = __ENet_ReceivePacket_S;
		NetFunc::_sendPacket = __ENet_SendPacket;
		NetFunc::_recevieSocketData = __ENet_ReceiveSocketData;
		NetFunc::_sendSocketData = __ENet_SendSocketData;
		return true;
	}

	__Initialize NetFunc::_initialize = nullptr;
	__Shutdown NetFunc::_shutdown = nullptr;
	__CreateHost NetFunc::_createHost = nullptr;
	__DestroyHost NetFunc::_destroyHost = nullptr;
	__ConnectTo_C NetFunc::_connectTo_C = nullptr;
	__ConnectTo__C NetFunc::_connectTo__C = nullptr;
	__Disconnect_C NetFunc::_disconnect_C = nullptr;
	__Disconnect_S NetFunc::_disconnect_S = nullptr;
	__ReceivePacket_C NetFunc::_receivePacket_C = nullptr;
	__ReceivePacket_S NetFunc::_receivePacket_S = nullptr;
	__SendPacket NetFunc::_sendPacket = nullptr;
	__ReceiveSocketData NetFunc::_recevieSocketData = nullptr;
	__SendSocketData NetFunc::_sendSocketData = nullptr;


	bool NetFunc::Initialize(Network::NetworkStateInfo& info)
	{
		NET_FUNC_IS_VALID(_initialize);
		return _initialize(info);
	}

	bool NetFunc::Shutdown()
	{
		NET_FUNC_IS_VALID(_shutdown);
		return _shutdown();
	}

	bool NetFunc::CreateHost(HostType type, NetworkStateInfo* network_info, HostInfo* out_result, bool LAN, uint32_t port)
	{
		NET_FUNC_IS_VALID(_createHost);
		return _createHost(type, network_info, out_result, LAN, port);
	}

	bool NetFunc::DestroyHost(HostInfo* host, bool LAN)
	{
		NET_FUNC_IS_VALID(_destroyHost);
		return _destroyHost(host, LAN);
	}

	bool NetFunc::ConnectTo_C(HostInfo* host, Connection* out_handle, const std::string& server, uint32_t port)
	{
		NET_FUNC_IS_VALID(_connectTo_C);
		return _connectTo_C(host, out_handle, server, port);
	}

	bool NetFunc::ConnectTo_C(HostInfo* host, Connection* out_handle, Connection* server)
	{
		NET_FUNC_IS_VALID(_connectTo__C);
		return _connectTo__C(host, out_handle, server);
	}

	bool NetFunc::Disconnect_C(HostInfo* host, Connection* connection)
	{
		NET_FUNC_IS_VALID(_disconnect_C);
		return _disconnect_C(host, connection);
	}

	bool NetFunc::Disconnect_S(HostInfo* host, Connection* connection)
	{
		NET_FUNC_IS_VALID(_disconnect_S);
		return _disconnect_S(host, connection);
	}

	bool NetFunc::ReceivePacket_C(HostInfo* host, NetworkStream& packet_data, Connection* out_source_connection)
	{
		NET_FUNC_IS_VALID(_receivePacket_C);
		return _receivePacket_C(host, packet_data, out_source_connection);
	}

	bool NetFunc::ReceivePacket_S(HostInfo* host, NetworkStream& packet_data, Connection* out_source_connection)
	{
		NET_FUNC_IS_VALID(_receivePacket_S);
		return _receivePacket_S(host, packet_data, out_source_connection);
	}

	bool NetFunc::SendPacket(HostInfo* host, Connection* connection, NetworkStream& packet_data, PacketSendMode mode)
	{
		NET_FUNC_IS_VALID(_sendPacket);
		return _sendPacket(host, connection, packet_data, mode);
	}

	bool NetFunc::ReceiveSocketData(HostInfo* host, NetworkStream& packet_data, Connection* source)
	{
		NET_FUNC_IS_VALID(_recevieSocketData);
		return _recevieSocketData(host, packet_data, source);
	}

	bool NetFunc::SendSocketData(HostInfo* host, NetworkStream& packet_data, Connection* source, uint32_t port)
	{
		NET_FUNC_IS_VALID(_sendSocketData);
		return _sendSocketData(host, packet_data, source, port);
	}


}

#include "enet/enet.h"

bool __ENet_Initialize(NetworkStateInfo& info)
{
	int result = enet_initialize();

	if (result != 0)
	{
		TRC_ERROR("Failed to initiaize ENet, Function: {} ", __FUNCTION__);
		return false;
	}
	return true;
}

bool __ENet_Shutdown()
{
	enet_deinitialize();
	return true;
}

bool __ENet_CreateHost(HostType type, NetworkStateInfo* network_info, HostInfo* out_result, bool LAN, uint32_t port)
{
	if (!out_result)
	{
		TRC_ERROR("Pass in a valid pointer to store created host, Function: {}", __FUNCTION__);
		return false;
	}

	switch (type)
	{
	case HostType::SERVER:
	{
		if (!network_info)
		{
			return false;
		}

		ENetAddress address;
		ENetHost* host = nullptr;

		address.host = ENET_HOST_ANY;
		address.port = port;

		host = enet_host_create(
			&address,
			network_info->max_num_connections,
			1,
			0,
			0
		);

		if (!host)
		{
			TRC_ERROR("Unable to create host, Function: {}", __FUNCTION__);
			return false;
		}

		out_result->internal_handle = host;

		enet_host_bandwidth_limit(host, 50 * KB, 50 * KB);

		if (LAN)
		{
			address.port += 24;
			ENetSocket socket = enet_socket_create(ENetSocketType::ENET_SOCKET_TYPE_DATAGRAM);
			int result = enet_socket_set_option(socket, ENetSocketOption::ENET_SOCKOPT_NONBLOCK, 1);
			result = enet_socket_set_option(socket, ENetSocketOption::ENET_SOCKOPT_BROADCAST, 1);
			result = enet_socket_bind(socket, &address);

			out_result->lan_socket = socket;

		}


		break;
	}
	case HostType::CLIENT:
	{

		ENetHost* host = nullptr;

		host = enet_host_create(
			nullptr,
			1,
			1,
			0,
			0
		);

		if (!host)
		{
			TRC_ERROR("Unable to create host, Function: {}", __FUNCTION__);
			return false;
		}

		out_result->internal_handle = host;
		enet_host_bandwidth_limit(host, 50 * KB, 50 * KB);
		if (LAN)
		{

			ENetAddress address;
			address.host = ENET_HOST_ANY;
			address.port = 0;
			ENetSocket socket = enet_socket_create(ENetSocketType::ENET_SOCKET_TYPE_DATAGRAM);
			int result = enet_socket_set_option(socket, ENetSocketOption::ENET_SOCKOPT_NONBLOCK, 1);
			result = enet_socket_set_option(socket, ENetSocketOption::ENET_SOCKOPT_BROADCAST, 1);
			result = enet_socket_bind(socket, &address);

			out_result->lan_socket = socket;
		}

		break;
	}
	}

	out_result->type = type;

	return true;
}
bool __ENet_DestroyHost(HostInfo* host, bool LAN)
{
	if (!host)
	{
		TRC_ERROR("Pass in a valid pointer to host, Function: {}", __FUNCTION__);
		return false;
	}

	if (!host->internal_handle)
	{
		TRC_ERROR("Invalid host handle, Function: {}", __FUNCTION__);
		return false;
	}

	if (LAN)
	{
		enet_socket_destroy(host->lan_socket);
	}

	ENetHost* handle = (ENetHost*)host->internal_handle;

	enet_host_destroy(handle);
	host->internal_handle = nullptr;

	return true;
}
bool __ENet_ConnectTo_C(HostInfo* host, Connection* out_handle, const std::string& server, uint32_t port)
{
	if (!host)
	{
		TRC_ERROR("Pass in a valid pointer to host, Function: {}", __FUNCTION__);
		return false;
	}

	if (!host->internal_handle)
	{
		TRC_ERROR("Invalid host handle, Function: {}", __FUNCTION__);
		return false;
	}

	if (!out_handle)
	{
		TRC_ERROR("Pass in a valid pointer to store the connection, Function: {}", __FUNCTION__);
		return false;
	}

	ENetAddress address;
	ENetEvent event;
	ENetPeer* peer;

	enet_address_set_host(&address, server.c_str());
	address.port = port;

	/* Initiate the connection, allocating the two channels 0 and 1. */
	peer = enet_host_connect((ENetHost*)host->internal_handle, &address, 1, 0);

	if (!peer)
	{
		TRC_ERROR("Unable to connect to {}, Function: {}", server, __FUNCTION__);
		return false;
	}


	///* Wait up to 5 seconds for the connection attempt to succeed. */
	//if (enet_host_service((ENetHost*)host->internal_handle, &event, 5000) > 0 &&
	//	event.type == ENET_EVENT_TYPE_CONNECT)
	//{
	//}
	//else
	//{
	//	/* Either the 5 seconds are up or a disconnect event was */
	//	/* received. Reset the peer in the event the 5 seconds   */
	//	/* had run out without any significant event.            */
	//	enet_peer_reset(peer);

	//	TRC_ERROR("Unable to connect to " << server << " , Function: {}", __FUNCTION__);
	//	return false;
	//}

	//out_handle->internal_handle = peer;


	return true;
}
bool __ENet_ConnectTo__C(HostInfo* host, Connection* out_handle, Connection* server)
{
	if (!host)
	{
		TRC_ERROR("Pass in a valid pointer to host, Function: {}", __FUNCTION__);
		return false;
	}

	if (!host->internal_handle)
	{
		TRC_ERROR("Invalid host handle, Function: {}", __FUNCTION__);
		return false;
	}

	if (!out_handle)
	{
		TRC_ERROR("Pass in a valid pointer to store the connection, Function: {}", __FUNCTION__);
		return false;
	}

	ENetAddress address;
	ENetEvent event;
	ENetPeer* peer;

	address.host = server->host;
	address.port = server->port;

	/* Initiate the connection, allocating the two channels 0 and 1. */
	peer = enet_host_connect((ENetHost*)host->internal_handle, &address, 1, 0);

	if (!peer)
	{
		TRC_ERROR("Unable to connect, Function: {}", __FUNCTION__);
		return false;
	}


	return true;
}
bool __ENet_Disconnect_C(HostInfo* host, Connection* connection)
{
	if (!host)
	{
		TRC_ERROR("Pass in a valid pointer to host, Function: {}", __FUNCTION__);
		return false;
	}

	if (!host->internal_handle)
	{
		TRC_ERROR("Invalid host handle, Function: {}", __FUNCTION__);
		return false;
	}

	if (!connection)
	{
		TRC_ERROR("Pass in a valid pointer to connection, Function: {}", __FUNCTION__);
		return false;
	}

	ENetEvent event;
	ENetPeer* peer = (ENetPeer*)connection->internal_handle;
	ENetHost* client = (ENetHost*)host->internal_handle;

	enet_peer_disconnect(peer, 0);

	/* Allow up to 16 milliseconds for the disconnect to succeed
	 * and drop any packets received packets.
	 */
	while (enet_host_service(client, &event, 16) > 0)
	{
		switch (event.type)
		{
		case ENET_EVENT_TYPE_RECEIVE:
			enet_packet_destroy(event.packet);
			break;

		case ENET_EVENT_TYPE_DISCONNECT:
			connection->internal_handle = nullptr;
			TRC_INFO("Disconnection succeeded.");
			return true;
		}
	}

	/* We've arrived here, so the disconnect attempt didn't */
	/* succeed yet.  Force the connection down.             */
	enet_peer_reset(peer);

	connection->internal_handle = nullptr;

	return true;
}
bool __ENet_Disconnect_S(HostInfo* host, Connection* connection)
{
	return __ENet_Disconnect_C(host, connection);
}
bool __ENet_ReceivePacket_C(HostInfo* host, NetworkStream& packet_data, Connection* out_source_connection)
{
	if (!host)
	{
		TRC_ERROR("Pass in a valid pointer to host, Function: {}", __FUNCTION__);
		return false;
	}

	if (!host->internal_handle)
	{
		TRC_ERROR("Invalid host handle, Function: {}", __FUNCTION__);
		return false;
	}

	ENetEvent network_event;
	ENetHost* client = (ENetHost*)host->internal_handle;

	while (enet_host_service(client, &network_event, 0) > 0)
	{
		switch (network_event.type)
		{
		case ENET_EVENT_TYPE_CONNECT:
		{
			out_source_connection->host = network_event.peer->address.host;
			out_source_connection->port = network_event.peer->address.port;
			out_source_connection->internal_handle = network_event.peer;

			return false;
			break;
		}

		case ENET_EVENT_TYPE_RECEIVE:
		{
			out_source_connection->host = network_event.peer->address.host;
			out_source_connection->port = network_event.peer->address.port;
			out_source_connection->internal_handle = network_event.peer;

			//packet_data = NetworkStream(network_event.packet->data, network_event.packet->dataLength, true);
			packet_data.Write(0, network_event.packet->data, network_event.packet->dataLength);
			packet_data.SetPosition(0);
			packet_data.MemSet(network_event.packet->dataLength, packet_data.GetSize(), 0x00);

			/* Clean up the packet now that we're done using it. */
			enet_packet_destroy(network_event.packet);

			return true;
			break;
		}

		case ENET_EVENT_TYPE_DISCONNECT:
		{
			out_source_connection->host = network_event.peer->address.host;
			out_source_connection->port = network_event.peer->address.port;
			out_source_connection->internal_handle = network_event.peer;
			//TODO: Handle server disconnection
			TRC_ERROR("Lost Connection to Server");

			return true;
			break;
		}
		}
	}


	return false;
}
bool __ENet_ReceivePacket_S(HostInfo* host, NetworkStream& packet_data, Connection* out_source_connection)
{
	if (!host)
	{
		TRC_ERROR("Pass in a valid pointer to host, Function: {}", __FUNCTION__);
		return false;
	}

	if (!host->internal_handle)
	{
		TRC_ERROR("Invalid host handle, Function: {}", __FUNCTION__);
		return false;
	}

	ENetEvent network_event;
	ENetHost* server = (ENetHost*)host->internal_handle;

	while (enet_host_service(server, &network_event, 0) > 0)
	{
		switch (network_event.type)
		{
		case ENET_EVENT_TYPE_CONNECT:
		{
			out_source_connection->host = network_event.peer->address.host;
			out_source_connection->port = network_event.peer->address.port;
			out_source_connection->internal_handle = network_event.peer;


			NetworkStream out_data(sizeof(PacketType));
			PacketType type = PacketType::CONNECTION_REQUEST;
			out_data.Write(type);

			packet_data = out_data;
			packet_data.SetPosition(0);

			return true;
			break;
		}

		case ENET_EVENT_TYPE_RECEIVE:
		{
			out_source_connection->host = network_event.peer->address.host;
			out_source_connection->port = network_event.peer->address.port;
			out_source_connection->internal_handle = network_event.peer;
			packet_data.Write(0, network_event.packet->data, network_event.packet->dataLength);
			packet_data.SetPosition(0);
			packet_data.MemSet(network_event.packet->dataLength, packet_data.GetSize(), 0x00);

			/* Clean up the packet now that we're done using it. */
			enet_packet_destroy(network_event.packet);


			return true;
			break;
		}

		case ENET_EVENT_TYPE_DISCONNECT:
		{
			out_source_connection->host = network_event.peer->address.host;
			out_source_connection->port = network_event.peer->address.port;
			out_source_connection->internal_handle = network_event.peer;

			NetworkStream out_data(sizeof(PacketType));
			PacketType type = PacketType::DISCONNECT;
			out_data.Write(type);

			packet_data = out_data;
			packet_data.SetPosition(0);

			return true;
			break;
		}
		}
	}

	return false;
}
bool __ENet_SendPacket(HostInfo* host, Connection* connection, NetworkStream& packet_data, PacketSendMode mode)
{
	if (!host)
	{
		TRC_ERROR("Pass in a valid pointer to host, Function: {}", __FUNCTION__);
		return false;
	}

	if (!host->internal_handle)
	{
		TRC_ERROR("Invalid host handle, Function: {}", __FUNCTION__);
		return false;
	}

	ENetPacketFlag flag = mode == PacketSendMode::RELIABLE ? ENetPacketFlag::ENET_PACKET_FLAG_RELIABLE : ENetPacketFlag::ENET_PACKET_FLAG_UNRELIABLE_FRAGMENT/* | ENetPacketFlag::ENET_PACKET_FLAG_UNSEQUENCED*/;

	ENetPacket* packet = enet_packet_create(packet_data.GetData(), packet_data.GetPosition(), flag);

	ENetPeer* peer = (ENetPeer*)connection->internal_handle;

	enet_peer_send(peer, 0, packet);

	return true;
}
bool __ENet_ReceiveSocketData(HostInfo* host, NetworkStream& packet_data, Connection* source)
{
	if (!host)
	{
		TRC_ERROR("Pass in a valid pointer to host, Function: {}", __FUNCTION__);
		return false;
	}

	if (!host->internal_handle)
	{
		TRC_ERROR("Invalid host handle, Function: {}", __FUNCTION__);
		return false;
	}

	ENetAddress address;
	ENetBuffer buffer;
	buffer.data = packet_data.GetData();
	buffer.dataLength = packet_data.GetSize();
	int len = enet_socket_receive(host->lan_socket, &address, &buffer, 1);

	if (len > 0)
	{
		packet_data = NetworkStream(buffer.data, buffer.dataLength, true);
		source->host = address.host;
		source->port = address.port;
		return true;
	}

	return false;
}
bool __ENet_SendSocketData(HostInfo* host, NetworkStream& packet_data, Connection* source, uint32_t port)
{
	if (!host)
	{
		TRC_ERROR("Pass in a valid pointer to host, Function: {}", __FUNCTION__);
		return false;
	}

	if (!host->internal_handle)
	{
		TRC_ERROR("Invalid host handle, Function: {}", __FUNCTION__);
		return false;
	}

	ENetAddress address;
	int result;
	if (source)
	{
		address.host = source->host;
		address.port = source->port;
	}
	else
	{
		address.host = ENET_HOST_BROADCAST;
		//result = enet_address_set_host(&address, "192.168.0.255");
		address.port = port;
	}

	ENetBuffer buffer;
	buffer.data = packet_data.GetData();
	buffer.dataLength = packet_data.GetSize();

	result = enet_socket_send(host->lan_socket, &address, &buffer, 1);

	return true;
}
