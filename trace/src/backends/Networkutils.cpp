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

bool __ENet_Initialize(NetworkStateInfo& info)
{

	return true;
}

bool __ENet_Shutdown()
{

	return true;
}

bool __ENet_CreateHost(HostType type, NetworkStateInfo* network_info, HostInfo* out_result, bool LAN, uint32_t port)
{
	return true;
}
bool __ENet_DestroyHost(HostInfo* host, bool LAN)
{
	return true;
}
bool __ENet_ConnectTo_C(HostInfo* host, Connection* out_handle, const std::string& server, uint32_t port)
{
	return true;
}
bool __ENet_ConnectTo__C(HostInfo* host, Connection* out_handle, Connection* server)
{
	return true;
}
bool __ENet_Disconnect_C(HostInfo* host, Connection* connection)
{
	return true;
}
bool __ENet_Disconnect_S(HostInfo* host, Connection* connection)
{
	return true;
}
bool __ENet_ReceivePacket_C(HostInfo* host, NetworkStream& packet_data, Connection* out_source_connection)
{
	return true;
}
bool __ENet_ReceivePacket_S(HostInfo* host, NetworkStream& packet_data, Connection* out_source_connection)
{
	return true;
}
bool __ENet_SendPacket(HostInfo* host, Connection* connection, NetworkStream& packet_data, PacketSendMode mode)
{
	return true;
}
bool __ENet_ReceiveSocketData(HostInfo* host, NetworkStream& packet_data, Connection* source)
{
	return true;
}
bool __ENet_SendSocketData(HostInfo* host, NetworkStream& packet_data, Connection* source, uint32_t port)
{
	return true;
}
