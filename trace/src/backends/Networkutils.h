#pragma once

#include "core/Core.h"
#include "core/Enums.h"
#include "networking/NetworkTypes.h"
#include "networking/NetworkStream.h"

namespace trace {
	using namespace Network;

	typedef bool (*__Initialize)(Network::NetworkStateInfo& info);
	typedef bool (*__Shutdown)();
	typedef bool (*__CreateHost)(HostType type, NetworkStateInfo* network_info, HostInfo* out_result, bool LAN, uint32_t port);
	typedef bool (*__DestroyHost)(HostInfo* host, bool LAN);
	typedef bool (*__ConnectTo_C)(HostInfo* host, Connection* out_handle, const std::string& server, uint32_t port);
	typedef bool (*__ConnectTo__C)(HostInfo* host, Connection* out_handle, Connection* server);
	typedef bool (*__Disconnect_C)(HostInfo* host, Connection* connection);
	typedef bool (*__Disconnect_S)(HostInfo* host, Connection* connection);
	typedef bool (*__ReceivePacket_C)(HostInfo* host, NetworkStream& packet_data, Connection* out_source_connection);
	typedef bool (*__ReceivePacket_S)(HostInfo* host, NetworkStream& packet_data, Connection* out_source_connection);
	typedef bool (*__SendPacket)(HostInfo* host, Connection* connection, NetworkStream& packet_data, PacketSendMode mode);
	typedef bool (*__ReceiveSocketData)(HostInfo* host, NetworkStream& packet_data, Connection* source);
	typedef bool (*__SendSocketData)(HostInfo* host, NetworkStream& packet_data, Connection* source, uint32_t port);

	class NetFuncLoader
	{
	public:
		static bool Load_ENet_Func();
	private:
		
	protected:
	};

	class NetFunc
	{

	public:
		static bool Initialize(Network::NetworkStateInfo& info);
		static bool Shutdown();
		static bool CreateHost(HostType type, NetworkStateInfo* network_info, HostInfo* out_result, bool LAN, uint32_t port);
		static bool DestroyHost(HostInfo* host, bool LAN);
		static bool ConnectTo_C(HostInfo* host, Connection* out_handle, const std::string& server, uint32_t port);
		static bool ConnectTo_C(HostInfo* host, Connection* out_handle, Connection* server);
		static bool Disconnect_C(HostInfo* host, Connection* connection);
		static bool Disconnect_S(HostInfo* host, Connection* connection);
		static bool ReceivePacket_C(HostInfo* host, NetworkStream& packet_data, Connection* out_source_connection);
		static bool ReceivePacket_S(HostInfo* host, NetworkStream& packet_data, Connection* out_source_connection);
		static bool SendPacket(HostInfo* host, Connection* connection, NetworkStream& packet_data, PacketSendMode mode);
		static bool ReceiveSocketData(HostInfo* host, NetworkStream& packet_data, Connection* source);
		static bool SendSocketData(HostInfo* host, NetworkStream& packet_data, Connection* source, uint32_t port);

	private:
		static __Initialize _initialize;
		static __Shutdown _shutdown;
		static __CreateHost _createHost;
		static __DestroyHost _destroyHost;
		static __ConnectTo_C _connectTo_C;
		static __ConnectTo__C _connectTo__C;
		static __Disconnect_C _disconnect_C;
		static __Disconnect_S _disconnect_S;
		static __ReceivePacket_C _receivePacket_C;
		static __ReceivePacket_S _receivePacket_S;
		static __SendPacket _sendPacket;
		static __ReceiveSocketData _recevieSocketData;
		static __SendSocketData _sendSocketData;

	protected:
		friend class NetFuncLoader;

	};

}
