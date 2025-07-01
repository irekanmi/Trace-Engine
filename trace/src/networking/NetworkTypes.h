#pragma once

#include "networking/NetworkStream.h"

#include <cinttypes>

namespace trace::Network {

#define SERVER_PORT 2367
#define DISCOVERY_PORT 6932

	enum class NetType
	{
		UNKNOWN,
		CLIENT,
		LISTEN_SERVER
	};

	enum class NetObjectType
	{
		UNKNOWN,
		SERVER,
		CLIENT,
		BOTH
	};

	enum class RPCType
	{
		UNKNOW,
		CLIENT,
		SERVER
	};

	enum class PacketMessageType : uint8_t
	{
		UNKNOWN,
		CREATE_ENTITY,
		INSTANCIATE_PREFAB,
		DESTROY_ENTITY,
		ENTIITES_UPDATE,
		SCENE_STATE,
		SCENE_STATE_RECEIVED,
		RPC,
		CUSTOM_DATA
	};

	typedef int SocketHandle;

	enum class PacketType : uint8_t
	{
		NONE,
		CONNECTION_REQUEST,
		CONNECTION_CHALLENGE,
		CHALLENGE_RESPONSE,
		CONNECTION_ACCEPTED,
		CONNECTION_ACKNOWLEGDED,
		DISCONNECT,
		INCOMING_DATA
	};

	enum class PacketSendMode
	{
		NONE,
		RELIABLE,
		UNRELIABLE
	};
	
	struct Packet
	{
		NetworkStream data;
		uint32_t connection_handle = 0;
	};

	struct NetworkStateInfo
	{
		uint32_t max_num_connections = 32;
		bool state_sync = true;
	};

	enum class HostType
	{
		NONE,
		SERVER,
		CLIENT
	};

	struct HostInfo
	{
		uint32_t host_id = 0;
		void* internal_handle = nullptr;
		HostType type = HostType::NONE;
		SocketHandle lan_socket;//NOTE: Used for LAN connection
	};

	struct Connection
	{
		uint32_t handle = 0;
		uint32_t host = 0;
		uint32_t port = 0;
		void* internal_handle = nullptr;
	};
}
